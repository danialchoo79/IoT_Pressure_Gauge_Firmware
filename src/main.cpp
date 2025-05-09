#include <Arduino.h>
#include "M5PoECAM.h"

void setup() {
  // put your setup code here, to run once:
  PoECAM.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  PoECAM.update();
  if (PoECAM.BtnA.wasPressed()) {
      Serial.println("BtnA Pressed");
  }
  if (PoECAM.BtnA.wasReleased()) {
      Serial.println("BtnA Released");
  }
}
