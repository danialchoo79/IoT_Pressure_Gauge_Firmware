#include "MQTTBase64.h"

// IPAddress local_IPMQTT(128, 53, 209, 114); // Free IP Network
// IPAddress gatewayMQTT(128, 53, 209, 1);    // Router IP
// IPAddress subnetMQTT(255, 255, 255, 0);    // Standard Subnet
// IPAddress dnsMQTT(8, 8, 8, 8);             // Optional: Google DNS

IPAddress local_IPMQTT(128, 53, 209, 115);  // Pick a free IP in your router's subnet
IPAddress gatewayMQTT(128, 53, 208, 1);   // Router's IP (gateway)
IPAddress subnetMQTT(255, 255, 248, 0);  // Common subnet mask
IPAddress PrimarydnsMQTT(128, 53, 1, 20);           // Google DNS or your router's DNS
IPAddress SecondarydnsMQTT(10, 53, 8, 20);           // Google DNS or your router's DNS
IPAddress brokerIP(128, 53, 209, 104); // change back to 114 for windows


// Wifi Details
const char* ssidmqttbase64 = "EQT-WLAN";
const char* passwordmqttbase64 = "3qu1pm3nt";
// const char* ssidmqttbase64 = "Danials Iphone";
// const char* passwordmqttbase64 = "Danial1997.";


// MQTT Broker info
// const char* mqtt_server = "128.53.209.120";
// const char* mqtt_server = "169.254.168.25";
// const char* mqtt_server = "172.20.10.3";
const char* mqtt_server = "128.53.209.104"; // change back to 104 for windows
const int mqtt_port = 1883;
const char* mqtt_topic = "cameras/cam1/image";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length);  // 👈 Add this at the top

void SetupWifiMQTTBase64()
{
    if (!WiFi.config(local_IPMQTT, gatewayMQTT, subnetMQTT, PrimarydnsMQTT, SecondarydnsMQTT)) {
        Serial.println("❌ Failed to configure static IP");
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidmqttbase64, passwordmqttbase64);
    Serial.println("Connecting to WiFi");
    
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    PingMQTTBroker(brokerIP);

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    client.setBufferSize(4096);

    Serial.println("\nConnected to MQTT!");

    Serial.print("WiFi IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("Connected to MQTT Broker at: ");
    Serial.println(mqtt_server);
}

void SetupCameraMQTTBase64()
{
    PoECAM.begin();
    delay(1000);

    if(!PoECAM.Camera.begin()) {
        Serial.println("Camera Init Failed");
        while (true) delay(1000);
    }

    PoECAM.Camera.sensor->set_pixformat(PoECAM.Camera.sensor, PIXFORMAT_JPEG);
    PoECAM.Camera.sensor->set_framesize(PoECAM.Camera.sensor, FRAMESIZE_QVGA);
    PoECAM.Camera.sensor->set_vflip(PoECAM.Camera.sensor, 1);
    PoECAM.Camera.sensor->set_hmirror(PoECAM.Camera.sensor, 0);

    Serial.println("Camera is Setup Properly");
}

void reconnect()
{
    while(!client.connected()) {
        
        Serial.println("Attempting MQTT Connection...");

        String clientId = "ESP32CamClient-";
        clientId += String(random(0xffff), HEX);

        client.connect(clientId.c_str());

        if(client.connect(clientId.c_str())) {
            client.subscribe("cameras/cam1/response");
            Serial.println("Connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds.");
            delay(5000);
        }
    }
}

void sendImageMQTT()
{
    if (!PoECAM.Camera.get()) {
        Serial.println("Failed to capture image.");
        return;
    }

    Serial.println("Image captured, encoding base64...");

    // Base64 encode JPEG Buffer
    String base64Image = base64::encode((uint8_t*)PoECAM.Camera.fb->buf, PoECAM.Camera.fb->len);

    // Print length of the encoded string
    Serial.print("Base64 image size (chars): ");
    Serial.println(base64Image.length());

    // Optional: print raw JPEG buffer size
    Serial.print("Raw JPEG size (bytes): ");
    Serial.println(PoECAM.Camera.fb->len);

    // Prepare JSON
    DynamicJsonDocument doc(16384);
    doc["camera_id"] = "cam1";
    doc["timestamp"] = "2025-07-16T15:20:00Z";
    doc["image_id"] = "cam1_20250716_152000";
    doc["image_b64"] = base64Image;

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.print("Json String Length: ");
    Serial.println(jsonString.length());

    // Publish JSON string to MQTT Topic
    if (client.publish(mqtt_topic, jsonString.c_str())) {
        Serial.println("MQTT publish succeeded");
    } else {
        Serial.println("MQTT publish failed");
        Serial.print("failed, rc=");
        Serial.println(client.state());
    }

    PoECAM.Camera.free();
}

void sendImageMQTT_RawJPEG()
{
    if (!PoECAM.Camera.get()) {
        Serial.println("Failed to capture image.");
        return;
    }

    size_t jpegSize = PoECAM.Camera.fb->len;
    uint8_t* jpegData = PoECAM.Camera.fb->buf;

    Serial.printf("JPEG size (bytes): %d\n", jpegSize);

    // Publish raw JPEG directly
    if (client.publish("cameras/cam1/image", jpegData, jpegSize)) {
        Serial.println("MQTT publish succeeded");
    } else {
        Serial.println("MQTT publish failed");
        Serial.print("failed, rc=");
        Serial.println(client.state());
    }

    PoECAM.Camera.free();
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("📥 Message arrived on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }