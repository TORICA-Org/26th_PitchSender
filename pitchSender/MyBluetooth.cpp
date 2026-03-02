#include <Arduino.h>
#include "MyBluetooth.h"

#include <math.h> 


//const float c3_frequency  130.81
constexpr float pi_2 = PI * 2.0;
// const float angular_frequency = pi_2 * c3_frequency;

// angular_frequency (radians per second) / sampling rate (samples per second)
// gives radians changed per sample period.
// const float deltaAngle = angular_frequency / 44100.0;

volatile float deltaAngle = 0.0;

void set_sound(float freq, float interval) {

  float angular_freq = pi_2 * freq;

  deltaAngle = angular_freq / 44100.0;
}

BluetoothA2DPSource a2dp_source;

// The supported audio codec in ESP32 A2DP is SBC. SBC audio stream is encoded
// from PCM data normally formatted as 44.1kHz sampling rate, two-channel 16-bit sample data
int32_t get_data_frames(Frame *frame, int32_t frame_count) {
    static float m_angle = 0.0;
    float m_amplitude = 10000.0;  // -32,768 to 32,767
    float m_phase = 0.0;
    // fill the channel data
    for (int sample = 0; sample < frame_count; ++sample) {
        frame[sample].channel1 = m_amplitude * sin(m_angle + m_phase);
        frame[sample].channel2 = frame[sample].channel1;
        m_angle += deltaAngle;
        if (m_angle > pi_2) m_angle -= pi_2;
    }
    // to prevent watchdog
    delay(1);

    return frame_count;
}


void init_bt() {
  a2dp_source.set_auto_reconnect(false);
  a2dp_source.set_data_callback_in_frames(get_data_frames);
  a2dp_source.set_volume(30);
  a2dp_source.start("LEXON MINO L");  
}
