/*-----------------------

このファイルの役割：胴体桁基板でのSD用関数

------------------------*/

#pragma once

#include <SD.h>
#include <TORICA_SD.h>

#include "parameters.h"

void initSD();

void flashHeader();

void flashSD(int flash_mode);