#include "HTTPCamNonBlocking.h"

const char* ssid = "EQT-WLAN";
const char* password = "3qu1pm3nt";

IPAddress local_IP(128, 53, 209, 114); // Free IP Network
IPAddress gateway(128, 53, 209, 1);    // Router IP
IPAddress subnet(255, 255, 255, 0);    // Standard Subnet
IPAddress dns(8, 8, 8, 8);             // Optional: Google DNS

AsyncWebServer server(80); // Port 80 (default HTTP)

long freeHeap = 0;
long freePSRAM = 0;
int RSSI = 0;

void InitialMemoryCheck()
{
    freeHeap = ESP.getFreeHeap();
    freePSRAM = ESP.getFreePsram();
    Serial.printf("Free heap (Before): %ld, PSRAM: %ld\n", freeHeap, freePSRAM);
}

void MemoryCheck()
{
    freeHeap = ESP.getFreeHeap();
    freePSRAM = ESP.getFreePsram();
    Serial.printf("Free heap (After): %ld, PSRAM: %ld\n", freeHeap, freePSRAM);
}

void SetupWiFi()
{
    // Set static IP before starting WiFi
    if (!WiFi.config(local_IP, gateway, subnet, dns)) {
        Serial.println("❌ Failed to configure static IP");
    }

    WiFi.begin(ssid, password); // Triggers DHCP
    WiFi.setSleep(false);
  
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  
    RSSI = WiFi.RSSI();
    Serial.printf("Signal strength (RSSI): %d\n", RSSI);
}

void SetupCamera()
{
    PoECAM.begin();
    delay(1000);

    if (!PoECAM.Camera.begin()) {
        Serial.println("Camera Init Failed");
        return;
    }

    PoECAM.Camera.sensor->set_pixformat(PoECAM.Camera.sensor, PIXFORMAT_JPEG);
    PoECAM.Camera.sensor->set_framesize(PoECAM.Camera.sensor, FRAMESIZE_QVGA);
    PoECAM.Camera.sensor->set_vflip(PoECAM.Camera.sensor, 1);
    PoECAM.Camera.sensor->set_hmirror(PoECAM.Camera.sensor, 0);
}

void PostImageOnServer()
{
    // Route for image capture
    server.on("/jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Handling image capture request");
        InitialMemoryCheck();

        if (PoECAM.Camera.get()) {
        Serial.println("Capture succeeded, sending image...");

        AsyncWebServerResponse *response = request->beginResponse_P(
            200,
            "image/jpeg",
            (const uint8_t *)PoECAM.Camera.fb->buf,
            PoECAM.Camera.fb->len
        );

        response->addHeader("Content-Disposition", "inline; filename=capture.jpg");
        request->send(response);

        PoECAM.Camera.free();
        } else {
        Serial.println("Capture failed!");
        request->send(500, "text/plain", "Camera capture failed");
        }

        MemoryCheck();
    });

    // Root route
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html",
        "<html><body><h1>PoECAM</h1><p><a href=\"/jpg\">View Image</a></p></body></html>");
    });

    server.begin();
    Serial.println("Async web server started");
}

void ImageCaptureHTTP()
{
    PostImageOnServer();    // Sets Up Server Routes
    MemoryCheck();          // Check RAM Usage
}
