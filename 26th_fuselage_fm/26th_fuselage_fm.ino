/*

Core0: PitchSender, IMU×2，BMP読み取り

Core1: UART受信，SD，スピーカー

LED1: SD_fslg.cppで使用
LED2: UARTHelper_fslg.cppで使用

*/

#include "fslg_config.h"          // 胴体桁基板のピン設定
#include "pitchsender_wrapper.h"
#include "BNO055.h"
#include "SD_wrapper.h"
#include "UARTHelper_fslg.h"
#include "parameters.h"
#include "speaker_wrapper.h"
#include "BMP3xx.h"


TaskHandle_t thp[2];  // マルチスレッドのタスクハンドル格納用

// デバッグ用タスクマネージャー
void printTaskStats() {
  // 統計情報を格納するバッファ
  char statsBuffer[1024];
  
  // FreeRTOSのランタイム統計を取得
  vTaskGetRunTimeStats(statsBuffer);
  
  Serial.println("=========================================");
  Serial.println("Task Name       Abs Time (us)   % Time");
  Serial.println("=========================================");
  Serial.println(statsBuffer);
}


void setup() {
  Serial.begin(115200);  // デバッグ用にパリティはいらないかな...ってか使えない気がする
  Serial.print("loading...\n\n");

  // インジケータランプ初期化
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  // Core0タスク初期化処理
  pitchsender_init(); // pichsenderは6軸IMU(LSM)を使う．LSMの初期化はpitchsender_init()内で行われている．
  BNO055_init();// BNOは記録のみで，pitchsenderには使わない．
  BMP3XX_init();

  // Core1タスク初期化処理
  initSDTask();  // SDカードの初期化
  initUART();

  xTaskCreatePinnedToCore(Core0_Task, "Core0_Task", 4096, NULL, 6, NULL, 0);

  xTaskCreatePinnedToCore(Core1_Task, "Core1_Task", 4096, NULL, 5, NULL, 1);

  // SD_wrapper.cpp/.h内にあるSD_Task()もxTaskCreatePinnedToCore()で呼ばれる．

  Serial.println("Setup Done.");
}


// FreeRTOSにタスクを管理してもらうので，loop()は空
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void Core0_Task(void *args){
  TickType_t xLastWakeTime = xTaskGetTickCount();     // タスクの開始時間を取得
  const TickType_t xFrequency = pdMS_TO_TICKS(10);  // 10ms周期で実行する

  while(1){
    vTaskDelayUntil(&xLastWakeTime, xFrequency);  // 10ms周期で実行する

    pitchsender_loop();
    read_BNO();
    read_bmp_fslg();

    // BNOのキャリブレーション状態は1秒おきで取得．
    static uint32_t BNO_counter = 0;
    if (BNO_counter > 100){
      BNO_counter = 0;
      read_BNO_cal();
    } else {
      BNO_counter++;
    }

    // For debug
    takeoff = true;
    urm_is_reliable = true;
    data_under_urm_altitude_m = 0.6;
    data_air_sdp_airspeed_ms = 10.0;

    // にいじゅく未来公園のど真ん中の座標
    data_air_gps_latitude_deg = 35.461432;
    data_air_gps_longitude_deg = 139.514524;

    run_speaker();

    // デバッグ用
    printTaskStats();
    // Serial.println(data_fslg_lsm_roll);
    // Serial.println("Core0 running");

  }
}



void Core1_Task(void *args){
  while(1){
    vTaskDelay(pdMS_TO_TICKS(10));  // 10ms周期で実行する

    receiveLog();

    queueLogdata();

    static uint8_t transmit_counter = 0;
    transmitLog(transmit_counter);
    transmit_counter++;
    if (transmit_counter > 1){
      transmit_counter = 0;
    }

  }
}
