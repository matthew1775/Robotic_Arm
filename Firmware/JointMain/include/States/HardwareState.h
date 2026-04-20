// HardwareState.h
#ifndef HARDWARE_STATE_H
#define HARDWARE_STATE_H

#include "Pins.h"
#include "Configs/HardwareConfig.h"


// ---
#include <ACS712.h>
#include <Adafruit_NeoPixel.h>
#include <L298NX2.h>
// ---

struct HardwareState
{
    L298NX2 twinMotorDriver;
    ACS712  lcSensor;
    Adafruit_NeoPixel NeoPixelConnector;

    HardwareState() :
    twinMotorDriver(
        Pins::ENABLE_PIN, Pins::IN_POS_PIN, Pins::IN_NEG_PIN,
        Pins::ENABLE_PIN, Pins::IN_POS_PIN, Pins::IN_NEG_PIN
    ),
    lcSensor(
        Pins::ADC_PIN, HardwareConfig::AMPM_VOLTS,
        HardwareConfig::AMPM_MAX_ADC, HardwareConfig::AMPM_MV_PER_AMPERE
    )
    {
        NeoPixelConnector.setPin(Pins::NEOPIXEL_PIN);
    }
};

#endif