#ifndef SETUPHTTPCAMERA_H
#define SETUPHTTPCAMERA_H

#include "M5PoECAM.h"
#include <WiFi.h>
#include <WebServer.h>  // ESP32 Web Server

#define SSID      "Danials Iphone"
#define PASSWORD  "Danial1997."

extern WebServer server;
extern long freeHeap;
extern long freePSRAM;
extern int RSSI;

extern IPAddress localIP;
extern IPAddress gateway;
extern IPAddress subnet;


void InitialMemoryCheck();
void handleCapture();
void SetupCamera();
void SetupWiFi();
void PostImageOnServer();
void MemoryCheck();


#endif