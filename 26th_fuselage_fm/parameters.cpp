/*------------------------------------

このファイルの役割：変数，定数の宣言
最終更新：2026/04/17 17:17
更新内容：送信用変数の見直し

------------------------------------*/


#include <Arduino.h>
#include "parameters.h"

// 離陸判定
volatile bool takeoff = false;

// SerialWeb用
volatile bool RESET_SIG = false;
volatile bool takeoff_decition = false;

// 動作時間
volatile uint32_t time_ms = 0;

// 気圧高度・超音波高度フィルタリング用
volatile float filtered_bmp_altitude_m = 0.0f;
volatile float filtered_urm_altitude_m = 0.0f;

// 超音波センサを信用するか
volatile bool urm_is_reliable = false;

// 対気速度フィルタリング用
volatile float filtered_airspeed_ms = 0.0f;

/*--- エアデータ用 ---*/

// BMP390
volatile float data_air_bmp_pressure_hPa = 0.0f;
volatile float data_air_bmp_temperature_deg = 0.0f;
volatile float data_air_bmp_altitude_m = 0.0f;

// GPS
volatile uint8_t data_air_gps_hour = 0;
volatile uint8_t data_air_gps_minute = 0;
volatile uint8_t data_air_gps_second = 0;
volatile uint8_t data_air_gps_centisecond = 0;
volatile double data_air_gps_latitude_deg = 0.0;
volatile double data_air_gps_longitude_deg = 0.0;
volatile double data_air_gps_altitude_m = 0.0;
volatile double data_air_gps_groundspeed_ms = 0.0;
volatile float data_air_gps_heading_deg = 0.0f;
volatile uint8_t data_air_gps_satellites = 0;

// SDP31
volatile float data_air_sdp_differentialPressure_Pa = 0.0f;
volatile float data_air_sdp_airspeed_ms = 0.0f;

// AoA,AoS
volatile float data_air_AoA_angle_deg = 0.0f;
volatile float data_air_AoS_angle_deg = 0.0f;

// ICS基盤
volatile int data_ics_angle = 0;


/*--- 胴体桁電装用 ---*/
volatile bool fslg_is_alive = false;

// BNO055
volatile float data_fslg_bno_accx_mss = 0.0f;
volatile float data_fslg_bno_accy_mss = 0.0f;
volatile float data_fslg_bno_accz_mss = 0.0f;
volatile float data_fslg_bno_qw = 0.0f;
volatile float data_fslg_bno_qx = 0.0f;
volatile float data_fslg_bno_qy = 0.0f;
volatile float data_fslg_bno_qz = 0.0f;
volatile float data_fslg_bno_roll = 0.0f;
volatile float data_fslg_bno_pitch = 0.0f;
volatile float data_fslg_bno_yaw = 0.0f;

// BNO055キャリブレーション状態保存用
volatile uint8_t data_fslg_bno_cal_system = 0;
volatile uint8_t data_fslg_bno_cal_gyro = 0;
volatile uint8_t data_fslg_bno_cal_accel = 0;
volatile uint8_t data_fslg_bno_cal_mag = 0;

// BMP390
volatile float data_fslg_bmp_pressure_hPa = 0.0f;
volatile float data_fslg_bmp_temperature_deg = 0.0f;
volatile float data_fslg_bmp_altitude_m = 0.0f;

// LSM6DSV16X
volatile float data_fslg_lsm_accx_mss = 0.0f;
volatile float data_fslg_lsm_accy_mss = 0.0f;
volatile float data_fslg_lsm_accz_mss = 0.0f;
volatile float data_fslg_lsm_roll = 0.0f;
volatile float data_fslg_lsm_pitch = 0.0f;
volatile float data_fslg_lsm_yaw = 0.0f;

/*--- Under電装部 ---*/
volatile bool under_is_alive = false;
volatile float data_under_bmp_pressure_hPa = 0.0f;
volatile float data_under_bmp_temperature_deg = 0.0f;
volatile float data_under_bmp_altitude_m = 0.0f;
volatile float data_under_urm_altitude_m = 0.0f;
volatile float data_under_tsd20_altitude_m = 0.0f;