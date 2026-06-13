/*---------------------------------------------------------

このファイルの役割：BNO055初期化動作・値読み取り
最終更新日：2026/04/11 00:39
更新内容：胴体桁電装向けに変数を変更

---------------------------------------------------------*/

#include <Arduino.h>
#include "BNO055.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

bool bno_init(void){
  if(!bno.begin()){
    Serial.println("no BNO055 detected");
    return false;
  }
  bno.setExtCrystalUse(true);
  Serial.println("BNO055 initialized.");
  return true;
}

imu::Vector<3> euler;
// imu::Quaternion quat;
// imu::Vector<3> accel;

volatile EulerAngles bno_angles;

void bno_read(void){
  // オイラー角（roll,pitch,yaw）の取得
  euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  // angles.yaw = euler.x();   // yaw角
  // angles.roll = euler.y();  // roll角
  // angles.pitch = euler.z(); // pitch角
  bno_angles.yaw = euler.x();   // yaw角
  bno_angles.pitch = euler.y();  // roll角
  bno_angles.roll = euler.z(); // pitch角
  
  // クォータニオンを取得
  // quat = bno.getQuat(); 
  // data_psd_bno_qw = quat.w(); 
  // data_psd_bno_qx = quat.x(); 
  // data_psd_bno_qy = quat.y(); 
  // data_psd_bno_qz = quat.z(); 

  // 加速度の取得
  // accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  // data_psd_bno_accx_mss = accel.x(); // x方向の加速度
  // data_psd_bno_accy_mss = accel.y(); // y方向の加速度
  // data_psd_bno_accz_mss = accel.z(); // z方向の加速度
  
}


// 以下キャリブレーション関係

// オフセット値取得
/* Under construction */
/* オフセット値を取得＆フラッシュメモリ領域に書き込んで電源ON時に読み込ませる予定 */

Calibration cal;
// キャリブレーション状態取得
void bno_read_cal(){
  // bno.getCalibration(&data_psd_bno_cal_system, &data_psd_bno_cal_gyro, &data_psd_bno_cal_accel, &data_psd_bno_cal_mag); //system, gyro, accel, magの順番
  bno.getCalibration(&cal.system, &cal.gyro, &cal.accel, &cal.mag); //system, gyro, accel, magの順番
}