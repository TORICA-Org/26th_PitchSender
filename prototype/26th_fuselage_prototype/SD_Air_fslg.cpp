/*-----------------------

このファイルの役割：胴体桁基板でのSD用関数

------------------------*/

#include "SD_Air_fslg.h"

//ピン配置定義ファイルを読み込む
#include "fslg_config.h"

#include "parameters.h"


TORICA_SD sd; //引数なしでインスタンス化

char SD_BUF[256]; //SD書き込み用バッファ

//SD初期化コード
void initSD(){

  //ESP32のSPI初期化コードはrp2040とちょっと違う
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!sd.begin(SD_CS)) {
    Serial.println("SD initialization failed!");
  } else {
    Serial.println("SD initialization done.");
  }
  
}


void flashHeader() {
  // この関数は`setup()`内なのでブロッキング関数（処理の流れが止まる関数）であっても構わない
  const char *str[3];

  for (int i = 0; i < 4 /* case0~3まで実行 */; i++) {

    switch (i) {
      case 0: // 12個
      { 
        str[0] = "time_ms,takeoff,urm_is_reliable,data_air_gps_hour,"; // 4個
        str[1] = "data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond, data_air_gps_latitude_deg," // 4個
        str[2] = "ata_air_gps_longitude_deg, data_air_gps_altitude_m, data_air_gps_groundspeed_ms,data_air_gps_heading_deg,"; // 4個  
        break;
        }
      case 1:
      { // 11個
        str[0] = "filtered_bmp_altitude_m,filtered_urm_altitude_m,filtered_airspeed_ms,data_air_bmp_pressure_hPa," // 4個
        str[1] = "data_air_bmp_temperature_deg,data_air_bmp_altitude_m,data_air_sdp_differentialPressure_Pa,data_air_sdp_airspeed_ms," // 4個
        str[2] = "data_air_AoA_angle_deg,data_air_AoS_angle_deg,data_ics_angle,"; // 3個
        break;
        }
      case 2: // 14個
      {
        str[1] = "fslg_is_alive,data_fslg_bno_qw,data_fslg_bno_qx,data_fslg_bno_qy,data_fslg_bno_qz,"; // 5個
        str[2] = "data_fslg_bno_roll,data_fslg_bno_pitch,data_fslg_bno_yaw,data_fslg_lsm_roll,data_fslg_lsm_pitch,"; // 5個
        str[2] = "data_fslg_lsm_yaw,data_fslg_bmp_pressure_hPa,data_fslg_bmp_temperature_deg,data_fslg_bmp_altitude_m"; // 4個
        break;
        }
      case 3: // 16個
      {
        str[0] = "data_fslg_bno_accx_mss,data_fslg_bno_accy_mss,data_fslg_bno_accz_mss,data_fslg_lsm_accx_mss,data_fslg_lsm_accy_mss, data_fslg_lsm_accz_mss," // 6個
        str[1] = "data_fslg_bno_cal_system,data_fslg_bno_cal_gyro,data_fslg_bno_cal_accel,data_fslg_bno_cal_mag,under_is_alive," // 5個
        str[2] = "data_under_bmp_pressure_hPa,data_under_bmp_temperature_deg,data_under_bmp_altitude_m,data_under_urm_altitude_m, data_under_tsd20_altitude_m\n"; // 5個
        break;
      }
      default:
      {
        Serial.println("The parameter value is out of range.");
        break;
      }
    }

    sprintf(SD_BUF, "%s%s%s", str[0], str[1], str[2]);

    sd.add_str(SD_BUF);
    sd.flash();

    delayMicroseconds(10);  // 遅延あったほうがいいと思う
  }
}


//とりあえず20Hz書き込みで様子見
void flashSD(int flash_mode){
    memset(SD_BUF, 0, sizeof(SD_BUF)); //SD_BUFを0で初期化

    switch (flash_mode) { // 計53個
    case 0: // 計12個
      {
        sprintf(SD_BUF, "%lu,%d,%d,%u,%u,%u,%u,%.7f,%.7f,%.2f,%.2f,%.1f,", 
        time_ms, takeoff, urm_is_reliable, data_air_gps_hour, // 4個
        data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond, data_air_gps_latitude_deg, // 4個
        data_air_gps_longitude_deg, data_air_gps_altitude_m, data_air_gps_groundspeed_ms,data_air_gps_heading_deg // 4個
        );
        sd.add_str(SD_BUF);
        sd.flash();
        break;
      }
    case 1:
      { // 計11個
        sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,", 
        filtered_bmp_altitude_m, filtered_urm_altitude_m, filtered_airspeed_ms, // 3個
        data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m, // 3個
        data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms, // 2個
        data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle); // 3個
        sd.add_str(SD_BUF);
        sd.flash();
        break;
      }
    case 2: // 計14個
      {
        sprintf(SD_BUF, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", 
        fslg_is_alive, // 1個
        data_fslg_bno_qw, data_fslg_bno_qx, data_fslg_bno_qy, data_fslg_bno_qz, // 4個
        data_fslg_bno_roll, data_fslg_bno_pitch, data_fslg_bno_yaw, // 3個
        data_fslg_lsm_roll, data_fslg_lsm_pitch, data_fslg_lsm_yaw, // 3個
        data_fslg_bmp_pressure_hPa, data_fslg_bmp_temperature_deg, data_fslg_bmp_altitude_m); // 3個
        sd.add_str(SD_BUF);
        sd.flash();
        break;
      }

    case 3: // 計16個
      {
        sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u,%u,%u,%u,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n", 
        data_fslg_bno_accx_mss, data_fslg_bno_accy_mss, data_fslg_bno_accz_mss, // 3個
        data_fslg_lsm_accx_mss, data_fslg_lsm_accy_mss, data_fslg_lsm_accz_mss, // 3個
        data_fslg_bno_cal_system, data_fslg_bno_cal_gyro, data_fslg_bno_cal_accel, data_fslg_bno_cal_mag, // 4個
        under_is_alive, // 1個
        data_under_bmp_pressure_hPa, data_under_bmp_temperature_deg, data_under_bmp_altitude_m, // 3個
        data_under_urm_altitude_m, data_under_tsd20_altitude_m); // 2個
        sd.add_str(SD_BUF);
        sd.flash();
        break;
      }
    default:
      {
        Serial.println("Invalid argument for flashSD()");
        break;
      }
  }
}