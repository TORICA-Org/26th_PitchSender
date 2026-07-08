#include <Arduino.h>
#include "UARTHelper_fslg.h"
#include "fslg_config.h"

#define Serial_Bico Serial1    // `Serial_Bico`を`Serial1`としてマクロを登録

//TORICA_UARTインスタンス化
#include <TORICA_UART.h>
TORICA_UART Bico_UART(&Serial_Bico);
char trans_buff[512];  // 送信する文字列を保存するためのバッファ


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


void receiveLog() {
  // エアデータから受信
  static unsigned long int last_bico_time_ms = 0;
  int readnum_bico = Bico_UART.readUART();
  const int bico_data_num = 6;  //正常な場合のデータ受信数

  if (readnum_bico == bico_data_num) {
    last_bico_time_ms = millis();
    //受信データを格納
    data_under_bmp_pressure_hPa = Under_UART.UART_data[0];
    data_under_bmp_temperature_deg = Under_UART.UART_data[1];
    data_under_bmp_altitude_m = Under_UART.UART_data[2];
    data_under_urm_altitude_m = Under_UART.UART_data[3];
    data_under_tsd20_altitude_m = Under_UART.UART_data[4];
    takeoff = Under_UART.UART_data[5];
  }

  //最終受信時間から1秒以上経過している場合は死んでいるとみなす
  // if (millis() - last_bico_time_ms > 1000) {
  //   bico_is_alive = false;
  // } else {
  //   bico_is_alive = true;
  // }
  
}