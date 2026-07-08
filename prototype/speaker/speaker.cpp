#include "speaker.h"

const float airspeed_factor  = 1.0; // 対気速度の補正係数．
// airspeed_adj = airspeed * airspeed_factor 


// スピーカー用ピン設定
#include "fslg_config.h"
void speaker_init(){
    pinMode(SPK, OUTPUT);
}

void speaker(float airspeed, float altitude, bool takeoff, bool isInsideArea, float time_to_reach){
    
    // 最重要：禁止区域内にいれば警報を鳴らす
    if (isInsideArea == true) {
        // 禁止区域内ならば

    }

    // 禁止区域接近までの時間
    else if (time_to_reach > 0.0 && time_to_reach < 6.0) {
    // 禁止区域接近まで6.0秒を切ったら
    }
    
    // 禁止区域外から十分に離れているとき
    else {
        
        // まずは対気速度による音の高さの決定
        static bool spk_flag = false; // 音が出ているかのフラグ
        static uint32_t speaker_last_change_time = millis();
        static uint32_t sound_duration = 100; // 音が出ている時間

        float airspeed_adj = airspeed * airspeed_factor // 補正済み対気速度を計算

        // 補正済み対気速度が10.8m/s以上のとき
        if (airspeed_adj >= 10.8){
            sound_freq = 1320;
            break;
            
        // 9.5m/s以上かつ10.8m/s未満のとき
        } else if (9.5 =< airspeed_adj && airspeed_adj < 10.8) {
            sound_freq = 880;
            break;

        // 0m/s以上9.5m/s未満のとき
        } else if (0 =< airspeed_adj && airspeed_adj < 9.5){
            sound_freq = 440;
            break;
        }
        
        // 高度によるインターバルの設定
        float interval;
        float altitude_max = 1.0;
        float altitude_min = 0.0;
        
        // まずはプラホ上にいるかどうか
        if (istakeoff == false){
            interval = 1000;
            break;

        // 離陸後
        } else {
            if (altitude >= 1.5 ){
                // 1.5m以上なら
                interval = 900;
                break;
            } else if (0.3 <= altitude && altitude < 1.5){
                // 0.3m以上1.5m未満なら
                interval = float_map(filtered_under_urm_altitude_m, altitude_min, altitude_max, 125, 700);
                break;
            } else if (0 <= altitude && altitude < 0.3 ){
                // 0m以上かつ0.3m以下なら
                interval = float_map(filtered_under_urm_altitude_m, altitude_min, altitude_max, 125, 700);
                break;
            } else {
                // 異常値（マイナス）のとき
                interval = 100;
                break; 
            }
        }   
    }


    uint32_t current_time = millis();
    uint32_t off_duration = interval - sound_duration;
    
    // 実際に鳴らす
    if (flight_phase != 0) {
        
        if (spk_flag == 0 && (current_time - speaker_last_change_time) > off_duration) {
            tone(SPK, sound_freq);
            speaker_last_change_time = current_time;
            spk_flag = 1;

        } else if (spk_flag == 1 && (current_time - speaker_last_change_time) > sound_duration) {
            noTone(SPK);
            speaker_last_change_time = current_time;
            spk_flag = 0;
        }
    }
}