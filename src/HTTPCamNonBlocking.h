#ifndef HTTPCAMNONBLOCKING_H
#define HTTPCAMNONBLOCKING_H

#include "M5PoECAM.h"
#include <Wifi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebServer.h>  // ESP32 Web Server

extern const char* ssid;
extern const char* password;

extern AsyncWebServer server;

extern long freeHeap;
extern long freePSRAM;
extern int RSSI;

extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress dns;

void InitialMemoryCheck();
void MemoryCheck();
void SetupWiFi();
void SetupCamera();
void PostImageOnServer();
void ImageCaptureHTTP();

#endif