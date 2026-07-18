# 05_drawio_mermaid_snippets.md - draw.io 貼り付け専用 Mermaid コード集

**プロジェクト**: 26th_Software_Fuselage (`26th_fuselage_fm`)  
**用途**: 本ドキュメントは、説明テキストやテーブルを排除し、draw.io (`app.diagrams.net`) の「挿入 > 高度な設定 > Mermaid...」からコピー＆ペーストしてそのまま図解を生成できるよう、厳格な draw.io 互換ルール（エッジラベルのカッコのダブルクォート囲み、標準ボックス `["..."]` 統一、特殊文字のクォート等）に準拠した純粋な Mermaid コードのみを収録しています。

---

## 1. システム全体アーキテクチャ図 (`doc/README.md`)

```mermaid
flowchart TD
    %% 外部基板・クライアント (External)
    subgraph External ["外部デバイス・インターフェース"]
        Bico_Board["Bico エアデータ中継基板<br>(UART 460800 bps / 8E1)"]
        BT_Earphone["Bluetooth A2DP イヤホン<br>(WF-C510等)"]
        Buzzer_SPK["圧電スピーカー / ブザー<br>(SPK GPIO13)"]
        LSM_IMU["LSM6DSV16X 6軸IMU<br>(I2C 0x6A/0x6B)"]
        BNO_IMU["BNO055 9軸IMU<br>(I2C 0x28)"]
        BMP_Sensor["BMP390 気圧温度センサ<br>(I2C 0x76)"]
        SD_Card["microSD カード<br>(SPI CS: GPIO16)"]
    end

    %% グローバルデータ・共有領域 (Globals & Queues)
    subgraph Globals ["グローバル変数 & キュー共有領域 (SRAM)"]
        G_Vars["volatile グローバル変数群<br>(parameters.h/.cpp)<br>エアデータ30項目 / 胴体桁24項目"]
        SD_Q["sdQueue (QueueHandle_t)<br>深さ: 20 / 要素: struct LogData"]
    end

    %% Core 0 タスク (10Hz / 100ms ループ & リアルタイムオーディオ)
    subgraph Core0 ["Core 0: 姿勢推定・高度・音声フィードバックタスク (Priority 6)"]
        C0_Task["Core0_Task (10ms周期ループ)"]
        Pitch_Loop["pitchsender_loop()"]
        LSM_Read["imu_refresh_euler() / imu_read_accel()"]
        BNO_Read["read_BNO() / read_BNO_cal()"]
        BMP_Read["read_bmp_fslg() / calculate_bmp_altitude()"]
        Spk_Check["run_speaker()"]
        Zone_Check["isInsideArea() / calc_time_to_reach()"]
        BT_Audio["bt_set_sound() -> A2DPコールバック"]
    end

    %% Core 1 タスク (100Hz / 10ms ループ & 通信ロギング)
    subgraph Core1 ["Core 1: UART通信・コマンド処理・SD記録制御タスク (Priority 5 & 4)"]
        C1_Task["Core1_Task (10ms周期ループ)"]
        UART_Recv["receiveLog()"]
        UART_Trans["transmitLog() (2モード時分割)"]
        Cmd_Detect["detect_RESET_signal() / コマンド解析"]
        Queue_Send["queueLogdata()"]
        SD_Worker["SD_Task (Priority 4)<br>非同期 SD 書き込みループ"]
        SD_Write["addDataToSDBuf() -> writeSD()"]
    end

    %% 外部から Core 0 ドライバへの接続
    LSM_IMU <-->|"I2C(Wire)"| LSM_Read
    BNO_IMU <-->|"I2C(Wire)"| BNO_Read
    BMP_Sensor <-->|"I2C(Wire)"| BMP_Read
    BT_Audio -->|"A2DP Audio Stream"| BT_Earphone
    Spk_Check -->|"tone() / noTone()"| Buzzer_SPK

    %% Core 0 内部フローとグローバル更新
    C0_Task --> Pitch_Loop
    C0_Task --> BNO_Read
    C0_Task --> BMP_Read
    C0_Task --> Spk_Check
    Pitch_Loop --> LSM_Read
    Pitch_Loop -->|"姿勢角 (roll, pitch, yaw) に応じた周波数決定"| BT_Audio
    LSM_Read -->|"書き込み"| G_Vars
    BNO_Read -->|"書き込み"| G_Vars
    BMP_Read -->|"書き込み"| G_Vars
    Spk_Check --> Zone_Check
    G_Vars -->|"GPS座標・速度参照"| Zone_Check

    %% 外部と Core 1 の接続
    Bico_Board -->|"Serial1 受信 (30項目 / コマンド)"| UART_Recv
    UART_Trans -->|"Serial1 送信 (14項目 / 9項目)"| Bico_Board
    SD_Write -->|"SPI(TORICA_SD.flash)"| SD_Card

    %% Core 1 内部フローとグローバル・キュー連携
    C1_Task --> UART_Recv
    C1_Task --> Queue_Send
    C1_Task --> Cmd_Detect
    C1_Task --> UART_Trans
    UART_Recv -->|"エアデータ30項目書き込み / フラグ制御"| G_Vars
    UART_Recv -->|"コマンド解析 (RESET / CALIB等)"| Cmd_Detect
    G_Vars -->|"54項目スナップショット読み出し"| Queue_Send
    G_Vars -->|"胴体桁データ読み出し"| UART_Trans
    Queue_Send -->|"xQueueSend(sdQueue)"| SD_Q
    SD_Q -->|"xQueueReceive(sdQueue)"| SD_Worker
    SD_Worker --> SD_Write
```

