#include <Arduino.h>
#include "Frequency.h"

constexpr float FREQ_A0 = 27.500;
constexpr float FREQ_B0 = 30.868;
constexpr float FREQ_C1 = 32.703;
constexpr float FREQ_D1 = 36.708;
constexpr float FREQ_E1 = 41.203;
constexpr float FREQ_F1 = 43.654;
constexpr float FREQ_G1 = 48.999;

float get_frequency(unsigned char key[]) {
  unsigned char pitch_name = key[0];
  uint8_t octave = key[1];
  float frequency = 0.0;

  if (pitch_name == 'A') {
    frequency = FREQ_A0 * pow(2, octave);
  }
  else if (pitch_name == 'B') {
    frequency = FREQ_B0 * pow(2, octave);
  }
  else if (pitch_name == 'C') {
    frequency = FREQ_C1 * pow(2, octave - 1);
  }
  else if (pitch_name == 'D') {
    frequency = FREQ_D1 * pow(2, octave - 1);
  }
  else if (pitch_name == 'E') {
    frequency = FREQ_E1 * pow(2, octave - 1);
  }
  else if (pitch_name == 'F') {
    frequency = FREQ_F1 * pow(2, octave - 1);
  }
  else if (pitch_name == 'G') {
    frequency = FREQ_G1 * pow(2, octave - 1);
  }

  return frequency;
}