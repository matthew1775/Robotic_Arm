// Network.h
#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include "States/NetworkState.h"
#include "States/HardwareCommandState.h"
#include "States/HardwareFeedbackState.h"

namespace Network
{
    void initNetwork(struct NetworkState &ns, struct HardwareCommandState &hcs);
    void handleNetwork(); // Wywoływane w loop()
    void sendFeedbackMessage(const HardwareFeedbackState &hs);
    void sendErrorMessage(uint32_t errorDesc);
}

#endif