#include "SetupHTTPCamera.h"

void setup() {
  handleCapture();
  SetupCamera();
  SetupWiFi();
  PostImageOnServer();
  MemoryCheck();
}

void loop() {
  server.handleClient();
}
