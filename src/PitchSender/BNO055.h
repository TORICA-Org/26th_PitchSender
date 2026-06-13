/*---------------------------------------------------------

このファイルの役割：BNO055初期化動作・値読み取り
最終更新日：2026/04/11 00:39
更新内容：胴体桁電装向けに変数を変更

---------------------------------------------------------*/

#pragma once

#include <Arduino.h>

#include "MyStruct.h"

bool bno_init(void);
void bno_read(void);
void bno_read_cal(void);

extern volatile EulerAngles bno_angles;
extern Calibration bno_cal;