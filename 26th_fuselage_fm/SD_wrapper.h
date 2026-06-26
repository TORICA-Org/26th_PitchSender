#pragma once
#include "fslg_config.h"          // 胴体桁基板のピン設定
#include "SD_fslg.h"

void initSDTask();
void queueLogdata();
void SD_Task(void *pvParameters);