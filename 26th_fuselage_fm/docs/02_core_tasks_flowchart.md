# 02_core_tasks_flowchart.md - マルチコア処理・FreeRTOSタスクとキュー制御

**プロジェクト**: 26th_Software_Fuselage (`26th_fuselage_fm`)  
**役割**: `setup()` におけるシステムの初期化シーケンスと、ESP32 デュアルコア上で稼働する 3つのメイン FreeRTOS タスク (`Core0_Task`, `Core1_Task`, `SD_Task`) および `sdQueue` の内部動作と通信フローについて解説します。

---

## 1. setup() 初期化シーケンスとタスク構成

`26th_fuselage_fm.ino` の `setup()` 関数では、ESP32 デュアルコア (Core 0 / Core 1) に応じたセンサーおよびハードウェアデバイスの初期化を実行した後、FreeRTOS の `xTaskCreatePinnedToCore()` により各コアへタスクをバインドして起動します。

### タスク割り当て仕様表
| タスク名 | バインド先 CPU コア | 優先度 (Priority) | スタックサイズ | 実行周期・トリガー | 主な担当処理 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Core0_Task`** | **Core 0** (`0`) | `6` (最高) | 4096 Byte | **10ms周期 (100Hz)**<br>(`vTaskDelayUntil`) | LSM姿勢推定、BMP高度計算、BNO取得、オーディオピッチ制御、警報ブザー |
| **`Core1_Task`** | **Core 1** (`1`) | `5` (高) | 4096 Byte | **10ms周期 (100Hz)**<br>(`vTaskDelay`) | Bico UART 受信・コマンド解析、SDキュー投入、Bico UART 時分割送信 |
| **`SD_Task`** | **Core 1** (`1`) | `4` (中) | 4096 Byte | **イベント駆動**<br>(`xQueueReceive`) | `sdQueue` からのデータ取り出し、コマンド記録 (`RESET`/`CALIB`)、SD カード書き込み |

---

### 1.1 setup() 初期化シーケンス図 (`sequenceDiagram`)

以下は、システム起動直後から各タスクが並行稼働を開始するまでの初期化シーケンスです。

```mermaid
sequenceDiagram
    autonumber
    actor Power as 電源ON / Reset
    participant Setup as setup()
    participant Core0_Init as Core 0 ドライバ初期化
    participant Core1_Init as Core 1 ドライバ初期化
    participant RTOS as FreeRTOS カーネル
    participant C0_Task as Core0_Task (Core 0)
    participant C1_Task as Core1_Task (Core 1)
    participant SD_Task as SD_Task (Core 1)

    Power->>Setup: マイコン起動
    Setup->>Setup: Serial.begin(115200) / インジケータピン設定 (LED1, LED2)

    Note over Setup,Core0_Init: --- Core 0 用センサー・オーディオ初期化 ---
    Setup->>Core0_Init: pitchsender_init()
    Core0_Init->>Core0_Init: imu_init() [LSM6DSV16X I2C 0x6A/0x6B 確認＆設定]
    Core0_Init->>Core0_Init: bt_init("WF-C510") [Bluetooth A2DP ソース開始＆フレームコールバック登録]
    Setup->>Core0_Init: BNO055_init() [BNO055 I2C 0x28 検出＆外部水晶発振器設定]
    Setup->>Core0_Init: BMP3XX_init() [BMP390 I2C 0x76 設定 / 8倍・4倍オーバーサンプリング]

    Note over Setup,Core1_Init: --- Core 1 用ストレージ・通信初期化 ---
    Setup->>Core1_Init: initSDTask() [SD_wrapper.cpp]
    Core1_Init->>RTOS: xQueueCreate(20, sizeof(LogData)) -> sdQueue 作成
    Core1_Init->>Core1_Init: initSD() [SPI ピン開始 & TORICA_SD.begin(CS: GPIO16)]
    Core1_Init->>Core1_Init: flashHeader() [CSVヘッダーを 4分割モードで SD 書き込み]
    Core1_Init->>RTOS: xTaskCreatePinnedToCore(SD_Task, Priority 4, Core 1)
    RTOS-->>SD_Task: SD_Task 起動 (キュー待ち受動状態へ)

    Setup->>Core1_Init: initUART() [UARTHelper_fslg.cpp]
    Core1_Init->>Core1_Init: Serial_Bico.setRxBufferSize(1024) / Serial_Bico.begin(460800, SERIAL_8E1, RX:32, TX:33)

    Note over Setup,RTOS: --- メインマルチコアタスク起動 ---
    Setup->>RTOS: xTaskCreatePinnedToCore(Core0_Task, Priority 6, Core 0)
    RTOS-->>C0_Task: Core0_Task 起動 (10ms周期の姿勢・音声ループ開始)

    Setup->>RTOS: xTaskCreatePinnedToCore(Core1_Task, Priority 5, Core 1)
    RTOS-->>C1_Task: Core1_Task 起動 (10ms周期の通信・キュー投入ループ開始)

    Setup->>Setup: Serial.println("Setup Done.")
    Note over Setup: loop() は空のため待機ループ (vTaskDelay 1000ms) へ移行
```

---

## 2. タスク間処理フローとキュー間通信図 (`flowchart TD`)

`Core0_Task` (Core 0) と `Core1_Task` (Core 1) は、それぞれ異なる CPU コア上で **10ms周期 (100Hz)** のメインループを維持しています。これら 2つのリアルタイム制御ループで計算・受信された全 54項目のデータが、`QueueHandle_t sdQueue` を介して非同期なストレージ書き込みタスク `SD_Task` へと受け渡される詳細な内部フローとデータ結合フローを以下に示します。

```mermaid
flowchart TD
    %% ================= Core 0 タスク =================
    subgraph Core0_Loop ["Core0_Task ループ (Core 0 / Priority 6 / 10ms 周期 / 100Hz)"]
        C0_Start["vTaskDelayUntil(10ms)<br>正確なループタイミング維持"]
        P_Loop["pitchsender_loop()"]
        LSM_Euler["imu_refresh_euler()<br>LSM6DSV16X FIFOクォータニオン解析"]
        LSM_Acc["imu_read_accel()<br>X, Y, Z 加速度取得"]
        Pitch_Eval["ピッチ角度判定<br>(TAIL_UP / LEVEL / TAIL_DOWN)"]
        BT_Tone["bt_set_sound(freq, interval)<br>Bluetooth A2DP 音階更新"]
        BNO_Loop["read_BNO()<br>オイラー角・クォータニオン・加速度取得"]
        BMP_Loop["read_bmp_fslg() -> calculate_bmp_altitude()<br>気圧温度取得＆標準大気高度計算"]
        BNO_Cal_Check{"BNO_counter > 100 ?<br>(約1秒おき)"}
        BNO_Cal["read_BNO_cal()<br>sys, gyro, accel, mag キャリブステータス更新"]
        GPS_Override["GPS座標更新<br>(にいじゅく未来公園デバッグ座標)"]
        Spk_Loop["run_speaker() -> speaker()<br>進入禁止区域判定 & TTC 計算により警報音出力"]

        C0_Start --> P_Loop
        P_Loop --> LSM_Euler --> LSM_Acc --> Pitch_Eval --> BT_Tone
        BT_Tone --> BNO_Loop --> BMP_Loop --> BNO_Cal_Check
        BNO_Cal_Check -->|Yes| BNO_Cal --> GPS_Override
        BNO_Cal_Check -->|No| GPS_Override
        GPS_Override --> Spk_Loop
        Spk_Loop --> C0_Start
    end

    %% ================= グローバル共有変数 =================
    subgraph Global_Mem ["SRAM 共有変数群 (volatile)"]
        GV_FSLG["胴体桁センサデータ<br>(BNO姿勢加速度 / BMP高度 / LSM姿勢加速度)"]
        GV_AIR["エアデータ & スピーカーフラグ<br>(GPS座標速度 / 対気速度 / 気圧高度 / RESET / CALIB)"]
    end

    %% ================= Core 1 タスク =================
    subgraph Core1_Loop ["Core1_Task ループ (Core 1 / Priority 5 / 10ms 周期 / 100Hz)"]
        C1_Start["vTaskDelay(10ms)"]
        Recv_UART["receiveLog()"]
        Listen_Bico{"Bico_UART.listenUART() &<br>parseBuffer == 30 ?"}
        Cmd_Parse["文字列コマンド判定<br>(RESET / SPK_EN / SPK_DIS / CHG_TO)"]
        Update_Air["UART_data[0..29] を<br>volatile エアデータ変数群へ格納"]
        LED2_Ctrl["LED2(GPIO25) 点灯制御"]
        Queue_Log["queueLogdata()"]
        Build_LogData["LogData 構造体へ全 54項目<br>(基本6 + Air18 + Fslg24 + Under6) を複製"]
        Q_Send["xQueueSend(sdQueue, &data, 0)"]
        Detect_Rst["detect_RESET_signal()<br>SPK_DISABLE により SPK_ENABLE リセット"]
        Trans_Log["transmitLog(transmit_counter)"]
        Trans_Mux{"transmit_counter == 0 ?<br>(10ms 毎に交互送信)"}
        Trans_M0["trans_mode = 0<br>BNO 加速度・姿勢・キャリブ (14項目) を Serial1 送信"]
        Trans_M1["trans_mode = 1<br>BMP・LSM 気圧高度・加速度・姿勢 (9項目) を Serial1 送信"]

        C1_Start --> Recv_UART --> Listen_Bico
        Listen_Bico -->|コマンド文字列検出| Cmd_Parse
        Listen_Bico -->|データ 30個正常受信| Update_Air --> LED2_Ctrl
        Listen_Bico -->|受信なし / エラー| Queue_Log
        Cmd_Parse --> Queue_Log
        LED2_Ctrl --> Queue_Log
        Queue_Log --> Build_LogData --> Q_Send --> Detect_Rst --> Trans_Log --> Trans_Mux
        Trans_Mux -->|Yes: counter=0| Trans_M0 --> C1_Start
        Trans_Mux -->|No: counter=1| Trans_M1 --> C1_Start
    end

    %% ================= SD Queue =================
    subgraph Queue_Area ["FreeRTOS キュー領域"]
        SD_Queue["sdQueue<br>(QueueHandle_t / 深さ 20 / struct LogData)"]
    end

    %% ================= SD タスク (Core 1) =================
    subgraph SD_Loop ["SD_Task ループ (Core 1 / Priority 4 / イベント駆動)"]
        Q_Recv["xQueueReceive(sdQueue, &receivedData, portMAX_DELAY)<br>キュー到達までブロック待機"]
        Check_Rst{"RESET_SIG == true ?"}
        Write_Rst["sd.add_str(RESET) / RESET_SIG = false"]
        Check_Cal{"CALIB_SIG == true ?"}
        Write_Cal["sd.add_str(CALIB) / CALIB_SIG = false"]
        Flash_Loop["mode = 0 から 3 まで反復<br>addDataToSDBuf(receivedData, mode)"]
        Format_SD["54項目を 4行 (13+11+14+16項目) の<br>CSV文字列に変換し SD_BUF へ格納"]
        Write_SD_Exe["writeSD()<br>LED1消灯 -> sd.flash() 書き込み -> LED1点灯"]

        Q_Recv --> Check_Rst
        Check_Rst -->|Yes| Write_Rst --> Check_Cal
        Check_Rst -->|No| Check_Cal
        Check_Cal -->|Yes| Write_Cal --> Flash_Loop
        Check_Cal -->|No| Flash_Loop
        Flash_Loop --> Format_SD --> Write_SD_Exe --> Q_Recv
    end

    %% Core 0 からグローバルへの書き込み
    LSM_Euler -->|"書き込み"| GV_FSLG
    LSM_Acc -->|"書き込み"| GV_FSLG
    BNO_Loop -->|"書き込み"| GV_FSLG
    BMP_Loop -->|"書き込み"| GV_FSLG
    BNO_Cal -->|"書き込み"| GV_FSLG

    %% Core 1 とグローバルの関係
    Update_Air -->|"書き込み"| GV_AIR
    Cmd_Parse -->|"書き込み (RESET_SIG 等)"| GV_AIR
    GV_FSLG -->|"読み出し (54項目複製)"| Build_LogData
    GV_AIR -->|"読み出し (54項目複製)"| Build_LogData
    GV_FSLG -->|"読み出し (送信データ取得)"| Trans_Log

    %% キュー間通信
    Q_Send -->|"LogData スナップショット投入"| SD_Queue
    SD_Queue -->|"LogData 取り出し"| Q_Recv
```

---

## 3. タスク・ループ設計の詳細解説

### 3.1 vTaskDelayUntil vs vTaskDelay の使い分けの意図
* `Core0_Task` では **`vTaskDelayUntil(&xLastWakeTime, xFrequency)`** を採用しています。これは「前回タスクが目を覚ました時刻から正確に 10ms (100Hz) 後に次回ループを開始する」制御構造であり、センサーのサンプリング周期 (積分時間) が変動すると積分誤差が蓄積する慣性航法計算（IMUのオイラー角・クォータニオン処理や高度微分計算）に必要な厳格な時間安定性を保証しています。
* `Core1_Task` では **`vTaskDelay(pdMS_TO_TICKS(10))`** を採用しています。Core 1 は UART からのバッファパース (`receiveLog`) のデータ量がパケット受信状況に応じて若干変動するため、処理終了後から一定時間 (10ms) 休眠させることで CPU 負荷を分散し、通信処理がバーストした際にも SD 書き込みタスクへ適度な実行タイムスライスを譲る設計にしています。

### 3.2 時分割送信 (`transmitLog`) の負荷分散
Core 1 から Bico 基板へセンサデータを返送する `transmitLog(transmit_counter)` は、1回で全 23項目を送ると UART TX バッファ (`460800 bps`) を占有して受信割り込みや SD キュー投入の遅延を引き起こす可能性があるため、**2フレーム時分割マルチプレクス**を行っています。
* 偶数回 (`transmit_counter == 0`): BNO055 加速度・クォータニオン・オイラー角・キャリブレーションステータスの **計14項目** (`trans_mode = 0`) を `sprintf` でフォーマットして送信。
* 奇数回 (`transmit_counter == 1`): BMP390 気圧・温度・高度および LSM6DSV16X 加速度・オイラー角の **計9項目** (`trans_mode = 1`) をフォーマットして送信。  
これにより 1フレームあたりの文字データ長を半減させ、通信帯域を平滑化しています。
