#include "pico/stdlib.h"
#include "tusb.h"

#include "../switch/switch.h"
#include "flash/flash.h"
#include "kbd.h"
#include <stdlib.h>

keyboard_switch_t new_kbdsw( uint8_t pin, uint8_t* current_idx, uint8_t* keys,
                             uint8_t keys_size ) {
    keyboard_switch_t ks = {
        .sw = new_switch( pin ),
        .key_count = 0,
        .keys = { 0 },
    };
    for ( uint8_t j = *current_idx; j < keys_size; j++ ) {
        if ( keys[j] == 0 ) {
            ks.key_count = j - *current_idx;
            *current_idx = j + 1;
            break;
        }

        if ( j - *current_idx > 3 ) break;

        ks.keys[j - *current_idx] = keys[j];
    }

    return ks;
}

keyboard_t* keyboard_new( void ) {
    uint8_t pins[PIN_COUNT] = KN;

    uint8_t keys[] = KN_keys;
    uint8_t keys_size = sizeof( keys ) / sizeof( uint8_t );
    uint8_t current_idx = 0;

    keyboard_t* kbd = ( keyboard_t* ) malloc( sizeof( keyboard_t ) );

    if ( kbd == NULL ) return kbd;

    // Assigning values of kb keys
    for ( uint8_t i = 0; i < PIN_COUNT; i++ ) {
        kbd->pins[i] = new_kbdsw( pins[i], &current_idx, keys, keys_size );
    }

    return kbd;
}

uint8_t kbd_get_pin( keyboard_t* kbd, uint8_t idx ) {
    return kbd->pins[idx].sw.pin;
}

uint8_t* kbd_get_keys( keyboard_t* kbd, uint8_t idx ) {
    return kbd->pins[idx].keys;
}

uint8_t kbd_get_key_count( keyboard_t* kbd, uint8_t idx ) {
    return kbd->pins[idx].key_count;
}

bool kbd_is_pressed( keyboard_t* kbd, uint8_t idx ) {
    return !gpio_get( kbd_get_pin( kbd, idx ) );
}

uint8_t kbd_is_waiting( keyboard_t* kbd, uint8_t idx ) {
    kbd->pins[idx].sw.debounce >>= 1;
    return kbd->pins[idx].sw.debounce;
}

void kbd_set_waiting( keyboard_t* kbd, uint8_t idx ) {
    kbd->pins[idx].sw.debounce = DEBOUNCE_TIME;
}

// Init the key pins
void keyboard_init( keyboard_t* kbd ) {
    if ( kbd == NULL ) return;
    uint8_t pin;
    for ( uint8_t i = 0; i < PIN_COUNT; i++ ) {
        pin = kbd_get_pin( kbd, i );
        gpio_init( pin );
        gpio_set_dir( pin, GPIO_IN );
        gpio_pull_up( pin );
    }
}

// Update status
bool keyboard_update_status( keyboard_t* kbd ) {
    if ( kbd == NULL ) return false;
    // uint16_t status = kbd->status;
    uint16_t new_status = 0;
    for ( uint8_t i = 0; i < PIN_COUNT; i++ ) {
        if ( !gpio_get( kbd_get_pin( kbd, i ) ) ) {
            new_status |= 1 << i;
        }
    }

    kbd->status = new_status;
    // return status != new_status;
    return new_status;
}

switch_t* kbd_get_sw( keyboard_t* kbd, uint8_t i ) { return &kbd->pins[i].sw; }

bool keyboard_update_buffer( keyboard_t* kbd, uint8_t* buffer,
                             uint8_t buff_length ) {
    if ( kbd == NULL ) return false;
    uint16_t status = kbd->status;
    uint16_t new_status = 0;

    uint8_t offset = 0;
    for ( uint8_t i = 0; i < PIN_COUNT; i++ ) {
        if ( sw_get_status( kbd_get_sw( kbd, i ) ) != SW_PRESSED ) continue;

        memcpy( buffer + offset, kbd_get_keys( kbd, i ),
                kbd_get_key_count( kbd, i ) );
        new_status |= 1 << i;
        offset += kbd_get_key_count( kbd, i );
    }

    kbd->status = new_status;
    return status != new_status;
}

void keyboard_debounce( keyboard_t* kbd ) {
    if ( kbd == NULL ) return;
    for ( uint8_t i = 0; i < PIN_COUNT; i++ ) {
        sw_debounce( kbd_get_sw( kbd, i ) );
    }
}