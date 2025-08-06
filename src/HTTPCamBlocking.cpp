#include "HTTPCamBlocking.h"

// This is based on a one to one communication using Hot Spot over HTTP

WebServer serverBlocking(80);  // Port 80 (default HTTP)

long freeHeapBlocking = 0;
long freePSRAMBlocking = 0;
int RSSIBlocking = 0;

/* ---------TO USE SPECIFIC IP---------*/
// IPAddress local_IP(192,168,1,184);
// IPAddress gateway(192,168,1,1);
// IPAddress subnet(255,255,255,0);
/* ---------TO USE SPECIFIC IP---------*/

void InitialMemoryCheckBlocking()
{
  freeHeapBlocking = ESP.getFreeHeap();
  freePSRAMBlocking = ESP.getFreePsram();

  Serial.print("Free heap memory (Before Photo): ");
  Serial.println(freeHeapBlocking);
  Serial.print("Free PSRAM (Before Photo): ");
  Serial.println(freePSRAMBlocking);
}

void MemoryCheckBlocking()
{
  freeHeapBlocking = ESP.getFreeHeap();
  freePSRAMBlocking = ESP.getFreePsram();

  Serial.print("Free heap memory (After Photo): ");
  Serial.println(freeHeapBlocking);
  Serial.print("Free PSRAM (After Photo): ");
  Serial.println(freePSRAMBlocking);

  Serial.println();
}

void handleCaptureBlocking()
{
  InitialMemoryCheckBlocking();

    if (PoECAM.Camera.get()) {
        Serial.println("Capture succeeded, sending image...");

        // Set HTTP content headers
        serverBlocking.sendHeader("Content-Type", "image/jpeg");
        serverBlocking.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
        serverBlocking.send_P(200, "image/jpeg", (const char *)PoECAM.Camera.fb->buf, PoECAM.Camera.fb->len); // P is PROGMEM; Sends Data from RAM
        PoECAM.Camera.free();
      } else {
        Serial.println("Capture failed!");
        serverBlocking.send(500, "text/plain", "Camera capture failed");
      }
}

void SetupCameraBlocking()
{
    PoECAM.begin();
    delay(1000);
  
    if (!PoECAM.Camera.begin()) {
      Serial.println("Camera Init Failed");
      return;
    }

    // Camera configuration
    PoECAM.begin();
    PoECAM.Camera.sensor->set_pixformat(PoECAM.Camera.sensor, PIXFORMAT_JPEG);
    PoECAM.Camera.sensor->set_framesize(PoECAM.Camera.sensor, FRAMESIZE_QVGA);
    PoECAM.Camera.sensor->set_vflip(PoECAM.Camera.sensor, 1);
    PoECAM.Camera.sensor->set_hmirror(PoECAM.Camera.sensor, 0);
}

void SetupWiFiBlocking()
{
    /* ---------TO USE SPECIFIC IP---------*/
    // EP32 will always use the same IP on your local network
    // if (!WiFi.config(local_IP, gateway, subnet)) {
    //   Serial.println("STA Failed to configure");
    // }
    /* ---------TO USE SPECIFIC IP---------*/
  
    WiFi.begin(SSIDCAM, PASSWORDCAM);
    WiFi.setSleep(false);
  
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP()); 

    RSSIBlocking = WiFi.RSSI();

    Serial.println();
    Serial.print("Signal strength (RSSI): ");
    Serial.print(RSSIBlocking);
    Serial.println();
}

void PostImageOnServerBlocking()
{
    // Route: /jpg returns a captured image
    serverBlocking.on("/jpg", HTTP_GET, handleCaptureBlocking);

    // Route: default root page with instructions
    serverBlocking.on("/", HTTP_GET, []() {
        serverBlocking.send(200, "text/html", "<html><body><h1>PoECAM</h1><p><a href=\"/jpg\">View Image</a></p></body></html>");
    });

    serverBlocking.begin();
    Serial.println("Web server started");
}

void ImageCaptureHTTPBlocking()
{
  handleCaptureBlocking();
  SetupCameraBlocking();
  SetupWiFiBlocking();
  PostImageOnServerBlocking();
  MemoryCheckBlocking();
}