#ifndef CONFIG_H
#define CONFIG_H

#include "stm32f1xx.h"

#define RADIO_MODE_TX
//#define RADIO_MODE_RX

#define RADIO_ADDR 0xA16D13
#define DATA_SIZE 1

typedef struct 
{
    const uint8_t * const length;
    uint8_t * const data;
} ControlData;

extern ControlData * const control_data;
#endif