/*---------------------------------------------------------

このファイルの役割：BNO055初期化動作・値読み取り
最終更新日：2026/04/11 00:39
更新内容：胴体桁電装向けに変数を変更

---------------------------------------------------------*/

#include <Arduino.h>
#include "BNO055.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "parameters.h"

// ESP32のFlash memoryにアクセスしてキャリブレーション情報を保存するため
#include <Preferences.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

bool BNO055_init(void){
  if(!bno.begin()){
    #ifdef DEBUG_MODE
    Serial.println("no BNO055 detected");
    #endif
    return false;
  }
  bno.setExtCrystalUse(true);
  return true;
}

void read_BNO(void){
  // オイラー角（roll,pitch,yaw）の取得
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  data_fslg_bno_yaw = euler.x();   // yaw角
  data_fslg_bno_roll = euler.y();  // roll角
  data_fslg_bno_pitch = euler.z(); // pitch角
  
  // クォータニオンを取得
  imu::Quaternion quat = bno.getQuat(); 
  data_fslg_bno_qw = quat.w(); 
  data_fslg_bno_qx = quat.x(); 
  data_fslg_bno_qy = quat.y(); 
  data_fslg_bno_qz = quat.z(); 

  // 加速度の取得
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  data_fslg_bno_accx_mss = accel.x(); // x方向の加速度
  data_fslg_bno_accy_mss = accel.y(); // y方向の加速度
  data_fslg_bno_accz_mss = accel.z(); // z方向の加速度
  
}


// 以下キャリブレーション関係

// オフセット値取得
/* Under construction */
/* オフセット値を取得＆フラッシュメモリ領域に書き込んで電源ON時に読み込ませる予定 */


// キャリブレーション状態取得
void read_BNO_cal(){
  uint8_t sys = 0, gyro = 0, accel = 0, mag = 0;
  bno.getCalibration(&sys, &gyro, &accel, &mag);

  data_fslg_bno_cal_system = sys;
  data_fslg_bno_cal_gyro   = gyro;
  data_fslg_bno_cal_accel  = accel;
  data_fslg_bno_cal_mag    = mag;
}

static Preferences prefs;
static bool isWritePermitted = false; // コマンドによるキャリブレーション情報書き込み許可フラグ
static String inputBuffer = "";       // シリアル受信用バッファ（コマンド用）

static void checkSerialCommand();
static void saveOffsetsToNVS(Adafruit_BNO055 &bno);

void BNO_Calib_init(){
  prefs.begin("bno_data", true); // 読み取り専用でflash読み取り
  if (prefs.isKey("offsets")) {
    adafruit_bno055_offsets_t savedOffsets;
    prefs.getBytes("offsets", &savedOffsets, sizeof(savedOffsets));
    bno.setSensorOffsets(savedOffsets);
    Serial.println("Calibration data restored");
  } else {
    Serial.println("No calibration data saved");
  }
  prefs.end();
}


void BNO_Calib(uint8_t sys, uint8_t gyro, uint8_t accel, uint8_t mag) {
  checkSerialCommand();
  if (isWritePermitted == true){
    Serial.printf("sys: %2d | accel: %2d | gyro: %2d | mag: %2d\n", sys, accel, gyro, mag); // キャリブレーション状態を表示
  }
  
  // 書き込み許可があり，全キャリブレーションが3の時のみ保存
  if (isWritePermitted && sys == 3 && gyro == 3 && accel == 3 && mag == 3) {
    saveOffsetsToNVS(bno);
    isWritePermitted = false; // 保存完了後に許可フラグを解除（次のBNO_CALIB受信まで待機）
  }
}

static void checkSerialCommand() {
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      inputBuffer.trim();
      if (inputBuffer.length() > 0) {
        if (inputBuffer == "BNO_CALIB") {
          isWritePermitted = true; // コマンド受信ごとに書き込み許可をセット
          Serial.println("\n[COMMAND] 'Calibration data flashing enabled");
        }
        inputBuffer = "";
      }
    } else {
      inputBuffer += c;
    }
  }
}

// NVSへオフセット情報を保存する内部関数
static void saveOffsetsToNVS(Adafruit_BNO055 &bno) {
  adafruit_bno055_offsets_t newOffsets;
  if (bno.getSensorOffsets(newOffsets)) {
    prefs.begin("bno_data", false); // 書き込みモード
    prefs.putBytes("offsets", &newOffsets, sizeof(newOffsets));
    prefs.end();
    Serial.println("\n [FLASHING SUCCESS] CALIBRATION DATA STORED");
  }
}
