#ifndef CONFIGS_H
#define CONFIGS_H

namespace HardwareConfig {
    constexpr float AMPM_VOLTS = 3.3;
    constexpr float AMPM_MV_PER_AMPERE = 66;
    constexpr uint16_t AMPM_MAX_ADC = 4095;
    constexpr uint32_t SERIAL_BAUD_RATE = 115200;
}

namespace NetworkConfig {
    constexpr const char* const MQTT_SERVER_IP = "192.168.1.1";
    constexpr const char* const MQTT_SERVER_ID = "";
    constexpr uint8_t MAC[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE3 };
    constexpr uint8_t IP[4] = { 192, 168, 1, 53 };
    constexpr uint16_t MQTT_PORT = 1883;
    constexpr uint16_t MQTT_FEEDBACK_DELAY = 100;
    constexpr const char* const TOPIC_ENC = "Robotic_arm_enc";
    constexpr const char* const TOPIC_CMD = "Robotic_arm_cmd";
    constexpr uint16_t MQTT_SAFETY_TIMEOUT = 1000;
    constexpr uint16_t RECONNECTION_TIMEOUT = 5000;
    constexpr uint16_t MAX_JSON_PAYLOAD = 1024;
}
#endif