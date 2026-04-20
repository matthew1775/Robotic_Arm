// NetworkHandlers.cpp
#include <Arduino.h>
#include <ArduinoJson.h>

#include "NetworkHandlers.h"

#include "States/NetworkState.h"
#include "States/HardwareCommandState.h"
#include "States/HardwareFeedbackState.h"

#include "Configs/NetworkConfig.h"


void NetworkHandlers::controlCmdHandler(StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc,
    struct NetworkState &ns,
    struct HardwareCommandState &hcs)
{  
}

void NetworkHandlers::feedbackEncHandler(
    StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc,
    struct NetworkState &ns,
    const struct HardwareFeedbackState &hfs
)
{
}

void NetworkHandlers::errorEncHandler(
        StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc,
        struct NetworkState &ns,
        const uint32_t errorDesc
    )
{
    doc.clear();
    doc["eventType"] = NetworkConfig::TOPIC_ENC;
    char errStr[12];
    snprintf(errStr, sizeof(errStr), "0x%X", errorDesc);
    doc["error"] = errStr;

    char buffer[NetworkConfig::MAX_JSON_PAYLOAD];
    serializeJson(doc, buffer);

    if(!ns.client.publish(NetworkConfig::TOPIC_ENC, buffer))
    {
        Serial.println("[MQTT] Error publish failure!");
    }
}