#pragma once

#include "parameters.h"
#include <TORICA_UART.h>

// 関数のプロトタイプ宣言
void initUART();
// void transmitHeader();
void transmitLog(int);
void receiveLog();
