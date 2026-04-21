#include "Pins.h"
#include "Configs.h"
#include "States.h"
#include "Network.h"
#include "Hardware.h"

HardwareFeedbackState hfs;
HardwareCommandState hcs;
NetworkState ns;
HardwareState hs;

void setup() {
    Serial.begin(HardwareConfig::SERIAL_BAUD_RATE);
    delay(1000);
    Serial.println("\n\n>>> ROBOTIC ARM MODULE SYSTEM START <<<");

    Pins::init_pins();
    Network::initNetwork(ns, hcs);
    Hardware::initHardware();

    Serial.println(">>> SETUP COMPLETE <<<");
}

void loop() {
    uint32_t now = millis();

    if(now - ns.lastMqttCmdTime > NetworkConfig::MQTT_SAFETY_TIMEOUT) {
        Serial.println("[WATCHDOG] !!! Connection loss - STOP !!!");
    }

    Network::handleNetwork();

    static uint32_t lastMqttFeedback = 0;
    if(now - lastMqttFeedback >= NetworkConfig::MQTT_FEEDBACK_DELAY) {
        lastMqttFeedback = now;
        Network::sendFeedbackMessage(hfs);
    }
}