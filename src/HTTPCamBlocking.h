#ifndef SETUPHTTPCAMERA_H
#define SETUPHTTPCAMERA_H

#include "M5PoECAM.h"
#include <WiFi.h>
#include <WebServer.h>  // ESP32 Web Server

#define SSIDCAM      "Danials Iphone"
#define PASSWORDCAM  "Danial1997."

extern WebServer serverBlocking;
extern long freeHeapBlocking;
extern long freePSRAMBlocking;
extern int RSSIBlocking;

extern IPAddress localIPBlocking;
extern IPAddress gatewayBlocking;
extern IPAddress subnetBlocking;

void InitialMemoryCheckBlocking();
void handleCaptureBlocking();
void SetupCameraBlocking();
void SetupWiFiBlocking();
void PostImageOnServerBlocking();
void MemoryCheckBlocking();
void ImageCaptureHTTPBlocking();

#endif