#ifndef _C200_DEFINES_H
    #define _C200_DEFINES_H

    #include "Arduino.h"

    // Amplifier constants
    #define RCA_0 0
    #define RCA_2 1
    #define RCA_3 2
    #define RCA_4 3
    #define AUX_1 4
    #define MIXED 5
    #define QTY_INPUTS 6

    #define TDA7303_ADDRESS 0x44

    // Pins constants
    #define BUTTON_CFG 0
    #define BUTTON_INP 1
    #define BUTTON_MUTE 13
    #define SDA A4
    #define SCL A5
    #define BKLED_PIN 3
    #define ENCODER_1 A6
    #define ENCODER_2 A7

    #define SW_R 5
    #define SW_L 12
    #define RELAY_1 6
    #define RELAY_2 7
    #define RELAY_3 8
    #define RELAY_4 9
    #define RELAY_5 10
    #define RELAY_6 11

    #define ENCODER_QTY 5
    
    // Backlight LED
    #define MAX_BKL 255
    #define MIN_BKL 0

    // EEPROM constants
    #define DIRTY_I 60000

    // Global constants
    #define LEFT 0
    #define RIGHT 1

    typedef struct inputValuesType {
        int8_t bass;
        int8_t treble;
        int8_t loud;
        int8_t left;
        int8_t right;
        int8_t gain;
        int8_t volume;
        int8_t subwoofer;
    } inputValuesType;

    typedef struct actualConfigType {
        inputValuesType inputValues[QTY_INPUTS + 1];
        uint8_t currentAudio[2];
        uint8_t backlight;
        uint8_t backlight_timeout;
        uint8_t mute;
    } actualConfigType;
#endif
