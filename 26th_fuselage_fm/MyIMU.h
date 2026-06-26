#pragma once

struct EulerAngles {  // オイラー角用の構造体.
  float roll, pitch, yaw;
};
extern volatile EulerAngles angles;

void imu_init();
void imu_refresh_euler();