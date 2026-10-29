#ifndef NRF24_H
#define NRF24_H

#include "stm32f1xx.h"
#include "config.h"

void NRF24_Init (void);
uint8_t NRF24_ProcessPacket (ControlData* control_data);

#endif