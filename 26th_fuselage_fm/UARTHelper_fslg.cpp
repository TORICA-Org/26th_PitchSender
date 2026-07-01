#include <Arduino.h>
#include "UARTHelper_fslg.h"
#include "fslg_config.h"
#include "parameters.h"

#define Serial_Bico Serial1    // `Serial_Bico`を`Serial1`としてマクロを登録

//TORICA_UARTインスタンス化
#include <TORICA_UART.h>
TORICA_UART Bico_UART(&Serial_Bico);
char trans_buff[2048];  // 送信する文字列を保存するためのバッファ


void initUART() {

  // UART初期化（<-まだ通信の開始処理はされていない）
  Serial_Bico.setRxBufferSize(1024); // バッファ(受信したデータの一時保管場所)サイズ指定(1024byte)

  // パラメータ設定とともに通信を開始
  // ICS通信の仕様に合わせ，`SERIAL_8E1`としている．
  // `8`:データビットの長さ
  // `E`:偶数パリティ(`N`:パリティなし，`O`:奇数パリティ)
  // `1`:ストップビット(データフレームの終わりを示すビット)の長さ
  Serial_Bico.begin(460800, SERIAL_8E1, SerialRX, SerialTX);

  Serial.begin(115200);  // デバッグ用にパリティはいらないかな...ってか使えない気がする
  Serial.print("loading...\n\n");
}



void transmitLog(int trans_mode) {  // 関数分けるのは面倒なので引数（0~3）でモード変更
  switch (trans_mode) {
    case 0: // 14個
    {
      sprintf(trans_buff, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u,%u,%u,%u,",
        data_fslg_bno_accx_mss, data_fslg_bno_accy_mss, data_fslg_bno_accz_mss, // 3個
        data_fslg_bno_qw, data_fslg_bno_qx, data_fslg_bno_qy, data_fslg_bno_qz, // 4個
        data_fslg_bno_roll, data_fslg_bno_pitch, data_fslg_bno_yaw, // 3個
        data_fslg_bno_cal_system, data_fslg_bno_cal_gyro, data_fslg_bno_cal_accel, data_fslg_bno_cal_mag); // 4個
      break;
    }
    case 1: // 9個
    {
      sprintf(trans_buff, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", 
        data_fslg_bmp_pressure_hPa, data_fslg_bmp_temperature_deg, data_fslg_bmp_altitude_m, // 3個
        data_fslg_lsm_accx_mss, data_fslg_lsm_accy_mss, data_fslg_lsm_accz_mss, // 3個
        data_fslg_lsm_roll, data_fslg_lsm_pitch, data_fslg_lsm_yaw); // 3個
      break;
    }
    default:
    {
      Serial.println("The parameter value is out of range.");
      break;
    }
  }
  //バッファをクリアしてから新しいデータを書き込み
  Serial_Bico.flush();
  Serial_Bico.print(trans_buff);
}


bool bico_is_alive = false;
void receiveLog() {
  bico_is_alive = false;

  digitalWrite(LED2, LOW);

  // エアデータから受信
  static unsigned long int last_bico_time_ms = 0;
  int readnum_bico = Bico_UART.readUART();
  const int bico_data_num = 30;  //正常な場合のデータ受信数

  if (readnum_bico == bico_data_num) {
    
    digitalWrite(LED2, HIGH);

    last_bico_time_ms = millis();
    //受信データを格納
    // 受信データを格納
    // 1回目の受信 13個
    time_ms = Bico_UART.UART_data[0];
    takeoff = static_cast<bool>(Bico_UART.UART_data[1]);
    urm_is_reliable = static_cast<bool>(Bico_UART.UART_data[2]);
    data_air_gps_hour = Bico_UART.UART_data[3];
    data_air_gps_minute = Bico_UART.UART_data[4];
    data_air_gps_second = Bico_UART.UART_data[5];
    data_air_gps_centisecond = Bico_UART.UART_data[6];
    data_air_gps_latitude_deg = Bico_UART.UART_data[7];
    data_air_gps_longitude_deg = Bico_UART.UART_data[8];
    data_air_gps_altitude_m = Bico_UART.UART_data[9];
    data_air_gps_groundspeed_ms = Bico_UART.UART_data[10];
    data_air_gps_heading_deg = Bico_UART.UART_data[11];
    data_air_gps_satellites = Bico_UART.UART_data[12];

    // 2回目の受信 11個
    filtered_bmp_altitude_m = Bico_UART.UART_data[13];
    filtered_urm_altitude_m = Bico_UART.UART_data[14];
    filtered_airspeed_ms = Bico_UART.UART_data[15];
    data_air_bmp_pressure_hPa = Bico_UART.UART_data[16];
    data_air_bmp_temperature_deg = Bico_UART.UART_data[17];
    data_air_bmp_altitude_m = Bico_UART.UART_data[18];
    data_air_sdp_differentialPressure_Pa = Bico_UART.UART_data[19];
    data_air_sdp_airspeed_ms = Bico_UART.UART_data[20];
    data_air_AoA_angle_deg = Bico_UART.UART_data[21];
    data_air_AoS_angle_deg = Bico_UART.UART_data[22];
    data_ics_angle = Bico_UART.UART_data[23];

    // 3回目の受信 6個
    under_is_alive = static_cast<bool>(Bico_UART.UART_data[24]);
    data_under_bmp_pressure_hPa = Bico_UART.UART_data[25];
    data_under_bmp_temperature_deg = Bico_UART.UART_data[26];
    data_under_bmp_altitude_m = Bico_UART.UART_data[27];
    data_under_urm_altitude_m = Bico_UART.UART_data[28];
    data_under_tsd20_altitude_m = Bico_UART.UART_data[29];
  }

  //最終受信時間から1秒以上経過している場合は死んでいるとみなす
  if (millis() - last_bico_time_ms > 1000) {
    bico_is_alive = false;
  } else {
    bico_is_alive = true;
  }
}