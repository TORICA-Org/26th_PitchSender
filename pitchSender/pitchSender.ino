#include "MyBluetooth.h"
#include "MyIMU.h"
#include "MyKalman.h"

void setup() {
  Serial.begin(115200);
  initBT();
  initIMU();
  initKalman();
}

void loop() {
  Serial.println(getEstPitch());
  delay(100);
}

