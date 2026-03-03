#pragma once

struct EulerAngles {  // オイラー角用の構造体.
  float roll, pitch, yaw;
};
extern volatile EulerAngles angles;

void init_imu();
void refresh_euler_angles();