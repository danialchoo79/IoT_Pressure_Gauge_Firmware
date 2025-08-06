#ifndef MQTTBASE64_H
#define MQTTBASE64_H

#include <Wifi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "HTTPCamNonBlocking.h"
#include "base64.h"
#include "PingMQTTBroker.h"

extern IPAddress local_IPMQTT;  // Free IP Network
extern IPAddress gatewayMQTT;   // Router IP
extern IPAddress subnetMQTT;    // Standard Subnet
extern IPAddress dnsMQTT;       // Optional: Google DNS
extern IPAddress brokerIP;
extern IPAddress PrimarydnsMQTT;           // Google DNS or your router's DNS
extern IPAddress SecondarydnsMQTT;           // Google DNS or your router's DNS

// Wifi Details
extern const char* ssidmqttbase64;
extern const char* passwordmqttbase64;

// MQTT Broker info
extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_topic;

extern PubSubClient client;

void SetupWifiMQTTBase64();
void SetupCameraMQTTBase64();
void reconnect();
void sendImageMQTT();
void sendImageMQTT_RawJPEG();

#endif