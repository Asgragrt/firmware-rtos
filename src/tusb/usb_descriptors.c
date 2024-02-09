#include "pico/unique_id.h"
#include "tud_utils.h"
#include "tusb.h"

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength = sizeof( tusb_desc_device_t ),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    /*
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    */
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0xCafe,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01 };

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb( void ) {
    return ( uint8_t const* ) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
uint8_t const desc_hid_report_kbd[] = {
    TUD_HID_REPORT_DESC_NKRO_KEYBOARD()
    // TUD_HID_REPORT_DESC_KEYBOARD()
};

uint8_t const desc_hid_report_inout[] = {
    TUD_HID_REPORT_DESC_GENERIC_INOUT( CFG_TUD_HID_EP_BUFSIZE ) };

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_hid_descriptor_report_cb( uint8_t itf ) {
    //(void) itf;
    switch ( itf ) {
        case 0:
            return desc_hid_report_kbd;

        case 1:
            return desc_hid_report_inout;

        default:
            return NULL;
    }
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR( 1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                           TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100 ),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR( ITF_NUM_HID_KBD, 4, HID_ITF_PROTOCOL_NONE,
                        sizeof( desc_hid_report_kbd ), EPNUM_HID_KBD,
                        CFG_TUD_HID_EP_BUFSIZE, POLLING_INTERVAL ),
    TUD_HID_INOUT_DESCRIPTOR( ITF_NUM_HID_INOUT, 5, HID_ITF_PROTOCOL_NONE,
                              sizeof( desc_hid_report_inout ), EPNUM_HID_INOUT,
                              0x80 | EPNUM_HID_INOUT, CFG_TUD_HID_EP_BUFSIZE,
                              POLLING_INTERVAL ),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb( uint8_t index ) {
    ( void ) index; // for multiple configurations
    return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

char unique_id[17];

// array of pointer to string descriptors
char const* string_desc_arr[] = {
    ( const char[] ){ 0x09,
                      0x04 },  // 0: is supported language is English (0x0409)
    "Asgragrt",                // 1: Manufacturer
    "KeyTao",                  // 2: Product
    unique_id,                 // 3: Serials, should use chip ID
    "NKRO Keyboard Interface", // 4: Keyboard Interface String
    "NKRO Keyboard Comms Interface" // 5: Keyboard Communication String
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const* tud_descriptor_string_cb( uint8_t index, uint16_t langid ) {
    ( void ) langid;
    pico_get_unique_board_id_string( unique_id, 32 );

    uint8_t chr_count;

    switch ( index ) {
        case 0:
            memcpy( &_desc_str[1], string_desc_arr[0], 2 );
            chr_count = 1;
            break;

        default:
            // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
            // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

            if ( !( index <
                    sizeof( string_desc_arr ) / sizeof( string_desc_arr[0] ) ) )
                return NULL;

            const char* str = string_desc_arr[index];

            // Cap at max char
            chr_count = ( uint8_t ) strlen( str );
            uint8_t const max_count =
                sizeof( _desc_str ) / sizeof( _desc_str[0] ) - 1;
            if ( chr_count > max_count ) chr_count = max_count;

            // Convert ASCII string into UTF-16
            for ( uint8_t i = 0; i < chr_count; i++ ) {
                _desc_str[1 + i] = str[i];
            }
            break;
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] =
        ( uint16_t ) ( ( TUSB_DESC_STRING << 8 ) | ( 2 * chr_count + 2 ) );

    return _desc_str;
}