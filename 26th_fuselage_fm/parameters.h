/*------------------------------------

このファイルの役割：変数，定数の宣言
最終更新：2026/04/17 17:17
更新内容：送信用変数の見直し

------------------------------------*/

#pragma once  // インクルードガード（複数回読み込まれないようにする）
#include <Arduino.h>

// 離陸判定
extern volatile bool takeoff;
// 動作時間
extern volatile uint32_t time_ms;

// SerialWeb用
extern volatile bool RESET_SIG;
extern volatile bool takeoff_decition;

/*--- 高度計算用 ---*/
// 気圧高度・超音波高度フィルタリング用
extern volatile float filtered_bmp_altitude_m;
extern volatile float filtered_urm_altitude_m;

// 超音波高度を信用するか
extern volatile bool urm_is_reliable;

// 対気速度フィルタリング用
extern volatile float filtered_airspeed_ms;

/*--- エアデータ用 ---*/

// BMP390
extern volatile float data_air_bmp_pressure_hPa;
extern volatile float data_air_bmp_temperature_deg;
extern volatile float data_air_bmp_altitude_m;

// GPS
extern volatile uint8_t data_air_gps_hour;
extern volatile uint8_t data_air_gps_minute;
extern volatile uint8_t data_air_gps_second;
extern volatile uint8_t data_air_gps_centisecond;
extern volatile double data_air_gps_latitude_deg;
extern volatile double data_air_gps_longitude_deg;
extern volatile double data_air_gps_altitude_m;
extern volatile double data_air_gps_groundspeed_ms;
extern volatile float data_air_gps_heading_deg;
extern volatile uint8_t data_air_gps_satellites;

// SDP31
extern volatile float data_air_sdp_differentialPressure_Pa;
extern volatile float data_air_sdp_airspeed_ms;

// AoA,AoS
extern volatile float data_air_AoA_angle_deg;
extern volatile float data_air_AoS_angle_deg;

// ICS基盤
extern volatile int data_ics_angle;

/*--- 胴体桁電装用 ---*/
extern volatile bool fslg_is_alive;

// BNO055
extern volatile float data_fslg_bno_accx_mss;
extern volatile float data_fslg_bno_accy_mss;
extern volatile float data_fslg_bno_accz_mss;
extern volatile float data_fslg_bno_qw;
extern volatile float data_fslg_bno_qx;
extern volatile float data_fslg_bno_qy;
extern volatile float data_fslg_bno_qz;
extern volatile float data_fslg_bno_roll;
extern volatile float data_fslg_bno_pitch;
extern volatile float data_fslg_bno_yaw;

// BNO055キャリブレーション状態保存用
extern volatile uint8_t data_fslg_bno_cal_system;
extern volatile uint8_t data_fslg_bno_cal_gyro;
extern volatile uint8_t data_fslg_bno_cal_accel;
extern volatile uint8_t data_fslg_bno_cal_mag;

// BMP390
extern volatile float data_fslg_bmp_pressure_hPa;
extern volatile float data_fslg_bmp_temperature_deg;
extern volatile float data_fslg_bmp_altitude_m;

// LSM6DSV16X
extern volatile float data_fslg_lsm_accx_mss;
extern volatile float data_fslg_lsm_accy_mss;
extern volatile float data_fslg_lsm_accz_mss;
extern volatile float data_fslg_lsm_roll;
extern volatile float data_fslg_lsm_pitch;
extern volatile float data_fslg_lsm_yaw;

/*--- Under電装部 ---*/
extern volatile bool under_is_alive;
extern volatile float data_under_bmp_pressure_hPa;
extern volatile float data_under_bmp_temperature_deg;
extern volatile float data_under_bmp_altitude_m;
extern volatile float data_under_urm_altitude_m;
extern volatile float data_under_tsd20_altitude_m;



struct LogData {
    // 6個
    bool takeoff;
    uint32_t time_ms;
    float filtered_bmp_altitude_m;
    float filtered_urm_altitude_m;
    bool urm_is_reliable;
    float filtered_airspeed_ms;

    /* エアデータ 18個*/
    float data_air_bmp_pressure_hPa;
    float data_air_bmp_temperature_deg;
    float data_air_bmp_altitude_m;
    uint8_t data_air_gps_hour;
    uint8_t data_air_gps_minute;
    uint8_t data_air_gps_second;
    uint8_t data_air_gps_centisecond;
    double data_air_gps_latitude_deg;
    double data_air_gps_longitude_deg;
    double data_air_gps_altitude_m;
    double data_air_gps_groundspeed_ms;
    float data_air_gps_heading_deg;
    uint8_t data_air_gps_satellites;
    float data_air_sdp_differentialPressure_Pa;
    float data_air_sdp_airspeed_ms;
    float data_air_AoA_angle_deg;
    float data_air_AoS_angle_deg;
    int data_ics_angle;

    /* 胴体桁基板 24個 */
    bool fslg_is_alive;
    float data_fslg_bno_accx_mss;
    float data_fslg_bno_accy_mss;
    float data_fslg_bno_accz_mss;
    float data_fslg_bno_qw;
    float data_fslg_bno_qx;
    float data_fslg_bno_qy;
    float data_fslg_bno_qz;
    float data_fslg_bno_roll;
    float data_fslg_bno_pitch;
    float data_fslg_bno_yaw;
    uint8_t data_fslg_bno_cal_system;
    uint8_t data_fslg_bno_cal_gyro;
    uint8_t data_fslg_bno_cal_accel;
    uint8_t data_fslg_bno_cal_mag;
    float data_fslg_bmp_pressure_hPa;
    float data_fslg_bmp_temperature_deg;
    float data_fslg_bmp_altitude_m;
    float data_fslg_lsm_accx_mss;
    float data_fslg_lsm_accy_mss;
    float data_fslg_lsm_accz_mss;
    float data_fslg_lsm_roll;
    float data_fslg_lsm_pitch;
    float data_fslg_lsm_yaw;
    
    /* Under基板 6個*/
    bool under_is_alive;
    float data_under_bmp_pressure_hPa;
    float data_under_bmp_temperature_deg;
    float data_under_bmp_altitude_m;
    float data_under_urm_altitude_m;
    float data_under_tsd20_altitude_m;
};