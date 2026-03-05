// ----- ESP32がCOMポートで認識されない -----
// 1. デバイスマネージャを確認
// 2. `cp2102n usb to uart bridge controller`を開くとドライバが無い
// 3. `https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads`にアクセス
// 4. `CP210x Windows Drivers`をダウンロード
// 5. 解凍し`CP210xVCPInstaller_x64.exe`を実行（Windowsの場合）

#include "MyBluetooth.h"
#include "Frequency.h"
#include "MyIMU.h"

constexpr float TOLERANCE = 3.0;

enum {
  TAIL_UP,
  LEVEL,
  TAIL_DOWN
} attitude = LEVEL;

void setup() {
  Serial.begin(115200);
  bt_init("BTW13X");
  imu_init();
}

void loop() {
  imu_refresh_euler();
  Serial.printf("%.5f %.5f %.5f\n", angles.roll, angles.pitch, angles.yaw);

  if (angles.pitch < -1 * TOLERANCE) {
    attitude = TAIL_UP;
  }
  else if (angles.pitch > TOLERANCE) {
    attitude = TAIL_DOWN;
  }
  else {
    attitude = LEVEL;
  }

  switch (attitude) {
    case TAIL_UP: {
      bt_set_sound(frequency_get("G4"), 1.0);
    }
    case LEVEL: {
      bt_set_sound(frequency_get("C4"), 1.0);
    }
    case TAIL_DOWN: {
      bt_set_sound(frequency_get("G3"), 1.0);
    }
  }
  delayMicroseconds(10);
}

