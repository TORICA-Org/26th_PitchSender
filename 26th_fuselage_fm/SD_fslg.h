/*-----------------------

このファイルの役割：胴体桁基板でのSD用関数

------------------------*/

#pragma once

#include <SD.h>
#include <TORICA_SD.h>

#include "parameters.h"

bool initSD();

void flashHeader();

void flashSD(int flash_mode);

void addDataToSDBuf(const LogData& data, int flash_mode);

void writeSD();