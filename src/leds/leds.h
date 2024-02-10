#ifndef _LEDS_H_
#define _LEDS_H_

#include "../keyboard_config.h"
#include "pico/stdlib.h"

#define LOW  0
#define HIGH 45000

enum {
    _simple_wave = 0,
    _breathing,
    _on_off,
    _double_wave,
    _fixed_on,
    _led_off,
    E_MODE_COUNT,
};

#define MODE_COUNT E_MODE_COUNT
#define SPEED      400
#define INIT_DUTY  1

typedef struct {
    uint8_t pin;
    uint16_t duty;
} led_t;

typedef struct {
    led_t leds[LED_COUNT];
    uint8_t mode;
    bool increasing;
    uint16_t time_counter;
    uint16_t led_counter;
    bool duty_assigned;
} led_array_t;

led_array_t led_array_init( void );

void led_array_set_levels( led_array_t* led_array );

void led_array_update_mode( led_array_t* led_array, uint8_t mode );

void led_array_update_values( led_array_t* led_array, uint64_t* delay_value );

#endif /* _LEDS_H_ */