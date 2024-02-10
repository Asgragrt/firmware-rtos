#ifndef _TUD_UTILS_H_
#define _TUD_UTILS_H_

#include "tusb.h"

#include "FreeRTOS.h"
#include "timers.h"

/* A combination of interfaces must have a unique product id, since PC will save
 * device driver after the first plug. Same VID/PID with different interface e.g
 * MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP( itf, n ) ( ( CFG_TUD_##itf ) << ( n ) )
#define USB_PID                                                                \
    ( 0x4000 | _PID_MAP( CDC, 0 ) | _PID_MAP( MSC, 1 ) | _PID_MAP( HID, 2 ) |  \
      _PID_MAP( MIDI, 3 ) | _PID_MAP( VENDOR, 4 ) )

#define POLLING_INTERVAL 1 // ms
// Number of parallel keys to send per report
#define KEYCODE_BUFFER   20

#define BITMAP_BYTE_SIZE 20

// clang-format off
#define TUD_HID_REPORT_DESC_NKRO_KEYBOARD() \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP                         ),\
    HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD                     ),\
    HID_COLLECTION ( HID_COLLECTION_APPLICATION                     ),\
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD                        ),\
        HID_USAGE_MIN    ( 224                                      ),\
        HID_USAGE_MAX    ( 231                                      ),\
        HID_LOGICAL_MIN  ( 0                                        ),\
        HID_LOGICAL_MAX  ( 1                                        ),\
        HID_REPORT_COUNT ( 8                                        ),\
        HID_REPORT_SIZE  ( 1                                        ),\
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE   ),\
    /* 7 bytes of padding for boot support (1 Reserved - 6 for 6kro) */ \
    HID_REPORT_SIZE        ( 8                                      ),\
    HID_REPORT_COUNT       ( 1 + 6                                  ),\
    HID_INPUT              ( HID_CONSTANT                           ),\
    /*LED output report*/ \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_LED                             ),\
        HID_REPORT_SIZE    ( 1                                      ),\
        HID_REPORT_COUNT   ( 5                                      ),\
        HID_USAGE_MIN      ( 1                                      ),\
        HID_USAGE_MAX      ( 5                                      ),\
        HID_OUTPUT         ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),\
    /*LED 3 bit padding*/ \
    HID_REPORT_SIZE        ( 3                                      ),\
    HID_REPORT_COUNT       ( 1                                      ),\
    HID_OUTPUT             ( HID_CONSTANT                           ),\
    /*bitmap of keys (20 bytes)*/\
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD                        ),\
        HID_REPORT_SIZE    ( 1                                      ),\
        HID_REPORT_COUNT_N ( BITMAP_BYTE_SIZE * 8, 2                ),\
        HID_LOGICAL_MIN    ( 0                                      ),\
        HID_LOGICAL_MAX    ( 1                                      ),\
        HID_USAGE_MIN      ( 0                                      ),\
        HID_USAGE_MAX      ( BITMAP_BYTE_SIZE * 8 - 1               ),\
        HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),\
    HID_COLLECTION_END
// clang-format on

//--------------------------------------------------------------------+
// Keyboard Report Struct
//--------------------------------------------------------------------+

typedef struct TU_ATTR_PACKED {
    uint8_t modifier; /**< Keyboard modifier (KEYBOARD_MODIFIER_* masks). */
    uint8_t reserved; /**< Reserved for OEM use, always set to 0. */
    uint8_t boot_keys[6];
    uint8_t key_bitmap[BITMAP_BYTE_SIZE];
} hid_nkro_keyboard_report_t;

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum {
    ITF_KBD,
    ITF_INOUT,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN                                                       \
    ( TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN )

#define EPNUM_HID_KBD   0x82
#define EPNUM_HID_INOUT 0x01

enum {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

//--------------------------------------------------------------------+
// Function prototypes
//--------------------------------------------------------------------+
void usb_task( void* param );

bool tud_hid_n_nkro_keyboard_report( uint8_t instance, uint8_t report_id,
                                     uint8_t keycode[KEYCODE_BUFFER] );

uint16_t tud_hid_get_report_cb( uint8_t itf, uint8_t report_id,
                                hid_report_type_t report_type, uint8_t* buffer,
                                uint16_t reqlen );
void tud_hid_set_report_cb( uint8_t itf, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize );

void hid_task( void* kbd );

void led_blinky_timer( void );
void led_blinky_cb( TimerHandle_t xTimer );

void tud_mount_cb( void );
void tud_umount_cb( void );
void tud_suspend_cb( bool remote_wakeup_en );
void tud_resume_cb( void );

#endif /* _TUD_UTILS_H_ */