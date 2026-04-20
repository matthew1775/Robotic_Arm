// HardwareConfig.h
#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <cstdint>

namespace HardwareConfig
{
    constexpr float     AMPM_VOLTS           = 3.3;
    constexpr float     AMPM_MV_PER_AMPERE   = 66; // 5A- 185, 20A- 100, 30A- 66
    constexpr uint16_t  AMPM_MAX_ADC         = 4095;

    constexpr uint32_t  BAUD_RATE            = 500E3; // 500kbps, change later 
    constexpr uint32_t  SERIAL_BAUD_RATE     = 115200;
}

#endif