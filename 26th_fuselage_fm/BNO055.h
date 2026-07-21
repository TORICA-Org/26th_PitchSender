/*---------------------------------------------------------

このファイルの役割：BNO055初期化動作・値読み取り
最終更新日：2026/04/11 00:39
更新内容：胴体桁電装向けに変数を変更

---------------------------------------------------------*/

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO055.h>

bool BNO055_init(void);

void read_BNO(void);

void read_BNO_cal(void);

void BNO_Calib_init();

void BNO_Calib(uint8_t sys, uint8_t gyro, uint8_t accel, uint8_t mag);