// ----- ESP32がCOMポートで認識されない -----
// 1. デバイスマネージャを確認
// 2. `cp2102n usb to uart bridge controller`を開くとドライバが無い
// 3. `https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads`にアクセス
// 4. `CP210x Windows Drivers`をダウンロード
// 5. 解凍し`CP210xVCPInstaller_x64.exe`を実行（Windowsの場合）

#include <Arduino.h>
#include <Wire.h>

#include "MyBluetooth.h"
#include "Frequency.h"
#include "LSM6DSV16X.h"
#include "BNO055.h"

// キャリブレーション
constexpr float CAL = 0.0; // [degree]

// プラホ勾配オフセット
constexpr float OFFSET = 3.0; // [degree]

// 許容誤差（±TOLERANCE[deg]まで許容する）
constexpr float TOLERANCE = 0.1; // [degree]

enum {
  TAIL_UP,
  LEVEL,
  TAIL_DOWN
} attitude = LEVEL;

constexpr uint8_t SDA_PIN = 22;
constexpr uint8_t SCL_PIN = 21;
const int CLOCK_SPEED = 400000;

void setup() {

#if defined(ESP32) || defined(ESP8266)
  // ------------------------------------
  // ESP系 (ESP32 または ESP8266) の処理
  // ------------------------------------
  Wire.begin(SDA_PIN, SCL_PIN, CLOCK_SPEED);

#elif defined(ARDUINO_ARCH_RP2040)
  // ------------------------------------
  // RP2040系 (Raspberry Pi Picoなど) の処理
  // ------------------------------------
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();
  Wire.setClock(CLOCK_SPEED);

#else
  // ------------------------------------
  // その他のボード (AVRなど) の処理
  // ------------------------------------
  
#endif

  Serial.begin(115200);
  
  delay(2000);

  bt_init("WF-C510");
  bno_init();
  lsm_init();
}

void loop() {

  lsm_refresh_euler();
  bno_read();
  bno_read_cal();

  String status;

  float accurate_pitch = lsm_angles.pitch + CAL;

  if (accurate_pitch < OFFSET - TOLERANCE) {
    attitude = TAIL_UP;
    status = "TAIL_UP";
  }
  else if (accurate_pitch > OFFSET + TOLERANCE) {
    attitude = TAIL_DOWN;
    status = "TAIL_DOWN";
  }
  else {
    attitude = LEVEL;
    status = "LEVEL";
  }

  float freq = 0.0;
  float interval = 0.0;

  switch (attitude) {
    case TAIL_UP: {
      freq = frequency_get("G5");
      interval = 0.05;
      break;
    }
    case LEVEL: {
      freq = frequency_get("C5");
      interval = 0.5;
      break;
    }
    case TAIL_DOWN: {
      freq = frequency_get("G5");
      interval = 1.5;
      break;
    }
  }

  // bt_set_sound(freq, interval);

  static unsigned long prev = 0;
  unsigned long cur = millis();
  if (cur - prev > 100) {
    prev = cur;
    // Serial.printf("%.05f, %.05f, %.05f\n", 
    //   lsm_angles.roll, accurate_pitch, lsm_angles.yaw);
    // Serial.printf("[%s | LEVEL: %.1f | %s]  (%.05f, %.05f, %.05f)\n", 
    //   bt_status, OFFSET, status, angles.roll, accurate_pitch, angles.yaw);
  }

  Serial.printf("%0.7f, %0.7f\n", bno_angles.pitch, lsm_angles.pitch);

  delay(1);
}

