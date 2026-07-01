/*-----------------------

このファイルの役割：胴体桁基板でのSD用関数

------------------------*/

#include "SD_fslg.h"

//ピン配置定義ファイルを読み込む
#include "fslg_config.h"
#include "parameters.h"

TORICA_SD sd;  //引数なしでインスタンス化

char SD_BUF[2048];  //SD書き込み用バッファ

bool SD_is_active = false;  //SDが正常に動作しているかどうかを示すフラグ
//SD初期化コード
bool initSD() {
  // SPI.begin();
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);  // SPIの初期化
  if (!sd.begin(SD_CS)) {
    SD_is_active = false;
    return false;
  };
  SD_is_active = true;
  return true;
}

void flashHeader() {
  // この関数は`setup()`内なのでブロッキング関数（処理の流れが止まる関数）であっても構わない
  if (SD_is_active) {
    const char *str[3];

    for (int i = 0; i < 4 /* case0~3まで実行 */; i++) {

      switch (i) {
        case 0:  // 13個
          {
            str[0] = "time_ms,takeoff,urm_is_reliable,data_air_gps_hour,";                                                         // 4個
            str[1] = "data_air_gps_minute,data_air_gps_second,data_air_gps_centisecond,data_air_gps_latitude_deg,";             // 4個
            str[2] = "data_air_gps_longitude_deg,data_air_gps_altitude_m,data_air_gps_groundspeed_ms,data_air_gps_heading_deg,data_air_gps_satellites,";  // 4個
            break;
          }
        case 1:
          {                                                                                                                                  // 11個
            str[0] = "filtered_bmp_altitude_m,filtered_urm_altitude_m,filtered_airspeed_ms,data_air_bmp_pressure_hPa,";                      // 4個
            str[1] = "data_air_bmp_temperature_deg,data_air_bmp_altitude_m,data_air_sdp_differentialPressure_Pa,data_air_sdp_airspeed_ms,";  // 4個
            str[2] = "data_air_AoA_angle_deg,data_air_AoS_angle_deg,data_ics_angle,";                                                        // 3個
            break;
          }
        case 2:  // 14個
          {
            str[0] = "fslg_is_alive,data_fslg_bno_qw,data_fslg_bno_qx,data_fslg_bno_qy,data_fslg_bno_qz,";                   // 5個
            str[1] = "data_fslg_bno_roll,data_fslg_bno_pitch,data_fslg_bno_yaw,data_fslg_lsm_roll,data_fslg_lsm_pitch,";     // 5個
            str[2] = "data_fslg_lsm_yaw,data_fslg_bmp_pressure_hPa,data_fslg_bmp_temperature_deg,data_fslg_bmp_altitude_m,";  // 4個
            break;
          }
        case 3:  // 16個
          {
            str[0] = "data_fslg_bno_accx_mss,data_fslg_bno_accy_mss,data_fslg_bno_accz_mss,data_fslg_lsm_accx_mss,data_fslg_lsm_accy_mss, data_fslg_lsm_accz_mss,";    // 6個
            str[1] = "data_fslg_bno_cal_system,data_fslg_bno_cal_gyro,data_fslg_bno_cal_accel,data_fslg_bno_cal_mag,under_is_alive,";                                  // 5個
            str[2] = "data_under_bmp_pressure_hPa,data_under_bmp_temperature_deg,data_under_bmp_altitude_m,data_under_urm_altitude_m,data_under_tsd20_altitude_m\n";  // 5個
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
}




// データをバッファに追加するだけの関数
void addDataToSDBuf(const LogData& data, int flash_mode) {
  if (SD_is_active) {
    memset(SD_BUF, 0, sizeof(SD_BUF));  //SD_BUFを0で初期化

    switch (flash_mode) {  // 計54個
      case 0:              // 計13個
        {
          snprintf(SD_BUF, sizeof(SD_BUF), "%lu,%d,%d,%u,%u,%u,%u,%.7f,%.7f,%.2f,%.2f,%.1f,%u,",
                  data.time_ms, data.takeoff, data.urm_is_reliable, data.data_air_gps_hour,                                                       // 4個
                  data.data_air_gps_minute, data.data_air_gps_second, data.data_air_gps_centisecond, data.data_air_gps_latitude_deg,              // 4個
                  data.data_air_gps_longitude_deg, data.data_air_gps_altitude_m, data.data_air_gps_groundspeed_ms, data.data_air_gps_heading_deg,data_air_gps_satellites  // 5個
          );
          sd.add_str(SD_BUF);
          // sd.flash();
          break;
        }
      case 1:
        {  // 計11個
          snprintf(SD_BUF, sizeof(SD_BUF), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,",
                  data.filtered_bmp_altitude_m, data.filtered_urm_altitude_m, data.filtered_airspeed_ms,            // 3個
                  data.data_air_bmp_pressure_hPa, data.data_air_bmp_temperature_deg, data.data_air_bmp_altitude_m,  // 3個
                  data.data_air_sdp_differentialPressure_Pa, data.data_air_sdp_airspeed_ms,                    // 2個
                  data.data_air_AoA_angle_deg, data.data_air_AoS_angle_deg, data.data_ics_angle);                   // 3個
          sd.add_str(SD_BUF);
          // sd.flash();
          break;
        }
      case 2:  // 計14個
        {
          snprintf(SD_BUF, sizeof(SD_BUF), "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,",
                  data.fslg_is_alive,                                                                         // 1個
                  data.data_fslg_bno_qw, data.data_fslg_bno_qx, data.data_fslg_bno_qy, data.data_fslg_bno_qz,                // 4個
                  data.data_fslg_bno_roll, data.data_fslg_bno_pitch, data.data_fslg_bno_yaw,                            // 3個
                  data.data_fslg_lsm_roll, data.data_fslg_lsm_pitch, data.data_fslg_lsm_yaw,                            // 3個
                  data.data_fslg_bmp_pressure_hPa, data.data_fslg_bmp_temperature_deg, data.data_fslg_bmp_altitude_m);  // 3個
          sd.add_str(SD_BUF);
          // sd.flash();
          break;
        }

      case 3:  // 計16個
        {
          snprintf(SD_BUF, sizeof(SD_BUF), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u,%u,%u,%u,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                  data.data_fslg_bno_accx_mss, data.data_fslg_bno_accy_mss, data.data_fslg_bno_accz_mss,                            // 3個
                  data.data_fslg_lsm_accx_mss, data.data_fslg_lsm_accy_mss, data.data_fslg_lsm_accz_mss,                            // 3個
                  data.data_fslg_bno_cal_system, data.data_fslg_bno_cal_gyro, data.data_fslg_bno_cal_accel, data.data_fslg_bno_cal_mag,  // 4個
                  data.under_is_alive,                                                                                    // 1個
                  data.data_under_bmp_pressure_hPa, data.data_under_bmp_temperature_deg, data.data_under_bmp_altitude_m,            // 3個
                  data.data_under_urm_altitude_m, data.data_under_tsd20_altitude_m);                                           // 2個
          sd.add_str(SD_BUF);
          // sd.flash();
          break;
        }
      default:
        {
          Serial.println("Invalid argument for flashSD()");
          break;
        }
    }
  }
}

// バッファにたまったデータをSDに書き込むだけの関数
void writeSD() {
  digitalWrite(LED1, LOW);

  if (SD_is_active) {
    sd.flash();
    
    digitalWrite(LED1, HIGH);

  }
}

// バッファをそのままSDに書き込む関数
void writeBufToSD(char* buffer) {
  // SDが正常で，バッファが空ではない場合
  if (SD_is_active && buffer != NULL) {
    sd.add_str(buffer);
  }
}



//とりあえず20Hz書き込みで様子見
void flashSD(int flash_mode) {
  if (SD_is_active) {
    memset(SD_BUF, 0, sizeof(SD_BUF));  //SD_BUFを0で初期化

    switch (flash_mode) {  // 計54個
      case 0:              // 計13個
        {
          sprintf(SD_BUF, "%lu,%d,%d,%u,%u,%u,%u,%.7f,%.7f,%.2f,%.2f,%.1f,%u,",
                  time_ms, takeoff, urm_is_reliable, data_air_gps_hour,                                                       // 4個
                  data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond, data_air_gps_latitude_deg,              // 4個
                  data_air_gps_longitude_deg, data_air_gps_altitude_m, data_air_gps_groundspeed_ms, data_air_gps_heading_deg,data_air_gps_satellites  // 5個
          );
          sd.add_str(SD_BUF);
          sd.flash();
          break;
        }
      case 1:
        {  // 計11個
          sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,",
                  filtered_bmp_altitude_m, filtered_urm_altitude_m, filtered_airspeed_ms,            // 3個
                  data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m,  // 3個
                  data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms,                    // 2個
                  data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle);                   // 3個
          sd.add_str(SD_BUF);
          sd.flash();
          break;
        }
      case 2:  // 計14個
        {
          sprintf(SD_BUF, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,",
                  fslg_is_alive,                                                                         // 1個
                  data_fslg_bno_qw, data_fslg_bno_qx, data_fslg_bno_qy, data_fslg_bno_qz,                // 4個
                  data_fslg_bno_roll, data_fslg_bno_pitch, data_fslg_bno_yaw,                            // 3個
                  data_fslg_lsm_roll, data_fslg_lsm_pitch, data_fslg_lsm_yaw,                            // 3個
                  data_fslg_bmp_pressure_hPa, data_fslg_bmp_temperature_deg, data_fslg_bmp_altitude_m);  // 3個
          sd.add_str(SD_BUF);
          sd.flash();
          break;
        }

      case 3:  // 計16個
        {
          sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u,%u,%u,%u,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                  data_fslg_bno_accx_mss, data_fslg_bno_accy_mss, data_fslg_bno_accz_mss,                            // 3個
                  data_fslg_lsm_accx_mss, data_fslg_lsm_accy_mss, data_fslg_lsm_accz_mss,                            // 3個
                  data_fslg_bno_cal_system, data_fslg_bno_cal_gyro, data_fslg_bno_cal_accel, data_fslg_bno_cal_mag,  // 4個
                  under_is_alive,                                                                                    // 1個
                  data_under_bmp_pressure_hPa, data_under_bmp_temperature_deg, data_under_bmp_altitude_m,            // 3個
                  data_under_urm_altitude_m, data_under_tsd20_altitude_m);                                           // 2個
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
}