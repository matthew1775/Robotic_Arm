#include <Arduino.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "Network.h"
#include "States.h"
#include "Configs.h"

namespace {
    static NetworkState *ns_ = nullptr;
    static HardwareCommandState *hcs_ = nullptr;
}

// Funkcja zwrotna dla przychodzących wiadomości MQTT
void callback(const char* topic, const byte* payload, uint16_t length) {
    if (ns_ == nullptr || hcs_ == nullptr) return;
    if (length > NetworkConfig::MAX_JSON_PAYLOAD) return;
    if (strcmp(topic, NetworkConfig::TOPIC_CMD) != 0) return;

    StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
        Serial.print("[MQTT] JSON Error: "); Serial.println(error.c_str());
        return;
    }

    ns_->lastMqttCmdTime = millis();
    NetworkHandlers::controlCmdHandler(doc, *ns_, *hcs_);
}

void Network::initNetwork(NetworkState &ns, HardwareCommandState &hcs) {
    ns_ = &ns;
    hcs_ = &hcs;
    
    digitalWrite(Pins::WIZ_RST_PIN, LOW); delay(100);
    digitalWrite(Pins::WIZ_RST_PIN, HIGH); delay(100);
    
    Ethernet.init(Pins::WIZ_SCN_PIN);
    Ethernet.begin((uint8_t*)NetworkConfig::MAC, IPAddress(NetworkConfig::IP[0], NetworkConfig::IP[1], NetworkConfig::IP[2], NetworkConfig::IP[3]));
    
    ns.client.setServer(NetworkConfig::MQTT_SERVER_IP, NetworkConfig::MQTT_PORT);
    ns.client.setCallback(callback);
    ns.client.setBufferSize(NetworkConfig::MAX_JSON_PAYLOAD);
}

void reconnect() {
    if (ns_->client.connected()) return;
    static uint32_t lastRec = 0;
    if (millis() - lastRec < NetworkConfig::RECONNECTION_TIMEOUT) return;

    lastRec = millis();
    if (ns_->client.connect(NetworkConfig::MQTT_SERVER_ID)) {
        ns_->client.subscribe(NetworkConfig::TOPIC_CMD);
        Serial.println("[MQTT] Connected");
    }
}

void Network::handleNetwork() {
    if (ns_ == nullptr) return;
    reconnect();
    ns_->client.loop();
}

void Network::sendFeedbackMessage(const HardwareFeedbackState &hfs) {
    if (ns_ == nullptr || !ns_->client.connected()) return;
    StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> doc;
    NetworkHandlers::feedbackEncHandler(doc, *ns_, hfs);
}