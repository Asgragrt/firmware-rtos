#include <stdio.h>
#include "pico/stdlib.h"

#include "tusb.h"
//#include "bsp/board.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "queue.h"

#include "leds/leds.h"
#include "kbd/kbd.h"

#include "tusb/tud_utils.h"

#include "flash/flash.h"
#include "pico/flash.h"

#include "pico/bootrom.h"

// TODO clean main file
// TODO use HID IN/OUT to transfer data between the device and the computer
// TODO app that handles the data transfer
// TODO clean flash directory

#define FLA_STACK_SIZE      configMINIMAL_STACK_SIZE * 2
#define USB_STACK_SIZE    (3*configMINIMAL_STACK_SIZE/2) * (CFG_TUSB_DEBUG ? 2 : 1)
#define HID_STACK_SIZE      configMINIMAL_STACK_SIZE
#define CDC_STACK_SIZE      configMINIMAL_STACK_SIZE
#define LED_STACK_SIZE      configMINIMAL_STACK_SIZE

#define LED_PIN 25

enum {
    ITF_KEYBOARD,
    ITF_INOUT,
};

enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

TimerHandle_t blinky_tm;

static void led_blinky_cb(TimerHandle_t xTimer);
static void fla_task(void *param);
static void usb_task(void *param);
static void hid_task(void *kbd);
static void led_task(void *leds);

keyboard_t* kbd;
led_array_t led_array;
uint8_t state = 0;

SemaphoreHandle_t writeFlash, updateMode;

void write_to_flash( void* param );

void boot_if_pressed( keyboard_t* kbd ) {
    if ( !kbd_is_pressed(kbd, 7) || !kbd_is_pressed(kbd, 8) ) return;

    reset_usb_boot( 0, 0);
}

void main(){
    kbd = keyboard_new();
    keyboard_init(kbd); 

    boot_if_pressed( kbd );

    writeFlash = xSemaphoreCreateBinary();
    updateMode = xSemaphoreCreateBinary();

    TaskHandle_t handleUSB, handleHID, handleFLA, handleCDC, handleLED;
    blinky_tm = xTimerCreate(NULL, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb);
    //xTaskCreate(fla_task, "fla", FLA_STACK_SIZE,       NULL, configMAX_PRIORITIES - 10, &handleFLA);
    xTaskCreate(usb_task, "usb", USB_STACK_SIZE,       NULL, configMAX_PRIORITIES - 2, &handleUSB);
    xTaskCreate(hid_task, "hid", HID_STACK_SIZE,        kbd, configMAX_PRIORITIES - 2, &handleHID);
    xTaskCreate(led_task, "led", LED_STACK_SIZE, &led_array, configMAX_PRIORITIES - 3, &handleLED);

    vTaskCoreAffinitySet( handleUSB, ( 1 << 0 ) );
    vTaskCoreAffinitySet( handleHID, ( 1 << 1 ) );
    xTimerStart(blinky_tm, 0);

    // Start the scheduler
    vTaskStartScheduler();

}

void write_to_flash( void* param ) {
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, (uint8_t*) param, FLASH_PAGE_SIZE);
}

void fla_task(void *param){
    uint8_t buffer[256] = {0};
    for(;;){
        if( xSemaphoreTake(writeFlash, (TickType_t) 10) == pdTRUE ) {
            buffer[0] = state;
            flash_safe_execute( write_to_flash , buffer, 1000 );
            xSemaphoreGive( updateMode );
        } else {
            vTaskDelay( 100 );
        }
    }
}

void led_task(void *leds) {
    (led_array_t*) leds;
    *( (led_array_t*) leds )= led_array_init();

    uint64_t delay = 500;

    led_array_update_mode( leds, flash_target_contents[0] );

    for(;;){
        //led_array_update_mode( leds, state );
        if ( xSemaphoreTake(updateMode, (TickType_t) 1) == pdTRUE ) { 
            led_array_update_mode( leds, flash_target_contents[0] );
        }
        led_array_update_values( leds, &delay );
        led_array_set_levels( leds );
        vTaskDelay( delay );
    }
}

static void usb_task(void *param) {
  (void) param;
  gpio_init( LED_PIN );
  gpio_set_dir( LED_PIN, true );
  //board_init();
  tusb_init();
  // init device stack on configured roothub port
  // This should be called after scheduler/kernel is started.
  // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  tud_init(BOARD_TUD_RHPORT);


  // RTOS forever loop
  while (1) {
    // put this thread to waiting state until there is new events
    tud_task();
  }
}

static void led_blinky_cb(TimerHandle_t xTimer) {
  (void) xTimer;
  static bool led_state = false;

  //board_led_write(led_state);
  gpio_put( LED_PIN, led_state );
  led_state = 1 - led_state; // toggle
}

void tud_mount_cb(void) {
  xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  if (tud_mounted()) {
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
  } else {
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
  }
}

void hid_task(void* kbd){
    (keyboard_t* ) kbd;
    uint8_t buffer[keycode_buffer];
    //vTaskDelay( pdMS_TO_TICKS(10) );
    
    for(;;){
        vTaskDelay( pdMS_TO_TICKS(POLLING_INTERVAL) );

        if ( tud_suspended() ){
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            vTaskDelay( pdMS_TO_TICKS(100) );
            if ( keyboard_update_status(kbd) ){
                tud_remote_wakeup();
            }
            continue;
        }

        if ( !tud_hid_n_ready( ITF_KEYBOARD ) ) continue;
        
        //Clear array buffer
        memset( buffer, 0, keycode_buffer * sizeof( uint8_t ) );
        keyboard_update_buffer(kbd, buffer, keycode_buffer);

        tud_hid_n_nkro_keyboard_report(ITF_KEYBOARD, 0, buffer);
        
    }
}