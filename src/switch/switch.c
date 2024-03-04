#include "switch.h"

switch_t new_switch(uint8_t pin){
    switch_t sw = {
        .pin = pin,
        .debounce = 0,
        .wait = 0,
        .status = SW_NOT_PRESSED,
    };
    return sw;
}

bool sw_is_pressed( switch_t* sw ){
    return !gpio_get( sw->pin );
}

void sw_debounce(switch_t* sw){    
    sw->debounce <<= 1;
    sw->debounce |= sw_is_pressed(sw);
    sw->wait >>= 1;
}

SW_STATUS sw_get_status( switch_t* sw ){
    if ( __builtin_popcount( (uint32_t) (sw->debounce) ) >= 7 ) {
        sw->wait = SW_DEBOUCE_TIME;
    }
    if ( sw->wait ) {
        sw->status = SW_PRESSED;
    } else {
        sw->status = SW_NOT_PRESSED;
    }
    
    return sw->status;
};