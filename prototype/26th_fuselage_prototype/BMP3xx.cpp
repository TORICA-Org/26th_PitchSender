/*---------------------------------------------------------

このファイルの役割：BMP390の初期化・値取得
最終更新日：2026/04/11 00:42
更新内容：read_bmp_fslg()作成
注) 各電装部において使用しない関数はコメントアウトすること．

---------------------------------------------------------*/

#pragma once
#include "BMP3xx.h"
#include "parameters.h"

Adafruit_BMP3XX bmp;

bool BMP3XX_init(void){
    if (!bmp.begin_I2C(0x77, &Wire1)) {
        #ifdef DEBUG_MODE
        Serial.println("Could not find a valid BMP3 sensor, check wiring!");
        #endif DEBUG_MODE
        while(1);
        return false;
    }
    
    // Set up oversampling and filter initialization
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);

    return true;

}



void read_bmp_air(void){
    if(bmp.performReading() == true){

        data_air_bmp_pressure_hPa = bmp.pressure / 100; // 気圧をhPaで表現
        data_air_bmp_temperature_deg = bmp.temperature; // 温度を℃で返す

    } else {
        //読み取れなかった場合，0.0を返す
        #ifdef DEBUG_MODE
        Serial.println("Failed to reading :(");
        #endif DEBUG_MODE
        data_air_bmp_pressure_hPa = 0.0;
        data_air_bmp_temperature_deg = 0.0;
    }
}


/* 機体下電装用 */

// void read_bmp_under(void){
//     if(bmp.performReading() == true){

//         data_under_bmp_pressure_hPa = bmp.pressure / 100 ; //気圧をhPaで表現
//         data_under_bmp_temperature_deg = bmp.temperature; //温度を℃で返す

//     } else if (bmp.performReading() == false){
//         //読み取れなかった場合，0.0を返す
//         #ifdef DEBUG_MODE
//         Serial.println("Failed to reading :(");
//         #endif DEBUG_MODE
//         data_under_bmp_pressure_hPa = 0.0; 
//         data_under_bmp_temperature_deg = 0.0;
//     }
// }


/* 胴体桁電装用 */

void read_bmp_fslg(void){
    if(bmp.performReading() == true){
        
        data_fslg_bmp_pressure_hPa = bmp.pressure / 100 ; //気圧をhPaで表現
        data_fslg_bmp_temperature_deg = bmp.temperature; //温度を℃で返す

    } else if (bmp.performReading() == false){
        //読み取れなかった場合，0.0を返す
        #ifdef DEBUG_MODE
        Serial.println("Failed to reading :(");
        #endif DEBUG_MODE
        data_fslg_bmp_pressure_hPa = 0.0; 
        data_fslg_bmp_temperature_deg = 0.0;
    }
}



/*---------------

float BMP3XX_getTemperature_deg(void){
    if(!bmp.performReading()){
        #ifdef DEBUG_MODE
        Serial.println("Failed to perform reading :(");
        return 0.0; //読み取れなかったら0.0を返す
    }
    return bmp.temperature; //温度を℃で返す
}

float BMP3XX_getPressure_hPa(void){
    if(!bmp.performReading()) {
        #ifdef DEBUG_MODE
        Serial.println("Failed to perform reading :(");
        return 0.0; //読み取れなかった場合0.0を返す
    }
    return bmp.pressure/100 ; //圧力をhPaで返す
}

---------------*/