---

## 2. インクルード依存関係グラフ (`doc/01_file_relationships.md`)

```mermaid
flowchart TD
    subgraph Main ["メインエントリ"]
        INO["26th_fuselage_fm.ino"]
    end

    subgraph Configs ["共通パラメータ & ピン設定"]
        PARAM["parameters.h / .cpp<br>(volatile変数 / struct LogData)"]
        CONF["fslg_config.h / .cpp<br>(GPIOピン定義)"]
    end

    subgraph Core0_Modules ["Core 0: 音声・センサ・警報モジュール"]
        PW["pitchsender_wrapper.h / .cpp"]
        IMU["MyIMU.h / .cpp<br>(LSM6DSV16X)"]
        BT["MyBluetooth.h / .cpp<br>(BluetoothA2DPSource)"]
        FREQ["Frequency.h / .cpp"]
        BNO["BNO055.h / .cpp"]
        BMP["BMP3xx.h / .cpp"]
        ALT["calculate_altitude.h / .cpp"]
        SW["speaker_wrapper.h / .cpp"]
        SPK["speaker.h / .cpp"]
        ZONE["check_restricted_zone.h / .cpp<br>(TinyGPS++)"]
    end

    subgraph Core1_Modules ["Core 1: UART通信・SDロギングモジュール"]
        SDW["SD_wrapper.h / .cpp<br>(sdQueue / SD_Task)"]
        SDF["SD_fslg.h / .cpp<br>(TORICA_SD)"]
        UART["UARTHelper_fslg.h / .cpp<br>(TORICA_UART / Serial1)"]
    end

    %% INOからのインクルード
    INO -->|"#include"| CONF
    INO -->|"#include"| PARAM
    INO -->|"#include"| PW
    INO -->|"#include"| BNO
    INO -->|"#include"| BMP
    INO -->|"#include"| ALT
    INO -->|"#include"| SW
    INO -->|"#include"| SDW
    INO -->|"#include"| UART
    INO -->|"#include"| IMU

    %% Core 0 内部のインクルード関係
    PW -->|"#include"| CONF
    PW -->|"#include"| IMU
    PW -->|"#include"| FREQ
    PW -->|"#include"| BT
    IMU -->|"#include"| PARAM
    BNO -->|"#include"| PARAM
    BMP -->|"#include"| PARAM
    ALT -->|"#include"| PARAM
    SW -->|"#include"| PARAM
    SW -->|"#include"| SPK
    SW -->|"#include"| ZONE
    SPK -->|"#include"| CONF
    SPK -->|"#include"| PARAM
    SPK -->|"#include"| ZONE

    %% Core 1 内部のインクルード関係
    SDW -->|"#include"| CONF
    SDW -->|"#include"| PARAM
    SDW -->|"#include"| SDF
    SDF -->|"#include"| CONF
    SDF -->|"#include"| PARAM
    UART -->|"#include"| CONF
    UART -->|"#include"| PARAM
```

