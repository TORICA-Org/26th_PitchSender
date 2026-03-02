#include <Arduino.h>
#include "MyKalman.h"
#include "MyIMU.h"

#include <Kalman.h>

// インスタンス
Kalman kalmanX; // Roll用
Kalman kalmanY; // Pitch用

// オフセット変数
float gyroX_offset = 0;
float gyroY_offset = 0;

// 時間管理
unsigned long prevMicros = 0;


void init_kalman() {
  // --- 1. ジャイロのキャリブレーション (X, Yのみ) ---
  // 起動時は静止させてください
  const int num_samples = 1000;
  
  float gx, gy, gz, ax, ay, az;

  for (int i = 0; i < num_samples; i++) {
    readSensor(gx, gy, gz, ax, ay, az);
    gyroX_offset += gx;
    gyroY_offset += gy;
    delay(1);
  }
  gyroX_offset /= num_samples;
  gyroY_offset /= num_samples;

  // --- 2. 初期角度のセット ---
  readSensor(gx, gy, gz, ax, ay, az);

  // 角度算出 (単位: 度)
  float roll  = atan2(ay, az) * RAD_TO_DEG;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * RAD_TO_DEG;

  kalmanX.setAngle(roll);
  kalmanY.setAngle(pitch);

  prevMicros = micros();
}


float get_est_pitch() {
  // --- 時間計算 (dt) ---
  unsigned long currentMicros = micros();
  float dt = (float)(currentMicros - prevMicros) / 1000000.0;
  prevMicros = currentMicros;

  // --- センサー値取得 ---
  float gx, gy, gz, ax, ay, az;
  readSensor(gx, gy, gz, ax, ay, az);

  gx -= gyroX_offset;
  gy -= gyroY_offset;

  // --- 角度算出 (観測値) ---
  float accRoll  = atan2(ay, az) * RAD_TO_DEG;
  float accPitch = atan2(-ax, sqrt(ay * ay + az * az)) * RAD_TO_DEG;

  // --- カルマンフィルタ更新 ---
  // Roll (X軸周り)
  float estRoll = kalmanX.getAngle(accRoll, gx, dt);
  // Pitch (Y軸周り)
  float estPitch = kalmanY.getAngle(accPitch, gy, dt);

  return estPitch;
}