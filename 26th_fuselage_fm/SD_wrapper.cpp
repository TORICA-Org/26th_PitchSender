#include "freertos/idf_additions.h"
#include "esp32-hal-gpio.h"
#include "SD_wrapper.h"
#include "parameters.h"
#include "SD_fslg.h"

QueueHandle_t sdQueue = NULL;  // SDタスク用のキュー

extern TORICA_SD sd; // SD_fslg.h内のインスタンスsdを扱うため

void initSDTask(){
    // 20件分収容できるキューを作成
    sdQueue = xQueueCreate(20, sizeof(LogData));  // キューの作成
    initSD();  // SDカードの初期化
    flashHeader();  // ヘッダーファイルの書き込み

    xTaskCreatePinnedToCore(SD_Task, "SD_Task", 4096, NULL, 4, NULL, 1);
}


void queueLogdata(){
    LogData data;

    data.time_ms = time_ms;
    data.takeoff = takeoff;
    data.filtered_bmp_altitude_m = filtered_bmp_altitude_m;
    data.filtered_urm_altitude_m = filtered_urm_altitude_m;
    data.urm_is_reliable = urm_is_reliable;
    data.filtered_airspeed_ms = filtered_airspeed_ms;

    data.data_air_bmp_pressure_hPa = data_air_bmp_pressure_hPa;
    data.data_air_bmp_temperature_deg = data_air_bmp_temperature_deg;
    data.data_air_bmp_altitude_m = data_air_bmp_altitude_m;
    data.data_air_gps_hour = data_air_gps_hour;
    data.data_air_gps_minute = data_air_gps_minute;
    data.data_air_gps_second = data_air_gps_second;
    data.data_air_gps_centisecond = data_air_gps_centisecond;
    data.data_air_gps_latitude_deg = data_air_gps_latitude_deg;
    data.data_air_gps_longitude_deg = data_air_gps_longitude_deg;
    data.data_air_gps_altitude_m = data_air_gps_altitude_m;
    data.data_air_gps_groundspeed_ms = data_air_gps_groundspeed_ms;
    data.data_air_gps_heading_deg = data_air_gps_heading_deg;
    data.data_air_gps_satellites = data_air_gps_satellites;
    data.data_air_sdp_differentialPressure_Pa = data_air_sdp_differentialPressure_Pa;
    data.data_air_sdp_airspeed_ms = data_air_sdp_airspeed_ms;
    data.data_air_AoA_angle_deg = data_air_AoA_angle_deg;
    data.data_air_AoS_angle_deg = data_air_AoS_angle_deg;
    data.data_ics_angle = data_ics_angle;

    data.fslg_is_alive = fslg_is_alive;
    data.data_fslg_bno_accx_mss = data_fslg_bno_accx_mss;
    data.data_fslg_bno_accy_mss = data_fslg_bno_accy_mss;
    data.data_fslg_bno_accz_mss = data_fslg_bno_accz_mss;
    data.data_fslg_bno_qw = data_fslg_bno_qw;
    data.data_fslg_bno_qx = data_fslg_bno_qx;
    data.data_fslg_bno_qy = data_fslg_bno_qy;
    data.data_fslg_bno_qz = data_fslg_bno_qz;
    data.data_fslg_bno_roll = data_fslg_bno_roll;
    data.data_fslg_bno_pitch = data_fslg_bno_pitch;
    data.data_fslg_bno_yaw = data_fslg_bno_yaw;
    data.data_fslg_bno_cal_system = data_fslg_bno_cal_system;
    data.data_fslg_bno_cal_gyro = data_fslg_bno_cal_gyro;
    data.data_fslg_bno_cal_accel = data_fslg_bno_cal_accel;
    data.data_fslg_bno_cal_mag = data_fslg_bno_cal_mag;
    data.data_fslg_bmp_pressure_hPa = data_fslg_bmp_pressure_hPa;
    data.data_fslg_bmp_temperature_deg = data_fslg_bmp_temperature_deg;
    data.data_fslg_bmp_altitude_m = data_fslg_bmp_altitude_m;
    data.data_fslg_lsm_accx_mss = data_fslg_lsm_accx_mss;
    data.data_fslg_lsm_accy_mss = data_fslg_lsm_accy_mss;
    data.data_fslg_lsm_accz_mss = data_fslg_lsm_accz_mss;
    data.data_fslg_lsm_roll = data_fslg_lsm_roll;
    data.data_fslg_lsm_pitch = data_fslg_lsm_pitch;
    data.data_fslg_lsm_yaw = data_fslg_lsm_yaw;

    data.under_is_alive = under_is_alive;
    data.data_under_bmp_pressure_hPa = data_under_bmp_pressure_hPa;
    data.data_under_bmp_temperature_deg = data_under_bmp_temperature_deg;
    data.data_under_bmp_altitude_m = data_under_bmp_altitude_m;
    data.data_under_urm_altitude_m = data_under_urm_altitude_m;
    data.data_under_tsd20_altitude_m = data_under_tsd20_altitude_m;


    // キューが作成されていたらキューにデータを送信
    if (sdQueue != NULL) {
        xQueueSend(sdQueue, &data, 0);  // キューにデータを送信
    }
}


void SD_Task(void *pvParameters){
    LogData receivedData;

    while(1){
        if (xQueueReceive(sdQueue, &receivedData, portMAX_DELAY) == pdPASS) {

            for (int mode = 0; mode < 4; mode++){
                addDataToSDBuf(receivedData, mode);
            }

            writeSD();
            
        }
    }
}

void detect_RESET_signal(){
    if (RESET_SIG == true){
        
    }   
}