/*---------------------------------------------------------

このファイルの役割：BMP390の初期化・値取得
最終更新日：2026/04/11 00:42
更新内容：read_bmp_fslg()作成
注) 各電装部において使用しない関数はコメントアウトすること．

---------------------------------------------------------*/

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP3XX.h>


//センサー初期化用
bool BMP3XX_init(void);

void read_bmp_air(void);

// void read_bmp_under(void);

// void read_bmp_fslg(void);


//float BMP3XX_getTemperature_deg(void);

//float BMP3XX_getPressure_hPa(void);