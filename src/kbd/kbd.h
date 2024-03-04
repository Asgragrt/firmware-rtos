#ifndef _KBD_H_
#define _KBD_H_

#include "../keyboard_config.h"
#include "../switch/switch.h"
#include "pico/stdlib.h"

#define PIN_COUNT     9

#define DEBOUNCE_TIME 0x1F // 5 ms

#define KN                                                                     \
    { K0, K1, K2, K3, K6, K7, K8, K4, K5 }

#define kc_s( ... ) __VA_ARGS__, 0

// Max 4 keys, else unexpected behavior
// Adding a 0 at the end to mark key binding endings
#define K0_keys     kc_s( HID_KEY_S )
#define K1_keys     kc_s( HID_KEY_D )
#define K2_keys     kc_s( HID_KEY_F )
#define K3_keys     kc_s( HID_KEY_SPACE )
#define K4_keys     kc_s( HID_KEY_CONTROL_LEFT, HID_KEY_O )
#define K5_keys     kc_s( HID_KEY_F1 )
#define K6_keys     kc_s( HID_KEY_J )
#define K7_keys     kc_s( HID_KEY_K )
#define K8_keys     kc_s( HID_KEY_L )
#define KN_keys                                                                \
    {                                                                          \
        K0_keys, K1_keys, K2_keys, K3_keys, K6_keys, K7_keys, K8_keys,         \
            K4_keys, K5_keys                                                   \
    }

typedef struct {
    switch_t sw;
    uint8_t key_count;
    uint8_t keys[4];
} keyboard_switch_t;

typedef struct {
    keyboard_switch_t pins[PIN_COUNT];
    uint16_t status;
} keyboard_t;

keyboard_t* keyboard_new( void );

void keyboard_init( keyboard_t* kbd );

bool kbd_is_pressed( keyboard_t* kbd, uint8_t idx );

bool keyboard_update_status( keyboard_t* kbd );

bool keyboard_update_buffer( keyboard_t* kbd, uint8_t* buffer,
                             uint8_t buflength );

void keyboard_debounce( keyboard_t* kbd );

#endif /* _KBD_H_ */