#include "PingMQTTBroker.h"

void PingMQTTBroker(IPAddress brokerIPPing)
{
    // Ping the MQTT Broker
    Serial.print("Pinging broker at: ");
    Serial.println(brokerIPPing);

    if(Ping.ping(brokerIPPing,5)) { // Ping 5 times
        Serial.println("Ping successful");
    } else {
        Serial.println("Ping failed");
    }
}