---

## 3. 初期化シーケンス図 (`doc/02_core_tasks_flowchart.md`)

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

## 4. タスク間処理フローとキュー間通信図 (`doc/02_core_tasks_flowchart.md`)

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

## 5. データパイプライン関数呼び出しフロー図 (`doc/03_data_pipeline_flowchart.md`)

```mermaid
flowchart TD
    %% 物理層受信
    subgraph Physics_Layer ["物理通信・センサ取得層 (Layer 1)"]
        Serial_Bico["Serial_Bico (Serial1 / 460800 bps)<br>UART ハードウェアバッファ"]
        LSM_Sensor["LSM6DSV16X FIFO (I2C 0x6A/0x6B)"]
        BNO_Sensor["BNO055 レジスタ (I2C 0x28)"]
        BMP_Sensor["BMP390 レジスタ (I2C 0x76)"]
    end

    %% Bico UART 受信パイプライン (Core 1)
    subgraph UART_Pipeline ["Bico UART パース & 反映パイプライン (receiveLog / Core 1)"]
        Listen_Func["Bico_UART.listenUART()<br>受信確認・バッファ読み取り"]
        Parse_Func["Bico_UART.parseBuffer(Bico_UART.buff)<br>カンマ区切り文字列から浮動小数/整数変換"]
        Extract_Func["extractLogData / convertArrayToLogData<br>(UART_data[0..29] の配列要素を抽出)"]
        Apply_Func["applyLogDataToGlobals<br>(volatile グローバル変数への一括格納と生存確認 bico_is_alive)"]
    end

    %% 胴体桁基板センサ計算パイプライン (Core 0)
    subgraph Fuselage_Pipeline ["胴体桁基板センサ解析パイプライン (Core 0_Task)"]
        LSM_Calc["imu_refresh_euler() / imu_read_accel()<br>クォータニオン正規化 & 物理値換算"]
        BNO_Calc["read_BNO() / read_BNO_cal()<br>9軸オイラー角・加速度取得"]
        BMP_Calc["read_bmp_fslg() -> calculate_bmp_altitude()<br>気圧温度取得＆標準大気式高度計算"]
    end

    %% グローバル領域とキュー投入
    subgraph Global_Ref ["グローバル変数 & キュー投入層 (Layer 3 & 2)"]
        GV_Store["volatile グローバル変数群 (parameters.h)<br>Air 18項目 + Core 6項目 + Fuselage 24項目 + Under 6項目"]
        Queue_Build["queueLogdata() (SD_wrapper.cpp)<br>LogData 構造体インスタンスへの全 54項目コピー"]
        Q_Push["xQueueSend(sdQueue, &data, 0)"]
    end

    %% SD 保存パイプライン (SD_Task / Core 1)
    subgraph SD_Pipeline ["非同期 SD ロギングパイプライン (SD_Task / Core 1 / Priority 4)"]
        Q_Pop["xQueueReceive(sdQueue, &receivedData, portMAX_DELAY)"]
        Add_Buf_Loop["addDataToSDBuf(receivedData, flash_mode)<br>mode = 0, 1, 2, 3 で 4回分割実行"]
        Fmt_M0["mode=0: snprintf (時間/離陸/GPS等 13項目)"]
        Fmt_M1["mode=1: snprintf (フィルタ高度速度/気圧/差圧等 11項目)"]
        Fmt_M2["mode=2: snprintf (生存/BNO姿勢/LSM姿勢/気圧等 14項目)"]
        Fmt_M3["mode=3: snprintf (加速度/キャリブ/Under等 16項目)"]
        Add_Str["sd.add_str(SD_BUF)<br>TORICA_SD 内部バッファへ追加"]
        Write_SD_Func["writeSD()<br>LED1消灯 -> sd.flash() (SPI物理書込) -> LED1点灯"]
    end

    %% フロー接続
    Serial_Bico --> Listen_Func
    Listen_Func --> Parse_Func
    Parse_Func --> Extract_Func
    Extract_Func --> Apply_Func
    Apply_Func -->|"書き込み (エアデータ 30項目)"| GV_Store

    LSM_Sensor --> LSM_Calc
    BNO_Sensor --> BNO_Calc
    BMP_Sensor --> BMP_Calc
    LSM_Calc -->|"書き込み"| GV_Store
    BNO_Calc -->|"書き込み"| GV_Store
    BMP_Calc -->|"書き込み"| GV_Store

    GV_Store --> Queue_Build
    Queue_Build --> Q_Push --> Q_Pop
    Q_Pop --> Add_Buf_Loop
    Add_Buf_Loop --> Fmt_M0 --> Add_Str
    Add_Buf_Loop --> Fmt_M1 --> Add_Str
    Add_Buf_Loop --> Fmt_M2 --> Add_Str
    Add_Buf_Loop --> Fmt_M3 --> Add_Str
    Add_Str --> Write_SD_Func
```

