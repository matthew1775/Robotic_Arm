// HardwareCommandState.h
#ifndef HARDWARE_COMMAND_STATE_H
#define HARDWARE_COMMAND_STATE_H

struct HardwareCommandState
{
    uint8_t S_W_L       : 1;
    uint8_t S_W_R       : 1;
    uint8_t S_INNE      : 1;
    uint8_t S_SCI       : 1;
    uint8_t S_ELEK      : 1;
    uint8_t S_RAM       : 1;
    uint8_t FAN1_ON     : 1;
    uint8_t NEOPIXEL    : 1;

    HardwareCommandState() :
    S_W_L(0), S_W_R(0), S_INNE(0), S_SCI(0),
    S_ELEK(0), S_RAM(0), FAN1_ON(0), NEOPIXEL(0)
    {}
};

#endif