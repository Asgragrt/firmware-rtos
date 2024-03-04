#ifndef _SWITCH_H_
#define _SWITCH_H_

#include "FreeRTOS.h"
#include "pico/stdlib.h"

#define DEBOUCE_SAMPLE_TICKS pdMS_TO_TICKS( 1 ) / 4
#define SW_DEBOUCE_TIME      0xFFFFF;

typedef enum {
    SW_NOT_PRESSED,
    SW_PRESSED,
} SW_STATUS;

typedef struct {
    uint8_t pin;
    uint8_t debounce;
    uint32_t wait;
    SW_STATUS status;
} switch_t;

switch_t new_switch( uint8_t pin );

void sw_debounce( switch_t* sw );

SW_STATUS sw_get_status( switch_t* sw );

#endif /* _SWITCH_H_ */