---

## 6. 時分割マルチプレクス送信フローチャート (`doc/04_web_command_flowchart.md`)

```mermaid
flowchart TD
    Trans_Start["transmitLog(trans_mode)<br>Core1_Task より 10ms ごとに実行"]
    Eval_Mode{"trans_mode の値判定<br>(static counter: 0 または 1)"}

    %% モード 0: BNO 姿勢・キャリブ (14項目)
    Mode0_Exec["trans_mode == 0<br>(BNO055 姿勢・加速度・キャリブ計 14項目)"]
    M0_Fmt["sprintf(trans_buff, AccX, AccY, AccZ, Qw, Qx, Qy, Qz,<br>Roll, Pitch, Yaw, CalSys, CalGyro, CalAcc, CalMag)"]

    %% モード 1: BMP・LSM 加速度・姿勢 (9項目)
    Mode1_Exec["trans_mode == 1<br>(BMP390・LSM6DSV16X 計 9項目)"]
    M1_Fmt["sprintf(trans_buff, BmpPress, BmpTemp, BmpAlt,<br>LsmAccX, LsmAccY, LsmAccZ, LsmRoll, LsmPitch, LsmYaw)"]

    %% 範囲外エラー
    Mode_Err["default: 範囲外エラー<br>Serial.println(The parameter value is out of range)"]

    %% 送信実行
    Serial_Flush["Serial_Bico.flush()<br>送信完了待ち＆バッファクリア"]
    Serial_Print["Serial_Bico.print(trans_buff)<br>文字列を 460800 bps で Bico 基板へ送信"]
    Counter_Inc["transmit_counter++<br>もし > 1 なら 0 へリセット"]

    Trans_Start --> Eval_Mode
    Eval_Mode -->|trans_mode == 0| Mode0_Exec --> M0_Fmt --> Serial_Flush
    Eval_Mode -->|trans_mode == 1| Mode1_Exec --> M1_Fmt --> Serial_Flush
    Eval_Mode -->|それ以外| Mode_Err
    Serial_Flush --> Serial_Print --> Counter_Inc
```

---

## 7. コマンド制御・キャリブレーション処理シーケンス図 (`doc/04_web_command_flowchart.md`)

