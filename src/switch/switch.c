#include "switch.h"

switch_t new_switch(uint8_t pin){
    switch_t sw = {
        .pin = pin,
        .debounce = 0,
        .status = SW_NOT_PRESSED,
    };
    return sw;
}

bool sw_is_pressed( switch_t* sw ){
    return !gpio_get( sw->pin );
}

void sw_debounce(switch_t* sw){
    // Here goes the debounce logic
    /*
    if (sw_is_pressed(sw)){
        sw->debounce = SW_DEBOUCE_TIME;
        sw->status = SW_PRESSED;
        return;
    } else if (sw->debounce) {
        sw->debounce >>= 1;
    } else {
        sw->status = SW_NOT_PRESSED;
    }
    */
    
    sw->debounce <<= 1;
    sw->debounce |= sw_is_pressed(sw);
}

SW_STATUS sw_get_status( switch_t* sw ){
    
    if ( __builtin_popcount( (uint32_t) (sw->debounce) ) >= 7 ) {
        sw->status = SW_PRESSED;
    } else {
        sw->status = SW_NOT_PRESSED;
    }
    
    return sw->status;
};