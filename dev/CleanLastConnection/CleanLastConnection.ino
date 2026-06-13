#include <BluetoothA2DPSource.h>

BluetoothA2DPSource a2dp_source;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  delay(3000);
  a2dp_source.clean_last_connection();
  Serial.println("Cleaned last connection.");
}


void loop() {

}
