// Network.cpp
#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>

#include "Pins.h"
#include "Network.h"
#include "NetworkHandlers.h"

#include "Configs/NetworkConfig.h"
#include "States/HardwareFeedbackState.h"
#include "States/HardwareCommandState.h"
#include "States/NetworkState.h"

namespace
{
	// Non-const local copies, needed because of Ethernet's begin limitations.
	static uint8_t MAC_[6]; 
	static uint8_t IP_[4];
	
	/*
	  Local pointers to NetworkState and HardwareCommandState structs.

	  Initialized in initNetwork() method.
	  Needed because callback function cannot accept any additional arguments apart from the ones dictated by PubSubClient. 
	*/
	static struct NetworkState *ns_ = nullptr;
	static struct HardwareCommandState *hcs_ = nullptr;
}

void callback(const char* topic, const byte* payload, uint16_t length)
{
  if(ns_ == nullptr)
  {
    Serial.print("[MQTT] Local copy of NetworkState is not initialized in Network.cpp file! Call the initNetwork() before using other methods. Stopping.");
    return;
  }
  
  if(hcs_ == nullptr)
  {
    Serial.print("[MQTT] Local copy of HardwareCommandState is not initialized in Network.cpp file! Call the initNetwork() before using other methods. Stopping.");
    return;
  }
  
  if (length > NetworkConfig::MAX_JSON_PAYLOAD)
  {
    Serial.println("[MQTT] ERROR: Payload is too big!");
    return; 
  }
  
  if(strcmp(topic, NetworkConfig::TOPIC_CMD) != 0) return;

  StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if(error)
  {
    Serial.print("[MQTT] Failed to deserialize the JSON!"); Serial.println(error.c_str());
    return;
  }
  
  ns_->lastMqttCmdTime = millis();

  NetworkHandlers::controlCmdHandler(doc, *ns_, *hcs_);
}

void Network::initNetwork(struct NetworkState &ns, struct HardwareCommandState &hcs)
{
  ns_ = &ns;
  hcs_ = &hcs;
	memcpy(MAC_, NetworkConfig::MAC, 6);
	memcpy(IP_, NetworkConfig::IP, 4);
  
  Serial.println("[ETHERNET] Init Ethernet...");
  digitalWrite(Pins::WIZ_RST_PIN, LOW); delay(100);
  digitalWrite(Pins::WIZ_RST_PIN, HIGH); delay(100);
  
  Ethernet.init(Pins::WIZ_SCN_PIN);
  Ethernet.begin(MAC_, IP_);
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("[ETHERNET] ERROR: W5500 not found!");
  }
  else
  {
    Serial.print("[ETHERNET] Ethernet IP: "); Serial.println(Ethernet.localIP());
  }
  
  ns.client.setServer(NetworkConfig::MQTT_SERVER_IP, NetworkConfig::MQTT_PORT);
  ns.client.setCallback(callback);
  ns.client.setBufferSize(NetworkConfig::MAX_JSON_PAYLOAD);
}

static void reconnect()
{
  if(ns_ == nullptr)
  {
    Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
    return;
  }

  if(ns_->client.connected()) return;

  static uint32_t lastRec = 0;

  if(millis() - lastRec < NetworkConfig::RECONNECTION_TIMEOUT) return;

  lastRec = millis();
  Serial.print("[MQTT] Connecting to "); Serial.print(NetworkConfig::MQTT_SERVER_IP); Serial.println("...");

  if(!ns_->client.connect(NetworkConfig::MQTT_SERVER_ID))
  {
    Serial.print("[MQTT] Failed, rc="); Serial.println(ns_->client.state());
    return;
  }

  Serial.println("[MQTT] Connected!");
  ns_->client.subscribe(NetworkConfig::TOPIC_CMD);
}

void Network::handleNetwork()
{
  if(ns_ == nullptr)
  {
    Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
    return;
  }

  if(millis() - ns_->lastMqttCmdTime > NetworkConfig::MQTT_SAFETY_TIMEOUT)
  {
    Serial.println("[MQTT] ERROR: Safety Timeout!");
    // ... Take action ...
  }

  reconnect();
  ns_->client.loop();
}

void Network::sendFeedbackMessage(const HardwareFeedbackState &hfs)
{
	if(ns_ == nullptr)
	{
		Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
		return;
	}

	if(!ns_->client.connected())
	{
		Serial.println("[MQTT] Failed to send message; NetworkState::client is not connected!");
		return;
	}

	StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> doc;

	NetworkHandlers::feedbackEncHandler(doc, *ns_, hfs);
}

void Network::sendErrorMessage(const uint32_t errorDesc)
{
	if(ns_ == nullptr)
	{
		Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
		return;
	}

	if(!ns_->client.connected())
	{
		Serial.println("[MQTT] Failed to send message; NetworkState's client is not connected!");
		return;
	}

	StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> doc;

	NetworkHandlers::errorEncHandler(doc, *ns_, errorDesc);
}