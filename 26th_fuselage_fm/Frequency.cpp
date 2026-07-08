#include <Arduino.h>
#include "Frequency.h"

constexpr float FREQ_A0 = 27.500;
constexpr float FREQ_B0 = 30.868;
constexpr float FREQ_C1 = 32.703;
constexpr float FREQ_D1 = 36.708;
constexpr float FREQ_E1 = 41.203;
constexpr float FREQ_F1 = 43.654;
constexpr float FREQ_G1 = 48.999;

float frequency_get(const char *key) {
  char pitch_name = key[0];
  String octave_char = String(key[1]);
  int octave = octave_char.toInt();
  float frequency = 0.0;

  if (pitch_name == 'A' && octave >= 0) {
    frequency = FREQ_A0 * pow(2, octave);
  }
  else if (pitch_name == 'B' && octave >= 0) {
    frequency = FREQ_B0 * pow(2, octave);
  }
  else if (pitch_name == 'C' && octave >= 1) {
    frequency = FREQ_C1 * pow(2, octave - 1);
  }
  else if (pitch_name == 'D' && octave >= 1) {
    frequency = FREQ_D1 * pow(2, octave - 1);
  }
  else if (pitch_name == 'E' && octave >= 1) {
    frequency = FREQ_E1 * pow(2, octave - 1);
  }
  else if (pitch_name == 'F' && octave >= 1) {
    frequency = FREQ_F1 * pow(2, octave - 1);
  }
  else if (pitch_name == 'G' && octave >= 1) {
    frequency = FREQ_G1 * pow(2, octave - 1);
  }
  else {
    frequency = 0.0;
  }

  return frequency;
}