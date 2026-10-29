#include "config.h"

static uint8_t s_data[DATA_SIZE];
static uint8_t s_length = DATA_SIZE;

ControlData * const control_data = &(ControlData){
    .data = s_data,
    .length = &s_length
};
