#pragma once

#include <Arduino.h>

struct EulerAngles {  // オイラー角用の構造体.
  float roll, pitch, yaw;
};

struct Calibration { // キャリブレーション用の構造体.
  unsigned char system, gyro, accel, mag; 
};