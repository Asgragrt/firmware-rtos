// Pico libraries
#include "pico/bootrom.h"
#include "pico/flash.h"
#include "pico/stdlib.h"

// Tiny USB
#include "tusb.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "flash/flash.h"
#include "kbd/kbd.h"
#include "leds/leds.h"
#include "tusb/tud_utils.h"

// TODO clean main file
// TODO app that handles the data transfer

#define FLA_STACK_SIZE configMINIMAL_STACK_SIZE * 2
#define USB_STACK_SIZE                                                         \
    ( 3 * configMINIMAL_STACK_SIZE / 2 ) * ( CFG_TUSB_DEBUG ? 2 : 1 )
#define HID_STACK_SIZE configMINIMAL_STACK_SIZE
#define CDC_STACK_SIZE configMINIMAL_STACK_SIZE
#define LED_STACK_SIZE configMINIMAL_STACK_SIZE

static void fla_task( void* param );
static void led_task( void* leds );
static void kbd_task( void* kbd );

keyboard_t* kbd;
led_array_t led_array;
uint8_t state = 0;

SemaphoreHandle_t writeFlash, updateMode;

void write_to_flash( void* param );

void boot_if_pressed( keyboard_t* kbd ) {
    if ( !kbd_is_pressed( kbd, 7 ) || !kbd_is_pressed( kbd, 8 ) ) return;

    reset_usb_boot( 0, 0 );
}

void main() {
    kbd = keyboard_new();
    keyboard_init( kbd );

    boot_if_pressed( kbd );

    writeFlash = xSemaphoreCreateBinary();
    updateMode = xSemaphoreCreateBinary();

    TaskHandle_t handleUSB, handleHID, handleFLA, handleCDC, handleLED;
    // xTaskCreate(fla_task, "fla", FLA_STACK_SIZE,       NULL,
    // configMAX_PRIORITIES - 10, &handleFLA);
    xTaskCreate( kbd_task, "kbd", USB_STACK_SIZE, kbd, configMAX_PRIORITIES - 1,
                 NULL );
    xTaskCreate( usb_task, "usb", USB_STACK_SIZE, NULL,
                 configMAX_PRIORITIES - 2, &handleUSB );
    xTaskCreate( hid_task, "hid", HID_STACK_SIZE, kbd, configMAX_PRIORITIES - 2,
                 &handleHID );
    xTaskCreate( led_task, "led", LED_STACK_SIZE, &led_array,
                 configMAX_PRIORITIES - 3, &handleLED );

    vTaskCoreAffinitySet( handleUSB, ( 1 << 0 ) );
    vTaskCoreAffinitySet( handleHID, ( 1 << 1 ) );

    led_blinky_timer();

    // Start the scheduler
    vTaskStartScheduler();
}

void kbd_task( void* kbd ) {
    ( keyboard_t* ) kbd;
    for ( ;; ) {
        keyboard_debounce( kbd );
        vTaskDelay( DEBOUCE_SAMPLE_TICKS );
    }
}

void write_to_flash( void* param ) {
    flash_range_erase( FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE );
    flash_range_program( FLASH_TARGET_OFFSET, ( uint8_t* ) param,
                         FLASH_PAGE_SIZE );
}

void fla_task( void* param ) {
    uint8_t buffer[256] = { 0 };
    for ( ;; ) {
        if ( xSemaphoreTake( writeFlash, ( TickType_t ) 10 ) == pdTRUE ) {
            buffer[0] = state;
            flash_safe_execute( write_to_flash, buffer, 1000 );
            xSemaphoreGive( updateMode );
        } else {
            vTaskDelay( 100 );
        }
    }
}

void led_task( void* leds ) {
    ( led_array_t* ) leds;
    *( ( led_array_t* ) leds ) = led_array_init();

    uint64_t delay = 500;

    led_array_update_mode( leds, flash_target_contents[0] );

    for ( ;; ) {
        // led_array_update_mode( leds, state );
        if ( xSemaphoreTake( updateMode, ( TickType_t ) 1 ) == pdTRUE ) {
            led_array_update_mode( leds, flash_target_contents[0] );
        }
        led_array_update_values( leds, &delay );
        led_array_set_levels( leds );
        vTaskDelay( delay );
    }
}