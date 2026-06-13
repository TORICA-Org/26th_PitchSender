#pragma once

#include <Arduino.h>

#include "MyStruct.h"

extern volatile EulerAngles lsm_angles;

void lsm_init();
void lsm_refresh_euler();