```mermaid
sequenceDiagram
    autonumber
    actor Bico as Bico エアデータ基板 (UART)
    participant Recv as receiveLog() (Core 1)
    participant Global as volatile グローバル変数 / parameters.cpp
    participant LSM as to_euler_angles() (Core 0)
    participant SD as SD_Task (Core 1)

    %% 1. RESET コマンド受信
    Note over Bico,SD: --- ログ区切り・リセットコマンド ---
    Bico->>Recv: 文字列中に "RESET" を含むパケット送信
    Recv->>Global: RESET_SIG = true
    Note right of Recv: queueLogdata() を経由して SD_Task へ変化伝搬
    SD->>Global: RESET_SIG が true であることを検出
    SD->>SD: sd.add_str("\nRESET\n") を SD バッファへ追加
    SD->>Global: RESET_SIG = false にクリア

    %% 2. スピーカー ON/OFF コマンド
    Note over Bico,SD: --- スピーカー強制 ON/OFF コマンド ---
    Bico->>Recv: 文字列中に "SPK_EN" または "SPK_DIS" を送信
    alt "SPK_EN" 受信時
        Recv->>Global: SPK_ENABLE = true
        Note right of Global: speaker() にて強制的に音を発出する状態へ遷移
    else "SPK_DIS" 受信時
        Recv->>Global: SPK_DISABLE = true
        Recv->>Global: SPK_ENABLE = false / SPK_DISABLE = false に即時クリア
    end

    %% 3. 離陸判定反転 (CHG_TO)
    Note over Bico,SD: --- 離陸判定フラグ反転コマンド ---
    Bico->>Recv: 文字列中に "CHG_TO" (Change Takeoff) を送信
    Recv->>Global: takeoff = !takeoff (現在の bool 状態を反転)

    %% 4. 姿勢キャリブレーション (CALIB)
    Note over Bico,SD: --- 6軸 IMU ゼロ点キャリブレーション ---
    Note over Global,LSM: 外部から CALIB = true がトリガーされた場合
    LSM->>Global: CALIB が true であることを検出
    LSM->>LSM: roll_offset = raw_roll / pitch_offset = raw_pitch / yaw_offset = raw_yaw を設定
    LSM->>Global: CALIB = false にクリア / CALIB_SIG = true をセット
    SD->>Global: CALIB_SIG が true であることを検出
    SD->>SD: sd.add_str("\nCALIB\n") を SD バッファへ追加 (キャリブ履歴記録)
    SD->>Global: CALIB_SIG = false にクリア
```

---

## 8. Bluetooth A2DP 音声＆進入禁止区域警報フローチャート (`doc/04_web_command_flowchart.md`)

```mermaid
flowchart TD
    subgraph Core0_Feedback ["Core 0 タスク：オーディオ＆スピーカー制御ループ"]
        P_Loop["pitchsender_loop()<br>10ms 周期ループ"]
        LSM_Angles["angles.pitch 評価<br>(TOLERANCE = ±0.2度)"]
        Att_Eval{"姿勢判定と音階選択"}
        Att_UP["TAIL_UP (機首下げ・尾翼上げ)<br>Freq: G5 (783.99Hz) / Inter: 0.05秒"]
        Att_LV["LEVEL (水平飛行)<br>Freq: C5 (523.25Hz) / Inter: 0.5秒"]
        Att_DN["TAIL_DOWN (機首上げ・尾翼下げ)<br>Freq: G5 (783.99Hz) / Inter: 0.1秒"]
        Set_BT["bt_set_sound(freq, interval)<br>target_deltaAngle / target_samples_interval 更新"]
        
        Run_Spk["run_speaker()<br>10ms 周期ループ"]
        Zone_Eval["isInsideArea(lat, lon)<br>ポリゴン6頂点と外積計算"]
        TTC_Eval["calc_time_to_reach(lat, lon, gs, head)<br>最短到達時間 TTC(秒) 算出"]
        Spk_Func["speaker(airspeed, altitude, takeoff, isInside, TTC)<br>警報モード決定"]
    end

    subgraph Hardware_Output ["ハードウェア出力層 (Layer 1 & 0)"]
        A2DP_Callback["get_data_frames() A2DP コールバック<br>SBC エンコード & フェードイン/アウト波形生成"]
        BT_Audio_Out["Bluetooth A2DP イヤホン<br>(パイロット耳元へピッチ断続音出力)"]
        Tone_Ctrl["tone(SPK, freq) / noTone(SPK)<br>GPIO13 矩形波出力"]
        SPK_Device["圧電スピーカー / ブザー<br>(機体コックピット内へ警報音出力)"]
    end

    P_Loop --> LSM_Angles --> Att_Eval
    Att_Eval -->|pitch < -0.2| Att_UP --> Set_BT
    Att_Eval -->|-0.2 <= pitch <= 0.2| Att_LV --> Set_BT
    Att_Eval -->|pitch > 0.2| Att_DN --> Set_BT
    Set_BT -->|"volatile 変数共有"| A2DP_Callback --> BT_Audio_Out

    Run_Spk --> Zone_Eval
    Run_Spk --> TTC_Eval
    Zone_Eval --> Spk_Func
    TTC_Eval --> Spk_Func
    Spk_Func -->|"current_mode: 1, 2, 3 に応じた<br>tone / noTone 制御"| Tone_Ctrl --> SPK_Device
```

