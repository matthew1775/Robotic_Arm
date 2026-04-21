// NetworkHandlers.h
#ifndef NETWORK_HANDLERS_H
#define NETWORK_HANDLERS_H

#include <ArduinoJson.h>
#include "Configs/NetworkConfig.h"

#include "States/NetworkState.h"
#include "States/HardwareCommandState.h"
#include "States/HardwareFeedbackState.h"

namespace NetworkHandlers
{
    void controlCmdHandler(
        StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc,
        struct NetworkState &ns,
        struct HardwareCommandState &hcs
    );

    void feedbackEncHandler(
        StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc,
        struct NetworkState &ns,
        const struct HardwareFeedbackState &hfs
    );
    
    void errorEncHandler(
        StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc,
        struct NetworkState &ns,
        const uint32_t errorDesc
    );
}

#endif