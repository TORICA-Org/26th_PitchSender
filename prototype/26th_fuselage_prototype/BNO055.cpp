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
  bno.getCalibration(&data_fslg_bno_cal_system, &data_fslg_bno_cal_gyro, &data_fslg_bno_cal_accel, &data_fslg_bno_cal_mag); //system, gyro, accel, magの順番
}