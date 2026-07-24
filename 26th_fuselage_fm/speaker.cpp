#include <stdint.h>
#include "speaker.h"
#include "check_restricted_zone.h"
#include "parameters.h"

// #define SPEAKER_ALT_ENABLE // やっぱり高度読み上げONにしたくなったらコメントアウト外す

const float airspeed_factor  = 1.0; // 対気速度の補正係数．
// airspeed_adj = airspeed * airspeed_factor

const float TTC_WARNING_SEC = 5.0; // アラートを出す閾値（境界線到達までの秒数）

// スピーカー用ピン設定
#include "fslg_config.h"
void speaker_init(){
    pinMode(SPK, OUTPUT);
}

// 変化する範囲を変換する関数
float float_map (float val, float in_min, float in_max, float out_min, float out_max) {
  return out_min + (val - in_min) * (out_max - out_min) / (in_max - in_min);
}


void speaker(float airspeed, float altitude, bool takeoff, bool isInsideArea, float time_to_reach){

    static int last_mode = -1;    // 前回のモードを記憶
    static bool spk_flag = false; // 音が出ているかのフラグ
    static uint32_t speaker_last_change_time = 0;

    // 現在のモード
    // 0. platform(停止), 1: 禁止区域内（即座に警報を鳴らす），2: 接近中（注意），3: 通常飛行
    int current_mode = 0;

    // current_modeを決定
    // 離陸後のみ or 強制的にスピーカーONになったら音を鳴らす
    if (takeoff == true || SPK_ENABLE == true){
        if (isInsideArea == false) {
            // 禁止区域内
            current_mode = 1;
        }
        // 禁止区域接近まで2.0秒を切ったら
        // time_to_reach > 0.0は禁止区域に接近中を意味する．（逆に0以下は禁止区域から離れている）
        else if (time_to_reach > 0.0 && time_to_reach < 2.0) {
            current_mode = 2;
        }
        // 通常飛行．禁止区域外から十分に離れているとき
        else {
            current_mode = 3;
        }
    } else { // 離陸前なので音を鳴らさない
        current_mode = 0;
    }

    // current_mode = 3; // デバッグ用：強制的に通常飛行(mode == 3)にする

    // 飛行位置が変わった瞬間（current_modeが変わった瞬間）の処理
    if (current_mode != last_mode){
        noTone(SPK); // 一旦音を止める
        spk_flag = false;
        // モード変更直後に即座に最初の音が鳴るようにタイマーを大きめに過去に設定
        speaker_last_change_time = millis() - 10000;
        last_mode = current_mode; // 現在のモードを更新
    }

    // モード0（停止中）のときは音を消して終了
    if (current_mode == 0){
        if (spk_flag) {
            noTone(SPK);
            spk_flag = false;
        }
        return;
    }

    // 機速に応じた音階を計算
    float airspeed_adj = airspeed * airspeed_factor;
    uint32_t airspeed_freq = 440;
    if (airspeed_adj >= 11.0){
        airspeed_freq = 1320;
    } else if (airspeed_adj >= 9.0){
        airspeed_freq = 880;
    } else {
        airspeed_freq = 440;
    }

    uint32_t sound_freq = 440;
    uint32_t sound_duration = 300; // 音が出ている時間
    uint32_t off_duration = 0;     // 音が消えている時間

    // モードごとの音階・発音時間・消音時間を設定
    if (current_mode == 1){ // 禁止区域内（警報音）
        sound_freq = 1760;
        sound_duration = 120;
        off_duration = 120;
    }

    else if (current_mode == 2){ // 禁止区域接近中．周波数は機速にあわせ，間隔を短くする
        sound_freq = airspeed_freq;
        sound_duration = 150;
        off_duration = 150;
    }

    else if (current_mode == 3){ // 通常飛行
        sound_freq = airspeed_freq;

        uint32_t interval = 900;
#ifdef SPEAKER_ALT_ENABLE
        float altitude_max = 1.0;
        float altitude_min = 0.0;

        if (altitude >= 1.5){
            interval = 900;
        } else if (altitude >= 0.0){
            interval = float_map(altitude, altitude_min, altitude_max, 125, 700);
        } else {
            interval = 100;
        }
#endif
        sound_duration = 300;
        if (interval > sound_duration) {
            off_duration = interval - sound_duration;
        } else {
            off_duration = 0;
        }
    }

    uint32_t current_time = millis();

    // 音が消えていて，消音時間を過ぎたら音を出す
    if (spk_flag == false && (current_time - speaker_last_change_time) > off_duration){
        tone(SPK, sound_freq);
        speaker_last_change_time = current_time;
        spk_flag = true;
    }
    // 音が出ていて，発音時間が過ぎたら音を消す
    else if(spk_flag == true && (current_time - speaker_last_change_time) > sound_duration){
        noTone(SPK);
        speaker_last_change_time = current_time;
        spk_flag = false;
    }
}


