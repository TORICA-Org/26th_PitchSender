#include <Arduino.h>
#include "MyBluetooth.h"

#include <math.h> 


//const float c3_frequency  130.81
constexpr float pi_2 = PI * 2.0;
// const float angular_frequency = pi_2 * c3_frequency;

// angular_frequency (radians per second) / sampling rate (samples per second)
// gives radians changed per sample period.
// const float deltaAngle = angular_frequency / 44100.0;

// float deltaAngle = 0.0;
// float interval = 0.0;

// 【共有変数】メインループとオーディオタスクの両方から参照されるため volatile を付与
// target_deltaAngle: 周波数から計算された、1サンプルあたりの位相変化量
// target_samples_interval: ON/OFFが切り替わるまでのサンプル数（時間）
volatile float target_deltaAngle = 0.0;
volatile uint32_t target_samples_interval = 0;

// 【内部状態】get_data_frames内でのみ使用し、音の滑らかさを管理
float current_gain = 0.0; // 現在の増幅率（0.0:無音 〜 1.0:最大音量）


/**
 * 外部（Loop等）から音程とインターバルを設定する関数
 * @param freq 周波数 (Hz)
 * @param interval_sec 音が鳴る/止まるの間隔（秒）。0を指定すると常時再生。
 */
void bt_set_sound(float freq, float interval_sec) {
  // A2DPの標準サンプリングレート 44100Hz に基づいて計算
  target_deltaAngle = (pi_2 * freq) / 44100.0;
  target_samples_interval = (uint32_t)(interval_sec * 44100.0);
}

BluetoothA2DPSource a2dp_source;


// The supported audio codec in ESP32 A2DP is SBC. SBC audio stream is encoded
// from PCM data normally formatted as 44.1kHz sampling rate, two-channel 16-bit sample data
/**
 * Bluetoothオーディオのデータ供給コールバック
 * 数百フレーム単位で呼ばれるが、ループ内で volatile 変数を参照することで
 * フレームの途中でも設定変更を反映させる。
 */
int32_t get_data_frames(Frame *frame, int32_t frame_count) {
  static float m_angle = 0.0;       // 現在の位相（0 〜 2π）
  static uint32_t sample_counter = 0; // 通算サンプル数（インターバル判定用）
  const float max_amp = 20000.0;    // 基本振幅

  for (int i = 0; i < frame_count; ++i) {
    
    // 1. 【インターバル判定】
    // 現在のサンプル位置が「鳴らすべき区間」か「止めるべき区間」かを判定
    bool should_be_on = true;
    if (target_samples_interval > 0) {
      // 経過サンプル数をインターバル幅で割り、偶数ならON、奇数ならOFF
      should_be_on = (sample_counter / target_samples_interval) % 2 == 0;
    }

    // 2. 【ノイズ低減（エンベロープ処理）】
    // 振幅を急激に 0 <-> 10000 で切り替えると「プツッ」というクリックノイズが出るため、
    // current_gain を使って数ミリ秒かけて滑らかに音量を変化させる（簡易フェード）。
    if (should_be_on) {
      if (current_gain < 1.0f) {
        current_gain += 0.005f; // フェードイン（約200サンプルで最大音量へ）
        if (current_gain > 1.0f) current_gain = 1.0f;
      }
    } else {
      if (current_gain > 0.0f) {
        current_gain -= 0.005f; // フェードアウト
        if (current_gain < 0.0f) current_gain = 0.0f; // 負の数にならないようガード
      }
    }

    // 3. 【波形生成】
    // 正弦波に現在のゲインを乗算してサンプル値を決定
    float sample_val = max_amp * current_gain * sin(m_angle);
    frame[i].channel1 = (int16_t)sample_val;
    frame[i].channel2 = (int16_t)sample_val;

    // 4. 【パラメータ更新】
    // 次のサンプルのために位相を進める
    m_angle += target_deltaAngle;
    if (m_angle > pi_2) m_angle -= pi_2;

    // サンプルカウンタをインクリメント
    sample_counter++;

    // カウンタが無限に増えて計算精度が落ちるのを防ぐため、
    // 現在のインターバル周期（2倍分）で剰余をとってリセット
    if (target_samples_interval > 0) {
      sample_counter %= (target_samples_interval * 2);
    }
  }
  return frame_count;
}


/**
 * @brief Buetooth A2DP connection states
 * @ingroup a2dp
 *
enum esp_a2d_connection_state_t {
    ESP_A2D_CONNECTION_STATE_DISCONNECTED = 0, /*!< connection released  *
    ESP_A2D_CONNECTION_STATE_CONNECTING,       /*!< connecting remote device *
    ESP_A2D_CONNECTION_STATE_CONNECTED,        /*!< connection established *
    ESP_A2D_CONNECTION_STATE_DISCONNECTING     /*!< disconnecting remote device *
} ;
*/

#define LENGTH 16
char bt_status[LENGTH];

// for esp_a2d_connection_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv426esp_a2d_connection_state_t
void connection_state_changed(esp_a2d_connection_state_t state, void *ptr){
  // Serial.println();
  // Serial.println(a2dp_source.to_str(state));
  memset(bt_status, 0, LENGTH);
  delayMicroseconds(10);
  strcpy(bt_status, a2dp_source.to_str(state));
}


void bt_init(const char *bt_name) {
  sprintf(bt_status, "Starting");
  a2dp_source.set_auto_reconnect(false);
  a2dp_source.set_data_callback_in_frames(get_data_frames);
  a2dp_source.set_on_connection_state_changed(connection_state_changed);
  a2dp_source.set_volume(50);
  a2dp_source.start(bt_name);
  /*
  while(!a2dp_source.is_connected()) {
    Serial.print(".");
    delay(1000);
  }
  */
  a2dp_source.set_auto_reconnect(true);
}
