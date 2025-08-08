#include "HTTPCamNonBlocking.h"
#include "HTTPCamBlocking.h"
#include "LED.h"
#include "Serial.h"
#include "MQTTBase64.h"
#include "PingMQTTBroker.h"

// void setup() {
//   Serial.begin(115200);
//   Serial.println();

//   Serial.println("Setting up Device...");

//   SetupWiFi();            // Setup Routes and Starts Server
//   SetupCamera();          // Initialise Camera
//   SetupLEDPin();

//   TurnONAllColorLED();

//   // ReserveMemoryForSerial(64);
  
//   // PostImageOnServer();
//   // delay(1000);
//   // TurnOFFAllColorLED();

//   // SetupWifiMQTTBase64();
//   // SetupCameraMQTTBase64();
//   // sendImageMQTT();
//   // sendImageMQTT_RawJPEG();
// }

// void loop() {
//   // ServiceSerialCommand();
//   // TurnONAllColorLED();

//   // if (!client.connected()) reconnect();
//   // client.loop();

//   // Publish a test message every 5 seconds
//   // static unsigned long lastMsg = 0;
//   // if (millis() - lastMsg > 5000) {
//   //   lastMsg = millis();
//   //   String testMsg = "hello from ESP32";
//   //   Serial.print("Publishing message: ");
//   //   Serial.println(testMsg);
//   //   client.publish(mqtt_topic, testMsg.c_str());
//   // }
// }

#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "M5PoECam.h"  // Include your camera library
#include <Ticker.h>

#define WIFI_SSID      "EQT-WLAN"
#define WIFI_PASSWORD  "3qu1pm3nt"

#define DEVICE         "cam1"

#define MQTT_HOST      IPAddress(128, 53, 209, 104)  // Broker IP
#define MQTT_PORT      1883

// Topics
#define MQTT_TOPIC     "cameras/" DEVICE "/image"
#define STATUS_TOPIC   "system/status/" DEVICE

const char* HEARTBEAT_TOPIC = "system/heartbeat/" DEVICE;

// Network config (optional static IP)
IPAddress local_IPMQTT2(128, 53, 209, 115);
IPAddress gatewayMQTT2(128, 53, 208, 1);
IPAddress subnetMQTT2(255, 255, 248, 0);
IPAddress PrimarydnsMQTT2(128, 53, 1, 20);
IPAddress SecondarydnsMQTT2(10, 53, 8, 20);

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

// Heartbeat Configurations
const unsigned long heartbeatInterval = 30000; // 30seconds

// Tickers
Ticker heartbeatTicker;
Ticker ledOffTicker;

void turnOffLEDsCallback() {
  TurnOFFAllColorLED();
}

void connectToWifi() {
  if (!WiFi.config(local_IPMQTT2, gatewayMQTT2, subnetMQTT2, PrimarydnsMQTT2, SecondarydnsMQTT2)) {
    Serial.println("❌ Failed to configure static IP");
  }

  WiFi.mode(WIFI_STA);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi has connected successfully.");
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");

      // Restart Timer Logic
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;
    default:
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("MQTT Connected");

  // Subscribe to Broker Heartbeat
  mqttClient.subscribe("system/heartbeat/+", 1);

  // Turn on all LED (White Colour)
  TurnONAllWhiteLED();

  // Capture image
  Serial.println("Capturing image...");
  if (!PoECAM.Camera.get()) {
    Serial.println("Failed to capture image");
    delay(1000);
    PoECAM.Camera.get(); // Try Again
    if(!PoECAM.Camera.get()) {
      Serial.println("Second Attempt Failed. Skipping...");
      return;
    }
  }

  size_t jpeg_len = PoECAM.Camera.fb->len;
  uint8_t* jpeg_data = PoECAM.Camera.fb->buf;

  Serial.printf("✅ Captured JPEG: %d bytes\n", jpeg_len);

  // Publish JPEG as binary
  bool ok = mqttClient.publish(MQTT_TOPIC, 1, false, (const char*)jpeg_data, jpeg_len);
  if (ok) {
    Serial.println("MQTT publish succeeded.");
  } else {
    Serial.println("MQTT publish failed.");
  }

  PoECAM.Camera.free();

  ledOffTicker.once(2, turnOffLEDsCallback);
}

// Receiving Heartbeat messages from Broker
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String topicStr = String(topic);
  String payloadStr;

  // Get the final payload by appending them based on length
  for(size_t i=0; i<len; i++) {
    payloadStr += (char)payload[i];
  }

  // Handles Heartbeat messages from the broker
  if(topicStr.startsWith("system/heartbeat/")) {
    Serial.printf("Heartbeat received from %s -> %s\n", topic, payloadStr.c_str());
  }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    // Restarts Timer
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void setupCamera() {
  PoECAM.begin();
  delay(1000);

  if (!PoECAM.Camera.begin()) {
    Serial.println("❌ Camera Init Failed");
    while (true) delay(1000);
  }

  // PoECAM Configurations
  PoECAM.Camera.sensor->set_pixformat(PoECAM.Camera.sensor, PIXFORMAT_JPEG);
  PoECAM.Camera.sensor->set_framesize(PoECAM.Camera.sensor, FRAMESIZE_QVGA);
  PoECAM.Camera.sensor->set_vflip(PoECAM.Camera.sensor, 1);
  PoECAM.Camera.sensor->set_hmirror(PoECAM.Camera.sensor, 0);

  Serial.println("📸 Camera initialized");
}

// Heartbeat Topic
void heartbeatMQTT() {
  if(mqttClient.connected()) {
    String heartbeatMsg = "alive";
    bool heartbeat = mqttClient.publish(HEARTBEAT_TOPIC, 1, false, heartbeatMsg.c_str());

    if (heartbeat) {
      Serial.print(DEVICE);
      Serial.println(" : Heartbeat sent.");
    } else {
      Serial.print(DEVICE);
      Serial.println(" : Heartbeat failed to send.");
    }
  }

  else {
    Serial.println("MQTT not connected. Heartbeat not sent.");
  }
}

void setup() {
  Serial.begin(115200);

  // PoECAM
  setupCamera();

  // Ring LED
  SetupLEDPin();

  // WiFi Events
  WiFi.onEvent(WiFiEvent);

  // Timers
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, nullptr, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, nullptr, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  // MQTT Configurations
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setWill(STATUS_TOPIC, 1, true, "disconnected");

  // MQTT Callbacks
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);

  // Heartbeat Configurations
  mqttClient.onMessage(onMqttMessage);
  heartbeatTicker.attach(30, heartbeatMQTT); // Calls every 30 seconds

  // Connect to WiFi - Set Last to prevent Race Conditions etc.
  connectToWifi();
}

void loop() {
  // Empty loop: async handled
}