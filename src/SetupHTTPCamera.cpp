#include "SetupHTTPCamera.h"

WebServer server(80);  // Port 80 (default HTTP)

long freeHeap = 0;
long freePSRAM = 0;
int RSSI = 0;

// IPAddress local_IP(192,168,1,184);
// IPAddress gateway(192,168,1,1);
// IPAddress subnet(255,255,255,0);

void InitialMemoryCheck()
{
  freeHeap = ESP.getFreeHeap();
  freePSRAM = ESP.getFreePsram();

  Serial.print("Free heap memory (Before Photo): ");
  Serial.println(freeHeap);
  Serial.print("Free PSRAM (Before Photo): ");
  Serial.println(freePSRAM);
}

void handleCapture()
{
  Serial.begin(115200);
  Serial.println();
  InitialMemoryCheck();

    if (PoECAM.Camera.get()) {
        Serial.println("Capture succeeded, sending image...");

        // Set HTTP content headers
        server.sendHeader("Content-Type", "image/jpeg");
        server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
        server.send_P(200, "image/jpeg", (const char *)PoECAM.Camera.fb->buf, PoECAM.Camera.fb->len); // P is PROGMEM; Sends Data from RAM
        PoECAM.Camera.free();
      } else {
        Serial.println("Capture failed!");
        server.send(500, "text/plain", "Camera capture failed");
      }
}

void SetupCamera()
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

void SetupWiFi()
{
    // EP32 will always use the same IP on your local network
    // if (!WiFi.config(local_IP, gateway, subnet)) {
    //   Serial.println("STA Failed to configure");
    // }
  
    WiFi.begin(SSID, PASSWORD);
    WiFi.setSleep(false);
  
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP()); 

    RSSI = WiFi.RSSI();

    Serial.println();
    Serial.print("Signal strength (RSSI): ");
    Serial.print(RSSI);
    Serial.println();
}

void PostImageOnServer()
{
    // Route: /jpg returns a captured image
    server.on("/jpg", HTTP_GET, handleCapture);

    // Route: default root page with instructions
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", "<html><body><h1>PoECAM</h1><p><a href=\"/jpg\">View Image</a></p></body></html>");
    });

    server.begin();
    Serial.println("Web server started");
}

void MemoryCheck()
{
  freeHeap = ESP.getFreeHeap();
  freePSRAM = ESP.getFreePsram();

  Serial.print("Free heap memory (After Photo): ");
  Serial.println(freeHeap);
  Serial.print("Free PSRAM (After Photo): ");
  Serial.println(freePSRAM);

  Serial.println();
}