#ifndef NETWORK_H
#define NETWORK_H
#include "States.h"

namespace Network {
    void initNetwork(struct NetworkState &ns, struct HardwareCommandState &hcs);
    void handleNetwork();
    void sendFeedbackMessage(const HardwareFeedbackState &hfs);
    void sendErrorMessage(uint32_t errorDesc);
}

namespace NetworkHandlers {
    void controlCmdHandler(StaticJsonDocument<1024> &doc, NetworkState &ns, HardwareCommandState &hcs);
    void feedbackEncHandler(StaticJsonDocument<1024> &doc, NetworkState &ns, const HardwareFeedbackState &hfs);
    void errorEncHandler(StaticJsonDocument<1024> &doc, NetworkState &ns, uint32_t errorDesc);
}
#endif