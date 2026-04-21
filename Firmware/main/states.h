#ifndef STATES_H
#define STATES_H

#include <Arduino.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ACS712.h>
#include <Adafruit_NeoPixel.h>
#include <L298NX2.h>

#include "Pins.h"
#include "Configs.h"

// Stan komend MQTT dla ramienia
struct HardwareCommandState {
    float joint1_rad, joint2_rad, joint3_rad, joint4_rad;
    float joint5_rad, joint6_rad, joint7_rad, joint8_rad;
    float joint_mini_1_rad, joint_mini_2_rad, joint_mini_3_rad;
    float joint_mini_4_rad, joint_mini_5_rad;

    float joint1_speed, joint2_speed, joint3_speed, joint4_speed;
    float joint5_speed, joint6_speed, joint7_speed, joint8_speed;
    float joint_mini_1_speed, joint_mini_2_speed, joint_mini_3_speed;
    float joint_mini_4_speed, joint_mini_5_speed;

    HardwareCommandState() {
        joint1_rad = joint2_rad = joint3_rad = joint4_rad = 0.0f;
        joint5_rad = joint6_rad = joint7_rad = joint8_rad = 0.0f;
        joint_mini_1_rad = joint_mini_2_rad = joint_mini_3_rad = 0.0f;
        joint_mini_4_rad = joint_mini_5_rad = 0.0f;
        joint1_speed = joint2_speed = joint3_speed = joint4_speed = 0.0f;
        joint5_speed = joint6_speed = joint7_speed = joint8_speed = 0.0f;
        joint_mini_1_speed = joint_mini_2_speed = joint_mini_3_speed = 0.0f;
        joint_mini_4_speed = joint_mini_5_speed = 0.0f;
    }
};

// Stan sensorów i diagnostyki
struct HardwareFeedbackState {
    uint16_t BAT_1_ADC, BAT_2_ADC, BAT_3_ADC, BAT_4_ADC;
    uint16_t ADC_WL, ADC_WR, ADC_RAM, ADC_ELEK, ADC_SCI, ADC_INNE;
    float TEMP;

    HardwareFeedbackState() : 
        BAT_1_ADC(0), BAT_2_ADC(0), BAT_3_ADC(0), BAT_4_ADC(0),
        ADC_WL(0), ADC_WR(0), ADC_RAM(0), ADC_ELEK(0), ADC_SCI(0),
        ADC_INNE(0), TEMP(0.0f) {}
};

// Stan komponentów sprzętowych
struct HardwareState {
    L298NX2 twinMotorDriver;
    ACS712 lcSensor;
    Adafruit_NeoPixel NeoPixelConnector;

    HardwareState() :
        twinMotorDriver(Pins::ENABLE_PIN, Pins::IN_POS_PIN, Pins::IN_NEG_PIN,
                        Pins::ENABLE_PIN, Pins::IN_POS_PIN, Pins::IN_NEG_PIN),
        lcSensor(Pins::ADC_PIN, HardwareConfig::AMPM_VOLTS,
                 HardwareConfig::AMPM_MAX_ADC, HardwareConfig::AMPM_MV_PER_AMPERE) {
        NeoPixelConnector.setPin(Pins::NEOPIXEL_PIN);
    }
};

// Stan połączenia sieciowego
struct NetworkState {
    EthernetClient ethClient;
    PubSubClient client;
    uint32_t lastMqttCmdTime = 0;

    NetworkState() : client(ethClient) {}
};

#endif