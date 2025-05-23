#include "HTTPCamNonBlocking.h"
#include "HTTPCamBlocking.h"
#include "LED.h"
#include "Serial.h"

void setup() {
  Serial.begin(115200);
  Serial.println();

  SetupWiFi();            // Setup Routes and Starts Server
  SetupCamera();          // Initialise Camera

  ReserveMemoryForSerial(64);
}

void loop() {
  // server.handleClient();
  // TurnAllColorLED1By1();
  
  ServiceSerialCommand();
}
