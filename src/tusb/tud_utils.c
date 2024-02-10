#include "tud_utils.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "../kbd/kbd.h"

void usb_task( void* param ) {
    ( void ) param;
    gpio_init( LED_PIN );
    gpio_set_dir( LED_PIN, true );
    // board_init();
    tusb_init();
    // init device stack on configured roothub port
    // This should be called after scheduler/kernel is started.
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS
    // queue API.
    tud_init( BOARD_TUD_RHPORT );

    // RTOS forever loop
    while ( 1 ) {
        // put this thread to waiting state until there is new events
        tud_task();
    }
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

bool tud_hid_n_nkro_keyboard_report( uint8_t instance, uint8_t report_id,
                                     uint8_t keycode[KEYCODE_BUFFER] ) {

    hid_nkro_keyboard_report_t report = {
        .modifier = 0,
        .reserved = 0,
        .boot_keys = { 0 },
        .key_bitmap = { 0 },
    };

    uint8_t key_count = 0;
    uint8_t current_key = 0;

    for ( uint8_t key = 0; key < KEYCODE_BUFFER && keycode[key] > 0; key++ ) {
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
    if ( itf == ITF_INOUT ) {
        tud_hid_n_report( ITF_INOUT, 0, buffer, bufsize );
    }
}

// HID TASK
void hid_task( void* kbd ) {
    ( keyboard_t* ) kbd;
    uint8_t buffer[KEYCODE_BUFFER];

    for ( ;; ) {
        vTaskDelay( pdMS_TO_TICKS( POLLING_INTERVAL ) );

        if ( tud_suspended() ) {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            vTaskDelay( pdMS_TO_TICKS( 100 ) );
            if ( keyboard_update_status( kbd ) ) {
                tud_remote_wakeup();
            }
            continue;
        }

        if ( !tud_hid_n_ready( ITF_KBD ) ) continue;

        // Clear array buffer
        memset( buffer, 0, KEYCODE_BUFFER * sizeof( uint8_t ) );
        keyboard_update_buffer( kbd, buffer, KEYCODE_BUFFER );

        // Generate hid report
        tud_hid_n_nkro_keyboard_report( ITF_KBD, 0, buffer );
    }
}

TimerHandle_t blinky_tm;

void led_blinky_timer( void ) {
    blinky_tm = xTimerCreate( NULL, pdMS_TO_TICKS( BLINK_NOT_MOUNTED ), true,
                              NULL, led_blinky_cb );
    xTimerStart( blinky_tm, 0 );
}

void led_blinky_cb( TimerHandle_t xTimer ) {
    ( void ) xTimer;
    static bool led_state = false;

    // board_led_write(led_state);
    gpio_put( LED_PIN, led_state );
    led_state = 1 - led_state; // toggle
}

void tud_mount_cb( void ) {
    xTimerChangePeriod( blinky_tm, pdMS_TO_TICKS( BLINK_MOUNTED ), 0 );
}

// Invoked when device is unmounted
void tud_umount_cb( void ) {
    xTimerChangePeriod( blinky_tm, pdMS_TO_TICKS( BLINK_NOT_MOUNTED ), 0 );
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb( bool remote_wakeup_en ) {
    ( void ) remote_wakeup_en;
    xTimerChangePeriod( blinky_tm, pdMS_TO_TICKS( BLINK_SUSPENDED ), 0 );
}

// Invoked when usb bus is resumed
void tud_resume_cb( void ) {
    if ( tud_mounted() ) {
        xTimerChangePeriod( blinky_tm, pdMS_TO_TICKS( BLINK_MOUNTED ), 0 );
    } else {
        xTimerChangePeriod( blinky_tm, pdMS_TO_TICKS( BLINK_NOT_MOUNTED ), 0 );
    }
}