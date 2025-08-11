#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "M5PoECam.h"  // Your camera library
#include <Ticker.h>
#include "LED.h"

#define WIFI_SSID      "EQT-WLAN"
#define WIFI_PASSWORD  "3qu1pm3nt"

#define DEVICE         "cam1"
#define LAST_WILL      "lwt"

#define MQTT_HOST      IPAddress(128, 53, 209, 104)  // Broker IP
#define MQTT_PORT      1883

// Topics
#define MQTT_TOPIC     "cameras/" DEVICE "/image"
#define LASTWILL_TOPIC "system/status/" DEVICE
#define CMD_TOPIC      "cameras/" DEVICE "/cmd"

const char* HEARTBEAT_TOPIC = "system/heartbeat/" DEVICE;

// Network config (optional static IP)
IPAddress local_IPMQTT2(128, 53, 209, 115);
IPAddress gatewayMQTT2(128, 53, 208, 1);
IPAddress subnetMQTT2(255, 255, 248, 0);
IPAddress PrimarydnsMQTT2(128, 53, 1, 20);
IPAddress SecondarydnsMQTT2(10, 53, 8, 20);

AsyncMqttClient mqttClient;

// Replace FreeRTOS timers with Tickers
Ticker mqttReconnectTicker;
Ticker wifiReconnectTicker;

// Heartbeat Configurations
const unsigned long heartbeatInterval = 30000; // 30 seconds

// Tickers
Ticker heartbeatTicker;
Ticker ledOffTicker;

void turnOffLEDsCallback() {
  TurnOFFAllColorLED();
}

void connectToWifi() {
  Serial.println("Connecting to WiFi...");
  if (!WiFi.config(local_IPMQTT2, gatewayMQTT2, subnetMQTT2, PrimarydnsMQTT2, SecondarydnsMQTT2)) {
    Serial.println("❌ Failed to configure static IP");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
  } else {
    Serial.println("WiFi not connected. Cannot connect to MQTT.");
  }
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      wifiReconnectTicker.detach();  // Stop WiFi reconnect attempts
      connectToMqtt();
      break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      mqttReconnectTicker.detach();  // Stop MQTT reconnect attempts
      wifiReconnectTicker.once(2, connectToWifi);  // Retry WiFi connection in 2 seconds
      break;

    default:
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("MQTT Connected");

  mqttReconnectTicker.detach();  // Stop MQTT reconnect attempts

  // Subscribe to Broker Heartbeat
  mqttClient.subscribe("system/heartbeat/+", 1);

  // Subscribe to Command Topic
  mqttClient.subscribe(CMD_TOPIC, 1);

  // Turn on all LED (White Colour)
  TurnONAllWhiteLED();
}

void captureAndPublishPhoto() {
  TurnONAllWhiteLED();

  Serial.println("Capturing image...");

  if (!PoECAM.Camera.get()) {
    Serial.println("Failed to capture image");
    delay(1000);
    PoECAM.Camera.get(); // Try Again
    if (!PoECAM.Camera.get()) {
      Serial.println("Second Attempt Failed. Skipping...");
      return;
    }
    if (!PoECAM.Camera.fb) {
      Serial.println("No framebuffer available");
      return;
    }
  }

  size_t jpeg_len = PoECAM.Camera.fb->len;
  uint8_t* jpeg_data = PoECAM.Camera.fb->buf;

  Serial.printf("✅ Captured JPEG: %d bytes\n", jpeg_len);

  bool ok = mqttClient.publish(MQTT_TOPIC, 1, false, (const char*)jpeg_data, jpeg_len);
  if (ok) {
    Serial.println("MQTT publish succeeded.");
  } else {
    Serial.println("MQTT publish failed.");
  }

  PoECAM.Camera.free();

  ledOffTicker.once(2, turnOffLEDsCallback);
}

// MQTT message handler
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties,
                   size_t len, size_t index, size_t total) {
  String topicStr = String(topic);
  String payloadStr;

  for (size_t i = 0; i < len; i++) {
    payloadStr += (char)payload[i];
  }

  payloadStr.trim();

  if (topicStr == CMD_TOPIC) {
    Serial.printf("Command received from %s -> %s\n", topic, payloadStr.c_str());
    if (payloadStr.equalsIgnoreCase("take_photo")) {
      captureAndPublishPhoto();
    }
    return;
  }

  if (topicStr.startsWith("system/heartbeat/")) {
    Serial.printf("Heartbeat received from %s -> %s\n", topic, payloadStr.c_str());
  }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  Serial.printf("Error Code: %u\n", static_cast<uint8_t>(reason));
  if (WiFi.isConnected()) {
    mqttReconnectTicker.once(2, connectToMqtt);  // Retry MQTT connect in 2 seconds
  }
}

void setupCamera() {
  PoECAM.begin();
  delay(1000);

  if (!PoECAM.Camera.begin()) {
    Serial.println("❌ Camera Init Failed");
    while (true) delay(1000);
  }

  PoECAM.Camera.sensor->set_pixformat(PoECAM.Camera.sensor, PIXFORMAT_JPEG);
  PoECAM.Camera.sensor->set_framesize(PoECAM.Camera.sensor, FRAMESIZE_QVGA);
  PoECAM.Camera.sensor->set_vflip(PoECAM.Camera.sensor, 1);
  PoECAM.Camera.sensor->set_hmirror(PoECAM.Camera.sensor, 0);

  Serial.println("📸 Camera initialized");
}

// Heartbeat sender
void heartbeatMQTT() {
  if (mqttClient.connected()) {
    String heartbeatMsg = "alive";
    bool heartbeat = mqttClient.publish(HEARTBEAT_TOPIC, 1, false, heartbeatMsg.c_str());

    if (heartbeat) {
      Serial.print(DEVICE);
      Serial.println(" : Heartbeat sent.");
    } else {
      Serial.print(DEVICE);
      Serial.println(" : Heartbeat failed to send.");
    }
  } else {
    Serial.println("MQTT not connected. Heartbeat not sent.");
  }
}

void setup() {
  Serial.begin(115200);

  setupCamera();
  SetupLEDPin();

  mqttClient.setWill(LASTWILL_TOPIC, 1, true, "disconnected");
  mqttClient.setClientId(DEVICE);

  WiFi.onEvent(WiFiEvent);

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);

  heartbeatTicker.attach_ms(heartbeatInterval, heartbeatMQTT);

  connectToWifi();
}

void loop() {
  // Nothing needed here for async client and Ticker timers
}
