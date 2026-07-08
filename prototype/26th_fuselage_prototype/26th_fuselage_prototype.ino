#include "fslg_config.h" //ピン設定など基板固有の設定値を読み込む

void setup() {
  // put your setup code here, to run once:
  Wire.begin(I2C_SDA, I2C_SCL, 400000); // I2C通信の初期化．SDA,SCL,CLOCK_SPEED

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  imu_init();
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
