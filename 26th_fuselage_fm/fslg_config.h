//ピン番号設定
#pragma once
#include <Arduino.h>

// 胴体桁基板ピン設定

// UART
extern const int SerialTX; // GPIO32
extern const int SerialRX; // GPIO33

// I2Cピン
extern const int I2C_SDA; // GPIO21
extern const int I2C_SCL; // GPIO22


// SD用ピン
extern const int SD_CS;   // GPIO16
extern const int SD_SCK;  // GPIO18
extern const int SD_MOSI; // GPIO23
extern const int SD_MISO; // GPIO19

// スピーカー用
extern const int SPK;     // GPIO13

// LED
extern const int LED1;     // GPIO3
extern const int LED2;    // GPIO25