---

## 9. 4層レイヤー構造・関数ヒエラルキー図 (`doc/06_layer_hierarchy_flowchart.md`)

```mermaid
flowchart TD
    %% =========================================================================
    %% Layer 3: 抽象データロジック＆アプリケーション計算層 (最も高い抽象度)
    %% =========================================================================
    subgraph Layer3 ["Layer 3: 抽象データロジック＆アプリケーション計算層"]
        L3_LogBuild["queueLogdata()<br>(スナップショット構築)"]
        L3_SDBuf["addDataToSDBuf()<br>(54項目 4分割 CSV 生成)"]
        L3_Euler["to_euler_angles() & normalize()<br>(クォータニオン正規化・オイラー角計算)"]
        L3_AltCalc["calculate_bmp_altitude()<br>(標準大気気圧高度計算)"]
        L3_FreqGet["frequency_get()<br>(音階文字列 -> 周波数換算)"]
        L3_Zone["isInsideArea() & calc_time_to_reach()<br>(ポリゴン外積内外判定 & TTC 最短到達秒数計算)"]
    end

    %% =========================================================================
    %% Layer 2: 中間ラッパー＆タスク・キュー制御層
    %% =========================================================================
    subgraph Layer2 ["Layer 2: 中間ラッパー＆タスク・キュー制御層"]
        L2_C0["Core0_Task()<br>(Core 0 メインループ 100Hz)"]
        L2_C1["Core1_Task()<br>(Core 1 メインループ 100Hz)"]
        L2_SD["SD_Task()<br>(Core 1 ロギングループ Priority 4)"]
        
        L2_PitchLoop["pitchsender_loop()<br>(姿勢＆音程フィードバックループ)"]
        L2_PitchInit["pitchsender_init()<br>(IMU＆BT 初期化ラッパー)"]
        L2_SpkRun["run_speaker()<br>(エリア警報統括ラッパー)"]
        L2_InitSDTask["initSDTask()<br>(キュー生成＆SDタスク起動ラッパー)"]
        
        L2_RecvLog["receiveLog()<br>(Bico UART 受信・コマンド解析ラッパー)"]
        L2_TransLog["transmitLog()<br>(Bico UART 2モード時分割送信ラッパー)"]
        L2_DetectRst["detect_RESET_signal()<br>(スピーカー制御リセット)"]
    end

    %% =========================================================================
    %% Layer 1: ハードウェアドライバ＆I/Oプロトコル層
    %% =========================================================================
    subgraph Layer1 ["Layer 1: ハードウェアドライバ＆I/Oプロトコル層"]
        L1_LSM["imu_init() / imu_refresh_euler() / imu_read_accel()<br>(LSM6DSV16X I2C ドライバ & FIFO読み取り)"]
        L1_Reg["writeReg() / readRegs()<br>(I2C レジスタ読み書きドライバ)"]
        L1_BNO["BNO055_init() / read_BNO() / read_BNO_cal()<br>(Adafruit BNO055 9軸センサドライバ)"]
        L1_BMP["BMP3XX_init() / read_bmp_fslg()<br>(Adafruit BMP3XX 気圧センサドライバ)"]
        L1_BT["bt_init() / bt_set_sound() / get_data_frames()<br>(Bluetooth A2DP ドライバ & エンベロープ合成)"]
        L1_SPK["speaker_init() / speaker()<br>(圧電スピーカー PWM ドライバ)"]
        L1_SD["initSD() / flashHeader() / writeSD()<br>(TORICA_SD クラス SPI ストレージドライバ)"]
        L1_UART["initUART() / Bico_UART.listenUART() / parseBuffer()<br>(TORICA_UART コア通信ドライバ)"]
    end

    %% =========================================================================
    %% Layer 0: 物理ハードウェア・ピン＆RTOSカーネル層 (最も低い抽象度)
    %% =========================================================================
    subgraph Layer0 ["Layer 0: 物理ハードウェア・ピン＆RTOSカーネル層"]
        L0_Pins["物理 GPIO ピン配置<br>UART: 32/33 | I2C: 21/22 | SPI: 16/18/19/23 | SPK: 13 | LED: 3/4/25"]
        L0_Periph["シリアル・無線ペリフェラル<br>Serial1 (UART) | Wire (I2C) | SPI | BluetoothA2DPSource"]
        L0_RTOS["FreeRTOS カーネル・OS プリミティブ<br>xTaskCreatePinnedToCore / xQueueCreate / xQueueSend / xQueueReceive"]
    end

    %% =================== 垂直呼び出し関係 (Layer 2 -> Layer 3) ===================
    L2_C1 -->|"呼び出し"| L3_LogBuild
    L2_SD -->|"呼び出し"| L3_SDBuf
    L2_C0 -->|"呼び出し"| L3_AltCalc
    L2_PitchLoop -->|"呼び出し"| L3_FreqGet
    L2_SpkRun -->|"呼び出し"| L3_Zone

    %% =================== 垂直呼び出し関係 (Layer 2 -> Layer 1) ===================
    L2_PitchInit -->|"呼び出し"| L1_LSM
    L2_PitchInit -->|"呼び出し"| L1_BT
    L2_PitchLoop -->|"呼び出し"| L1_LSM
    L2_PitchLoop -->|"呼び出し"| L1_BT
    L1_LSM -->|"内部呼び出し"| L3_Euler
    L1_LSM -->|"内部呼び出し"| L1_Reg
    
    L2_C0 -->|"呼び出し"| L1_BNO
    L2_C0 -->|"呼び出し"| L1_BMP
    L2_C0 -->|"呼び出し"| L2_PitchLoop
    L2_C0 -->|"呼び出し"| L2_SpkRun
    L2_SpkRun -->|"呼び出し"| L1_SPK

    L2_InitSDTask -->|"呼び出し"| L1_SD
    L2_RecvLog -->|"呼び出し"| L1_UART
    L2_TransLog -->|"呼び出し"| L0_Periph
    L2_C1 -->|"呼び出し"| L2_RecvLog
    L2_C1 -->|"呼び出し"| L2_TransLog
    L2_C1 -->|"呼び出し"| L2_DetectRst
    L2_SD -->|"呼び出し"| L1_SD

    %% =================== 垂直呼び出し関係 (Layer 1 -> Layer 0) ===================
    L1_Reg -->|"I2C 物理通信"| L0_Periph
    L1_BNO -->|"I2C 物理通信"| L0_Periph
    L1_BMP -->|"I2C 物理通信"| L0_Periph
    L1_BT -->|"A2DP オーディオ出力"| L0_Periph
    L1_SPK -->|"tone / noTone 矩形波出力"| L0_Pins
    L1_SD -->|"SPI フラッシュ書き出し"| L0_Pins
    L1_UART -->|"Serial1 物理受信"| L0_Pins

    %% =================== RTOS とタスク・キュー結合 (Layer 0 -> Layer 2/3) ===================
    L0_RTOS -->|"タスク生成＆スケジューリング"| L2_C0
    L0_RTOS -->|"タスク生成＆スケジューリング"| L2_C1
    L0_RTOS -->|"タスク生成＆スケジューリング"| L2_SD
    L2_InitSDTask -->|"xQueueCreate()"| L0_RTOS
    L3_LogBuild -->|"xQueueSend(sdQueue)"| L0_RTOS
    L2_SD -->|"xQueueReceive(sdQueue)"| L0_RTOS
```
