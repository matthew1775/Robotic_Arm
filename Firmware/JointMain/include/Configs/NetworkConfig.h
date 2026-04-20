// NetworkConfig.h
#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <cstdint>

namespace NetworkConfig
{
    // --- Network adresses ---
    constexpr const char* const MQTT_SERVER_IP    = "192.168.1.1"; 
    constexpr const char* const MQTT_SERVER_ID    = ""; // !!!requires clarification!!!
    constexpr uint8_t MAC[6]   = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE3 }; // non-const because of Ethernet.begin type limitations
    constexpr uint8_t IP[4]    = { 192, 168, 1, 53 };
    // MAC_ and IP_ adresses, despite being quite static, are defined in Network.cpp as inline static variables.
    // It has to be this way due to Ethernet.begin limitations.

    // --- MQTT configuration ---
    constexpr uint16_t MQTT_PORT            = 1883;
    constexpr uint16_t MQTT_FEEDBACK_DELAY  = 100;
    
    // --- MQTT Topics ---
    // Only two needed by design (_enc topic for feedback, _cmd topic for command), so these names are totally ok.
    constexpr const char* const TOPIC_ENC         = "Robotic_arm_enc";
    constexpr const char* const TOPIC_CMD         = "Robotic_arm_cmd";

    // --- Network Safety ---
    constexpr uint16_t MQTT_SAFETY_TIMEOUT  = 1000;
    constexpr uint16_t RECONNECTION_TIMEOUT = 5000;
    constexpr uint16_t MAX_JSON_PAYLOAD     = 256;
}

#endif