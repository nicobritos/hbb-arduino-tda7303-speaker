#include "Arduino.h"
#include <EEPROM.h>
#include "defines.h"

#define EE_START 10
#define EE_ADDRESS_BKLIGHT (EE_START + 0)
#define EE_ADDRESS_BKLIGHT_TIMEOUT (EE_START + 1)
#define EE_ADDRESS_CURR_AUDIO_LEFT (EE_START + 2)
#define EE_ADDRESS_CURR_AUDIO_RIGHT (EE_START + 3)
#define EE_ADDRESS_INPUT_CONFIG_START (EE_START + 4)

void resetEEPROM() {
    EEPROM.update(EE_ADDRESS_BKLIGHT, 0);
    EEPROM.update(EE_ADDRESS_BKLIGHT_TIMEOUT, 0);
    EEPROM.update(EE_ADDRESS_CURR_AUDIO_LEFT, 0);
    EEPROM.update(EE_ADDRESS_CURR_AUDIO_RIGHT, 0);
    for (uint8_t i = 0; i < QTY_INPUTS; ++i) {
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8, 0);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 1, 0);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 2, 0);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 3, 0);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 4, 0);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 5, 0);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 6, 0);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 7, 0);
    }
}

void saveSettings(actualConfigType *actualConfig) {
    EEPROM.update(EE_ADDRESS_BKLIGHT, actualConfig->backlight);
    EEPROM.update(EE_ADDRESS_BKLIGHT_TIMEOUT, actualConfig->backlight_timeout);
    EEPROM.update(EE_ADDRESS_CURR_AUDIO_LEFT, actualConfig->currentAudio[LEFT]);
    EEPROM.update(EE_ADDRESS_CURR_AUDIO_RIGHT, actualConfig->currentAudio[RIGHT]);
    for (uint8_t i = 0; i < QTY_INPUTS; ++i) {
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8, actualConfig->inputValues[i].bass);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 1, actualConfig->inputValues[i].treble);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 2, actualConfig->inputValues[i].loud);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 3, actualConfig->inputValues[i].left);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 4, actualConfig->inputValues[i].right);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 5, actualConfig->inputValues[i].gain);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 6, actualConfig->inputValues[i].volume);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 7, actualConfig->inputValues[i].subwoofer);
    }
}

void restoreConfig(actualConfigType *actualConfig) {
    actualConfig->backlight = EEPROM.read(EE_ADDRESS_BKLIGHT);
    actualConfig->backlight_timeout = EEPROM.read(EE_ADDRESS_BKLIGHT_TIMEOUT);
    actualConfig->currentAudio[LEFT] = EEPROM.read(EE_ADDRESS_CURR_AUDIO_LEFT);
    actualConfig->currentAudio[RIGHT] = EEPROM.read(EE_ADDRESS_CURR_AUDIO_RIGHT);
    for (uint8_t i = 0; i < QTY_INPUTS; ++i) {
        actualConfig->inputValues[i].bass = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8);
        actualConfig->inputValues[i].treble = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 1);
        actualConfig->inputValues[i].loud = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 2);
        actualConfig->inputValues[i].left = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 3);
        actualConfig->inputValues[i].right = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 4);
        actualConfig->inputValues[i].gain = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 5);
        actualConfig->inputValues[i].volume = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 6);
        actualConfig->inputValues[i].subwoofer = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 7);
    }
    actualConfig->mute = 1;
}
