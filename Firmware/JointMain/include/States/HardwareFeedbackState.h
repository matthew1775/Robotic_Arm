// HardwareFeedbackState.h
#ifndef HARDWARE_FEEDBACK_STATE_H
#define HARDWARE_FEEDBACK_STATE_H

#include <cstdint>

struct HardwareFeedbackState
{
    uint16_t BAT_1_ADC;
    uint16_t BAT_2_ADC;
    uint16_t BAT_3_ADC;
    uint16_t BAT_4_ADC;
    uint16_t ADC_WL;
    uint16_t ADC_WR;
    uint16_t ADC_RAM;
    uint16_t ADC_ELEK;
    uint16_t ADC_SCI;
    uint16_t ADC_INNE;
    float    TEMP;

    HardwareFeedbackState() :
    BAT_1_ADC(0), BAT_2_ADC(0), BAT_3_ADC(0), BAT_4_ADC(0),
    ADC_WL(0), ADC_WR(0), ADC_RAM(0), ADC_ELEK(0), ADC_SCI(0),
    ADC_INNE(0), TEMP(0.0F) {}
};

#endif