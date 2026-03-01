#include <Arduino.h>
#include "MyBluetooth.h"

A2DPStream out;

void initBT() {
  auto cfg = out.defaultConfig(TX_MODE);
  cfg.name = "BTW13X";
  cfg.auto_reconnect = true;

  if (!out.begin(cfg)) {
    Serial.println("A2DPStream begin failed");
    while (true);
  }
  Serial.println("Connecting to headphones...");

  while(!out.source().is_connected()){
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}

