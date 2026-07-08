#include "calculate_altitude.h"

void calculate_bmp_altitude() {
    
    // 気圧高度計算
    if (data_fslg_bmp_pressure_hPa == 0.0f && data_fslg_bmp_temperature_deg == 0.0f) { // 気圧が0.0hPaのときはセンサー値が取得できていないとみなす
        data_fslg_bmp_altitude_m = 0.0f;
    } else {
        data_fslg_bmp_altitude_m = (powf(1013.25 / data_fslg_bmp_pressure_hPa, 1 / 5.257) - 1) * (data_fslg_bmp_temperature_deg + 273.15) / 0.0065;
    }
}