#ifndef _C200_DEFINES_H
    #define _C200_DEFINES_H

    // Amplifier constants
    #define RCA_0 0
    #define RCA_2 1
    #define RCA_3 2
    #define RCA_4 3
    #define AUX_1 4
    #define MIXED 5
    #define QTY_INPUTS 6

    #define TDA7303_ADDRESS 0x44

    #define SDA A4
    #define SCL A5
    #define BKLED_PIN 3
    #define ENCODER_1 A6
    #define ENCODER_2 A7

    // EEPROM constants
    #define DIRTY_I 60000

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
