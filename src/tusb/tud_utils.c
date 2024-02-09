#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board.h"
#include "pico/stdlib.h"
#include "tud_utils.h"
#include "tusb.h"

#include "../flash/flash.h"
#include "../kbd/kbd.h"

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

bool tud_hid_n_nkro_keyboard_report( uint8_t instance, uint8_t report_id,
                                     uint8_t keycode[keycode_buffer] ) {

    hid_nkro_keyboard_report_t report = {
        .modifier = 0,
        .reserved = 0,
        .boot_keys = { 0 },
        .key_bitmap = { 0 },
    };

    uint8_t key_count = 0;
    uint8_t current_key = 0;

    for ( uint8_t key = 0; key < keycode_buffer; key++ ) {
        current_key = keycode[key];

        // Modifier keys
        if ( current_key >= 0xE0 ) {
            report.modifier |= ( 1 << ( current_key - 0xE0 ) );
            continue;
        }

        // Boot keys support
        if ( key_count < 6 ) {
            report.boot_keys[key_count] = current_key;
        }

        // NKRO
        report.key_bitmap[current_key >> 3] |= 1 << ( current_key & 0x7 );
    }

    return tud_hid_n_report( instance, report_id, &report, sizeof( report ) );
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb( uint8_t itf, uint8_t report_id,
                                hid_report_type_t report_type, uint8_t* buffer,
                                uint16_t reqlen ) {
    // TODO not Implemented
    ( void ) itf;
    ( void ) report_id;
    ( void ) report_type;
    ( void ) buffer;
    ( void ) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb( uint8_t itf, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize ) {
    // TODO set LED based on CAPLOCK, NUMLOCK etc...
    //(void) itf;
    ( void ) report_id;
    ( void ) report_type;
    //(void) buffer;
    //(void) bufsize;
    if ( itf == 1 ) {
        tud_hid_n_report( 1, 0, buffer, bufsize );
    }
}