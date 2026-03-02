// ----- ESP32がCOMポートで認識されない -----
// 1. デバイスマネージャを確認
// 2. `cp2102n usb to uart bridge controller`を開くとドライバが無い
// 3. `https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads`にアクセス
// 4. `CP210x Windows Drivers`をダウンロード
// 5. 解凍し`CP210xVCPInstaller_x64.exe`を実行（Windowsの場合）

//#include "MyBluetooth.h"
#include "Frequency.h"
#include "MyIMU.h"
#include "MyKalman.h"

void setup() {
  Serial.begin(115200);
  while(!Serial);
  delay(3000);
  //init_bt();
  init_imu();
  init_kalman();
}

void loop() {
  Serial.println(get_est_pitch());
  delay(100);
}

