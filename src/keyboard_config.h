#ifndef _KEYBOARD_CONFIG_H_
#define _KEYBOARD_CONFIG_H_

#define PROTOTYPE_CONFIG 1

#if PROTOTYPE_CONFIG
// Status LED
#define LED_PIN   25

// Switches pins
#define K0        0
#define K1        1
#define K2        2
#define K3        3
#define K4        7
#define K5        8
#define K6        4
#define K7        5
#define K8        6

// Switches LEDS
#define LED_COUNT 9
#define LED0      18
#define LED1      19
#define LED2      20
#define LED3      21
#define LED4      22
#define LED5      26
#define LED6      27
#define LED7      28
#define LED8      15

#define LEDN                                                                   \
    { LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8 }

#else

// Status LED
#define LED_PIN   0

// Switches pins
#define K0        1
#define K1        3
#define K2        5
#define K3        12
#define K4        14
#define K5        16
#define K6        20
#define K7        22
#define K8        24

// Switches LEDS
#define LED_COUNT 10
#define LED0      2
#define LED1      4
#define LED2      6
#define LED3      13
#define LED4      15
#define LED5      17
#define LED6      19
#define LED7      21
#define LED8      23
#define LED9      25
#define LEDN                                                                   \
    { LED0, LED1, LED2, LED4, LED3, LED6, LED5, LED7, LED8, LED9 }
#endif

#endif /* _KEYBOARD_CONFIG_H_ */