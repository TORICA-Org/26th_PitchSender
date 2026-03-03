// ----- ESP32がCOMポートで認識されない -----
// 1. デバイスマネージャを確認
// 2. `cp2102n usb to uart bridge controller`を開くとドライバが無い
// 3. `https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads`にアクセス
// 4. `CP210x Windows Drivers`をダウンロード
// 5. 解凍し`CP210xVCPInstaller_x64.exe`を実行（Windowsの場合）

#include "MyBluetooth.h"
#include "Frequency.h"
#include "MyIMU.h"

void setup() {
  Serial.begin(115200);
  while(!Serial);
  delay(3000);
  
  Serial.println("Ready!");
  float frequency = get_frequency("C3");
  Serial.print("Frequency: ");
  Serial.println(frequency);
  set_sound(frequency, 1.0);
  Serial.println("Sound setted!");
  //init_bt("C2");
  Serial.println("Initialized!");

  init_imu();
}

void loop() {
  if(Serial.available()){
    String str = Serial.readStringUntil('\n');
    char buff[3];
    str.toCharArray(buff, 3);
    Serial.print("ChangeTo: ");
    Serial.println(buff);
    float frequency = get_frequency(buff);
    Serial.print("Frequency: ");
    Serial.println(frequency);
    set_sound(frequency, 1.0);
  }

  refresh_euler_angles();
  Serial.printf("%.5f %.5f %.5f\n", angles.roll, angles.pitch, angles.yaw);
}

