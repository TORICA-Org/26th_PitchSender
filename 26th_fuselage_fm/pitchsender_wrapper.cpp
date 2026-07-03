#include "pitchsender_wrapper.h"
#include "fslg_config.h"

enum {
  TAIL_UP,
  LEVEL,
  TAIL_DOWN
} attitude = LEVEL;

void pitchsender_init() {
  // IMU初期化とBluetoothデバイス初期化
  imu_init();
  // bt_init("WF-C510");
  // bt_init("Echo Buds 00UG");
}


void pitchsender_loop() {
  imu_refresh_euler();

  String status;

  if (angles.pitch < -1 * TOLERANCE) {
    attitude = TAIL_UP;
    status = "TAIL_UP";
  } else if (angles.pitch > TOLERANCE) {
    attitude = TAIL_DOWN;
    status = "TAIL_DOWN";
  } else {
    attitude = LEVEL;
    status = "LEVEL";
  }

  float freq = 0.0;
  float interval = 0.0;

  switch (attitude) {
    case TAIL_UP:
      {
        freq = frequency_get("G5");
        interval = 0.05;
        break;
      }
    case LEVEL:
      {
        freq = frequency_get("C5");
        interval = 0.5;
        break;
      }
    case TAIL_DOWN:
      {
        freq = frequency_get("G5");
        interval = 0.1;
        break;
      }
  }

  bt_set_sound(freq, interval);

  static unsigned long prev = 0;
  unsigned long cur = millis();
  if (cur - prev > 1000) {
    prev = cur;
    // for debug
    // Serial.printf("[%s | %s]  (%.5f, %.5f, %.5f)\n", bt_status, status, angles.roll, angles.pitch, angles.yaw);
  }

  // 10ms周期のループはxTaskCreatePinnedToCore()で管理する
  // delayMicroseconds(10);
}
