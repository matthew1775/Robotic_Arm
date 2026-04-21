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
    // Sprawdzamy czy dokument zawiera klucz "payload", który jest obiektem JSON
    if (doc["payload"].is<JsonObject>()) 
    {
        JsonObject payload = doc["payload"];

        // Pozycje główne
        if(payload["joint1_rad"].is<JsonVariant>()) hcs.joint1_rad = payload["joint1_rad"];
        if(payload["joint2_rad"].is<JsonVariant>()) hcs.joint2_rad = payload["joint2_rad"];
        if(payload["joint3_rad"].is<JsonVariant>()) hcs.joint3_rad = payload["joint3_rad"];
        if(payload["joint4_rad"].is<JsonVariant>()) hcs.joint4_rad = payload["joint4_rad"];
        if(payload["joint5_rad"].is<JsonVariant>()) hcs.joint5_rad = payload["joint5_rad"];
        if(payload["joint6_rad"].is<JsonVariant>()) hcs.joint6_rad = payload["joint6_rad"];
        if(payload["joint7_rad"].is<JsonVariant>()) hcs.joint7_rad = payload["joint7_rad"];
        if(payload["joint8_rad"].is<JsonVariant>()) hcs.joint8_rad = payload["joint8_rad"];

        // Pozycje MINI
        if(payload["joint_mini_1_rad"].is<JsonVariant>()) hcs.joint_mini_1_rad = payload["joint_mini_1_rad"];
        if(payload["joint_mini_2_rad"].is<JsonVariant>()) hcs.joint_mini_2_rad = payload["joint_mini_2_rad"];
        if(payload["joint_mini_3_rad"].is<JsonVariant>()) hcs.joint_mini_3_rad = payload["joint_mini_3_rad"];
        if(payload["joint_mini_4_rad"].is<JsonVariant>()) hcs.joint_mini_4_rad = payload["joint_mini_4_rad"];
        if(payload["joint_mini_5_rad"].is<JsonVariant>()) hcs.joint_mini_5_rad = payload["joint_mini_5_rad"];

        // Prędkości główne
        if(payload["joint1_speed"].is<JsonVariant>()) hcs.joint1_speed = payload["joint1_speed"];
        if(payload["joint2_speed"].is<JsonVariant>()) hcs.joint2_speed = payload["joint2_speed"];
        if(payload["joint3_speed"].is<JsonVariant>()) hcs.joint3_speed = payload["joint3_speed"];
        if(payload["joint4_speed"].is<JsonVariant>()) hcs.joint4_speed = payload["joint4_speed"];
        if(payload["joint5_speed"].is<JsonVariant>()) hcs.joint5_speed = payload["joint5_speed"];
        if(payload["joint6_speed"].is<JsonVariant>()) hcs.joint6_speed = payload["joint6_speed"];
        if(payload["joint7_speed"].is<JsonVariant>()) hcs.joint7_speed = payload["joint7_speed"];
        if(payload["joint8_speed"].is<JsonVariant>()) hcs.joint8_speed = payload["joint8_speed"];

        // Prędkości MINI
        if(payload["joint_mini_1_speed"].is<JsonVariant>()) hcs.joint_mini_1_speed = payload["joint_mini_1_speed"];
        if(payload["joint_mini_2_speed"].is<JsonVariant>()) hcs.joint_mini_2_speed = payload["joint_mini_2_speed"];
        if(payload["joint_mini_3_speed"].is<JsonVariant>()) hcs.joint_mini_3_speed = payload["joint_mini_3_speed"];
        if(payload["joint_mini_4_speed"].is<JsonVariant>()) hcs.joint_mini_4_speed = payload["joint_mini_4_speed"];
        if(payload["joint_mini_5_speed"].is<JsonVariant>()) hcs.joint_mini_5_speed = payload["joint_mini_5_speed"];
    }
}

// Pozostałe, nieużywane w tym pytaniu metody:
void NetworkHandlers::feedbackEncHandler(...) {}
void NetworkHandlers::errorEncHandler(...) {}