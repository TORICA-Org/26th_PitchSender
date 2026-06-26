#pragma once

#include "parameters.h"
#include <TORICA_ICS.h>
#include <TORICA_UART.h>

// `extern`宣言すれば`TransmitUART.h`をインクルードしたファイルで使えるようになる
extern SerialPIO Serial_GPS;
extern SerialPIO Serial_ESP;

// 関数のプロトタイプ宣言
void initUART();
// void transmitHeader();
void transmitLog(int);
void receiveLog();
