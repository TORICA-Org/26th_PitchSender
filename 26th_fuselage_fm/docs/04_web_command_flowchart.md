# 04_web_command_flowchart.md - 時分割配信・コマンド制御・フィードバック警報詳細

**プロジェクト**: 26th_Software_Fuselage (`26th_fuselage_fm`)  
**役割**: 胴体桁基板 (`ESP32 Devkit 32E`) における Bico 基板との時分割マルチプレクス送信 (`transmitLog`)、受信コマンド (`RESET`/`SPK_EN`/`CALIB`等) の制御シーケンス、およびパイロット向け Bluetooth A2DP ピッチ音声フィードバックと進入禁止区域 TTC 警告ブザー (`run_speaker`) の詳細メカニズムについて解説します。

*(※注: 本ドキュメントは `requirements.md` の第5項 `04_web_command_flowchart.md` に対応するドキュメントです。本プロジェクト `26th_fuselage_fm.ino` では Wi-Fi AP や電流電圧計 (`power_checker.cpp`) の代わりに、Bico との高頻度シリアルマルチプレクス通信、及びパイロット向け Bluetooth ワイヤレス音声フィードバック・圧電スピーカーによる空間警報制御を担うため、それらの詳細仕様書として構成します。)*

---

## 1. 時分割マルチプレクス送信制御 (`transmitLog`) のフローチャート (`flowchart TD`)

胴体桁基板は独自に取得している 23項目の高精度センサーデータ（BNO055 姿勢・加速度、LSM6DSV16X 姿勢・加速度、BMP390 高度）を `Serial1` (`460800 bps, 8E1`) で Bico 基板へと返信し、テレメトリーに合流させます。全ての文字列を一度に送ると送信バッファやタスク時間を圧迫するため、`Core1_Task` から 10ms 周期で呼ばれる `transmitLog(transmit_counter)` で **2ステップ時分割マルチプレクス送信** を行います。

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

## 2. コマンド制御・キャリブレーション処理のシーケンス図 (`sequenceDiagram`)

Bico 基板は、Web コンソールまたは操縦席スイッチ等から送信されたコマンドを UART データバッファ (`Bico_UART.buff`) の中に文字列コマンド (`RESET`, `SPK_EN`, `SPK_DIS`, `CHG_TO`) として混入させて胴体桁基板へ中継します。また、キャリブレーション信号 (`CALIB`) はグローバル変数経由で姿勢計算に介在します。これらを受信・検出した際の状態遷移と `SD_Task` との同期処理を以下に示します。

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

## 3. Bluetooth A2DP 音声フィードバック & 進入禁止区域 TTC 警報ロジック

本システムにおけるパイロットインターフェースは、「**姿勢を音階で伝える Bluetooth A2DP ワイヤレスイヤホン (`WF-C510`)**」と、「**進入禁止区域との距離感をトーンで警告する車載圧電スピーカー (`SPK`: GPIO13)**」という 2つの独立したフィードバック系で構築されています。

### 3.1 構成図・ピン接続と制御フロー (`flowchart TD`)

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

### 3.2 進入禁止区域判定 (`isInsideArea`) と最短到達時間 (`calc_time_to_reach`) の計算式解説

#### 1. エリア内外判定 (`isInsideArea`): 境界ベクトルと外積 Z による判定
反時計回りに定義されたポリゴン頂点配列 $B_0, B_1, \dots, B_{N-1}$（にいじゅく未来公園内 6頂点）に対し、各境界線ベクトル $\vec{B_{k-1} B_k}$ と機体位置ベクトル $\vec{B_{k-1} P}$ の 2次元外積 $Z$ を計算します。

$$Z = (B_k.\text{lon} - B_{k-1}.\text{lon}) \times (P.\text{lat} - B_{k-1}.\text{lat}) - (B_k.\text{lat} - B_{k-1}.\text{lat}) \times (P.\text{lon} - B_{k-1}.\text{lon})$$

* 判定基準: 全ての境界線 $k$ において $Z \ge 0$ の場合のみ **「飛行可能区域内 (`true`)」** と判定。1つでも $Z < 0$（境界線の右側）があれば **「禁止区域内 (`false`)」** とみなします。

#### 2. 最短到達時間計算 (`calc_time_to_reach`): 接近速度と距離の三角関数モデル
機体が対地速度 $v$、進行方位 $\alpha$ で飛行しているとき、方位角 $\theta_k$ の境界線ベクトルに向かう接近速度 $v_{\text{app}}$ と到達秒数 $t$ を求めます。

1. **接近速度 $v_{\text{app}}$**:  
   $$v_{\text{app}} = v \cdot \sin(\alpha - \theta_k)$$
   （※ $v_{\text{app}} \le 0.05\,\text{m/s}$ の場合は平行・遠ざかっているため除外）
2. **境界直線への垂直距離 $D_{\text{min}}$**:  
   終点 $B_k$ から機体 $P$ までの直線距離を $L_{B_k P}$、$\angle B_{k-1} B_k P = \phi_k$ としたとき、垂直最短距離は次式で計算されます。
   $$D_{\text{min}} = L_{B_k P} \cdot |\sin \phi_k|$$
3. **到達時間 $t$**:  
   $$t = \frac{D_{\text{min}}}{v_{\text{app}}}$$
   全境界線の中での最小値 $\min(t)$ を **TTC (Time To Contact)** として返します。

---

### 3.3 スピーカー警報 4段階モード決定表 (`speaker.cpp`)

| モード (`current_mode`) | 状態名称 | 判定条件 | ブザー周波数 (`sound_freq`) | 断続インターバル (`interval`) | 備考 |
| :---: | :--- | :--- | :--- | :--- | :--- |
| **`0`** | **停止 (Platform)** | `takeoff == false && SPK_ENABLE == false` | 無音 (`noTone`) | 停止 | 離陸前や地上待機中は音を鳴らさない |
| **`1`** | **禁止区域内 (Inside Alert)** | `takeoff == true` 且つ `isInsideArea == false` | **`440 Hz` (A4)** | **連続発音 (ピーーー)** | 最優先の緊急警報。即座に退避が必要 |
| **`2`** | **危険接近 (Approaching Alert)** | `takeoff == true` 且つ `0.0 < TTC < 6.0秒` | **`880 Hz` (A5)** | **連続発音 (ピーーー)** | 6秒以内に境界線へ到達する危険な接近状態 |
| **`3`** | **通常飛行 (Normal Flight)** | 区域内 (`isInsideArea == true`) 且つ安全 (`TTC >= 6.0` または遠ざかり) | 対気速度による3段階:<br>・`< 9.5 m/s`: **`440 Hz`**<br>・`9.5〜10.8 m/s`: **`880 Hz`**<br>・`>= 10.8 m/s`: **`1320 Hz`** | 高度による変化:<br>・`< 0.0m`: `100ms`<br>・`0.0〜1.5m`: `700〜125ms` 可変<br>・`>= 1.5m`: `900ms` | 機速・高度・上昇下降ペテンを音の高さと間隔でパイロットにフィードバックする通常動作 |
