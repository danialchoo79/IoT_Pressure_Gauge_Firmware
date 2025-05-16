#include "SetupHTTPCamera.h"
#include "LED.h"

void setup() {
  handleCapture();
  SetupCamera();
  SetupWiFi();
  PostImageOnServer();
  MemoryCheck();
  // SetupLEDPin();
}

void loop() {
  server.handleClient();
  // TurnAllColorLED();
}
