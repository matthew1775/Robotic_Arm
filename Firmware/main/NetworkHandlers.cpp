#include <ArduinoJson.h>
#include "Network.h"
#include "States.h"
#include "Configs.h"

void NetworkHandlers::controlCmdHandler(StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc, 
                                        NetworkState &ns, 
                                        HardwareCommandState &hcs) {
    if (doc["payload"].is<JsonObject>()) {
        JsonObject p = doc["payload"];
        
        // Mapowanie pozycji
        if(p.containsKey("joint1_rad")) hcs.joint1_rad = p["joint1_rad"];
        if(p.containsKey("joint2_rad")) hcs.joint2_rad = p["joint2_rad"];
        if(p.containsKey("joint3_rad")) hcs.joint3_rad = p["joint3_rad"];
        if(p.containsKey("joint4_rad")) hcs.joint4_rad = p["joint4_rad"];
        if(p.containsKey("joint5_rad")) hcs.joint5_rad = p["joint5_rad"];
        if(p.containsKey("joint6_rad")) hcs.joint6_rad = p["joint6_rad"];
        
        // Mapowanie prędkości
        if(p.containsKey("joint1_speed")) hcs.joint1_speed = p["joint1_speed"];
        if(p.containsKey("joint_mini_1_rad")) hcs.joint_mini_1_rad = p["joint_mini_1_rad"];
        // ... (dodaj pozostałe osie analogicznie)
    }
}

void NetworkHandlers::feedbackEncHandler(StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc, 
                                         NetworkState &ns, 
                                         const HardwareFeedbackState &hfs) {
    doc.clear();
    doc["eventType"] = "Robotic_arm_feedback";
    JsonObject data = doc.createNestedObject("payload");
    data["temp"] = hfs.TEMP;
    data["bat1"] = hfs.BAT_1_ADC;
    
    char buffer[NetworkConfig::MAX_JSON_PAYLOAD];
    serializeJson(doc, buffer);
    ns.client.publish(NetworkConfig::TOPIC_ENC, buffer);
}

void NetworkHandlers::errorEncHandler(StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc, 
                                      NetworkState &ns, 
                                      uint32_t errorDesc) {
    doc.clear();
    doc["eventType"] = "error";
    doc["code"] = errorDesc;
    char buffer[128];
    serializeJson(doc, buffer);
    ns.client.publish(NetworkConfig::TOPIC_ENC, buffer);
}