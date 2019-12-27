#include <Wire.h>
#include "TDA7303.h"
#include "defines.h"
#include "AudioInputHandler.h"
#include "ConfigurationHandler.h"
#include "LCDHandler.h"
#include "InputHandler.h"

actualConfigType actualConfig = {0};

TDA7303 *amplifier = new TDA7303(TDA7303_ADDRESS);
InputHandler *inputHandler = new InputHandler();

audioEntryADT audioEntries[QTY_INPUTS] = {0};

uint8_t dirty = 0; // Save EEPROM when dirty is 1
uint32_t lastDirty = 0;
uint32_t lastChangedMenu = 0;

void updateAudioSettings();
void updateInput(uint8_t audio, uint8_t side);
void mute(bool value);
void processEncoder(int8_t enc);
void updateTDA();

void setup() {
    restoreConfig(&actualConfig);
    initializeLCDHandler(&actualConfig);
    Wire.begin();
    
    // Set up buttons
    inputHandler->registerPin(BUTTON_CFG, HIGH);
    inputHandler->registerPin(BUTTON_INP, HIGH);
    inputHandler->registerPin(BUTTON_MUTE, HIGH);

    // Set up volume knob
    inputHandler->registerEncoder(ENCODER_1, ENCODER_2, ENCODER_INPUT_CODE, ENCODER_QTY);
    
    // Set up audio inputs
    pinMode(SW_R, OUTPUT);
    pinMode(SW_L, OUTPUT);
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);
    pinMode(RELAY_5, OUTPUT);
    pinMode(RELAY_6, OUTPUT);
    
    digitalWrite(SW_R, 0);
    digitalWrite(SW_L, 0);
    digitalWrite(RELAY_1, 0);
    digitalWrite(RELAY_2, 0);
    digitalWrite(RELAY_3, 0);
    digitalWrite(RELAY_4, 0);
    digitalWrite(RELAY_5, 0);
    digitalWrite(RELAY_6, 0);

    // RCA0 WORKS
    audioEntries[RCA_0] = addAudioEntry(TDA_INPUT_2);
    registerRelay(audioEntries[RCA_0], RELAY_2, 0, RELAY_1, 0);
    registerRelay(audioEntries[RCA_0], RELAY_3, 0, RELAY_4, 0);
    registerRelay(audioEntries[RCA_0], RELAY_5, 0, RELAY_6, 0);
    registerRelay(audioEntries[RCA_0], SW_L, 0, SW_R, 0);

    // TODO: Check TDA_INPUT
    audioEntries[RCA_2] = addAudioEntry(TDA_INPUT_1);
    registerRelay(audioEntries[RCA_2], RELAY_2, 0, RELAY_1, 0);
    registerRelay(audioEntries[RCA_2], RELAY_3, 1, RELAY_4, 1);
    registerRelay(audioEntries[RCA_2], RELAY_5, 0, RELAY_6, 0);
    registerRelay(audioEntries[RCA_2], SW_L, 1, SW_R, 1);

    audioEntries[RCA_3] = addAudioEntry(TDA_INPUT_2);
    registerRelay(audioEntries[RCA_3], RELAY_2, 0, RELAY_1, 0);
    registerRelay(audioEntries[RCA_3], RELAY_3, 1, RELAY_4, 1);
    registerRelay(audioEntries[RCA_3], RELAY_5, 1, RELAY_6, 1);
    registerRelay(audioEntries[RCA_3], SW_L, 1, SW_R, 1);

    audioEntries[RCA_4] = addAudioEntry(TDA_INPUT_2);
    registerRelay(audioEntries[RCA_4], RELAY_2, 0, RELAY_1, 0);
    registerRelay(audioEntries[RCA_4], RELAY_3, 0, RELAY_4, 0);
    registerRelay(audioEntries[RCA_4], RELAY_5, 0, RELAY_6, 0);
    registerRelay(audioEntries[RCA_4], SW_L, 1, SW_R, 1);

    audioEntries[AUX_1] = addAudioEntry(TDA_INPUT_3);
    registerRelay(audioEntries[AUX_1], RELAY_2, 1, RELAY_1, 1);
    registerRelay(audioEntries[AUX_1], RELAY_3, 1, RELAY_4, 1);
    registerRelay(audioEntries[AUX_1], RELAY_5, 0, RELAY_6, 0);
    registerRelay(audioEntries[AUX_1], SW_L, 1, SW_R, 1);

    // Finish set up
    updateTDA();
    amplifier->setMute(true);
    updateInput(actualConfig.currentAudio[LEFT], LEFT);
    updateInput(actualConfig.currentAudio[RIGHT], RIGHT);
}

void loop() {
    const uint8_t *activePins = inputHandler->readInputs();
    if (inputHandler->getActivePinsCount() > 0) {
        // Let's only process 1 button at a time
        if (!actualConfig.mute) {
            interact(&actualConfig);
            switch (activePins[0]) {
                case BUTTON_CFG:
                    processConfigButton(&actualConfig);
                    break;
                case BUTTON_INP:
                    actualConfig.currentAudio[LEFT] = actualConfig.currentAudio[RIGHT] = (actualConfig.currentAudio[LEFT] + 1) % (QTY_INPUTS-1);  // Cicla todos menos el MIXED (que no es un input en si)
                    updateInput(actualConfig.currentAudio[LEFT], LEFT, actualConfig.inputValues[actualConfig.currentAudio[LEFT]].subwoofer);
                    updateInput(actualConfig.currentAudio[RIGHT], RIGHT, actualConfig.inputValues[actualConfig.currentAudio[LEFT]].subwoofer);
                    updateAudioSettings();
                    break;
            }
        }    
        if (activePins[0] == BUTTON_MUTE) {
            actualConfig.mute = !actualConfig.mute;
            if (actualConfig.mute) {
                mute(true);
                gotoMuteMenu(&actualConfig);
            } else {
                mute(false);
                interact(&actualConfig);
                gotoMainMenu(&actualConfig);
            }
        }
    }

    if (!actualConfig.mute) {
        encoderADT *activeEncoders = inputHandler->readEncoders();
        if (inputHandler->getActiveEncoderCount() > 0) {
            interact(&actualConfig);
            processEncoder(inputHandler->getEncoderDirection(activeEncoders[0]));
        }
    }

    if (dirty && millis() - lastDirty > DIRTY_I) {
        saveSettings(&actualConfig);
        dirty = 0;
    }
    refresh(&actualConfig);
}

void processEncoder(int8_t enc) {
    uint8_t dev, updateMe = 0;
    if (enc == 0)
        return;
    if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
        dev = MIXED;
    else
        dev = actualConfig.currentAudio[LEFT];

    switch (getCurrentMenu()) {
        case MENU_MAIN:
            if (enc > 0) {
                if (actualConfig.inputValues[dev].volume < MAX_VOL) {
                    actualConfig.inputValues[dev].volume++;
                    updateMe = 1;
                }
            } else {
                if (actualConfig.inputValues[dev].volume > MIN_VOL) {
                    actualConfig.inputValues[dev].volume--;
                    updateMe = 1;
                }
            }
            if (updateMe)
                setVolumeTDA(amplifier, actualConfig.inputValues[dev].volume);
            break;
            
        case MENU_TREB:
            if (enc > 0) {
                if (actualConfig.inputValues[dev].treble < MAX_TRB) {
                    actualConfig.inputValues[dev].treble++;
                    updateMe = 1;
                }
            } else {
                if (actualConfig.inputValues[dev].treble > MIN_TRB) {
                    actualConfig.inputValues[dev].treble--;
                    updateMe = 1;
                }
            }
            if (updateMe)
                setTreble(amplifier, actualConfig.inputValues[dev].treble);
            break;
            
        case MENU_BASS:
            if (enc > 0) {
                if (actualConfig.inputValues[dev].bass < MAX_BSS) {
                    actualConfig.inputValues[dev].bass++;
                    updateMe = 1;
                }
            } else {
                if (actualConfig.inputValues[dev].bass > MIN_BSS) {
                    actualConfig.inputValues[dev].bass--;
                    updateMe = 1;
                }
            }
            if (updateMe)
                setBass(amplifier, actualConfig.inputValues[dev].bass);
            break;
            
        case MENU_EXTBASS:
            if (enc > 0) {
                if (!actualConfig.inputValues[dev].subwoofer) {
                    actualConfig.inputValues[dev].subwoofer = 1;
                    updateMe = 1;
                }
            } else {
                if (actualConfig.inputValues[dev].subwoofer) {
                    actualConfig.inputValues[dev].subwoofer = 0;
                    updateMe = 1;
                }
            }
            if (updateMe) {
                updateInput(actualConfig.currentAudio[LEFT], LEFT, actualConfig.inputValues[dev].subwoofer);
                updateInput(actualConfig.currentAudio[RIGHT], RIGHT, actualConfig.inputValues[dev].subwoofer);
            }
            break;
            
        case MENU_LOUD:
            if (enc > 0) {
                if (!actualConfig.inputValues[dev].loud) {
                    actualConfig.inputValues[dev].loud = 1;
                    updateMe = 1;
                    setLoud(amplifier);
                }
            } else {
                if (actualConfig.inputValues[dev].loud) {
                    actualConfig.inputValues[dev].loud = 0;
                    updateMe = 1;
                    unsetLoud(amplifier);
                }
            }
            break;
            
        case MENU_OFFS:
            if (getSelectedConfig() == RIGHT) {
                if (enc > 0) {
                    if (actualConfig.inputValues[dev].right < MAX_ATT) {
                        actualConfig.inputValues[dev].right++;
                        updateMe = 1;
                    }
                } else if (enc < 0) {
                    if (actualConfig.inputValues[dev].right > MIN_ATT) {
                        actualConfig.inputValues[dev].right--;
                        updateMe = 1;
                    }
                }
                if (updateMe)
                    setRight(amplifier, actualConfig.inputValues[dev].right);
            } else if (getSelectedConfig() == LEFT) {
                if (enc > 0) {
                    if (actualConfig.inputValues[dev].left < MAX_ATT) {
                        actualConfig.inputValues[dev].left++;
                        updateMe = 1;
                    }
                } else if (enc < 0) {
                    if (actualConfig.inputValues[dev].left > MIN_ATT) {
                        actualConfig.inputValues[dev].left--;
                        updateMe = 1;
                    }
                }
                if (updateMe)
                    setLeft(amplifier, actualConfig.inputValues[dev].left);
            }
            break;
            
        case MENU_GAIN:
            if (getSelectedConfig() == RIGHT) {
                dev = actualConfig.currentAudio[RIGHT];
            } else if (getSelectedConfig() == LEFT) {
                dev = actualConfig.currentAudio[LEFT];
            } else {
                break;
            }
            if (enc > 0) {
                if (actualConfig.inputValues[dev].gain < MAX_GAN) {
                    actualConfig.inputValues[dev].gain++;
                    updateMe = 1;
                }
            } else if (enc < 0) {
                if (actualConfig.inputValues[dev].gain > MIN_GAN) {
                    actualConfig.inputValues[dev].gain--;
                    updateMe = 1;
                }
            }
            if (updateMe)
                setGain(amplifier, actualConfig.inputValues[dev].gain);
            break;
            
        case MENU_BLED:
            if (enc > 0) {
                if (actualConfig.backlight < MAX_BKL) {
                    actualConfig.backlight++;
                    setBacklight(actualConfig.backlight);
                    updateMe = 1;
                }
            }
            else if (enc < 0) {
                if (actualConfig.backlight > MIN_BKL) {
                    actualConfig.backlight--;
                    setBacklight(actualConfig.backlight);
                    updateMe = 1;
                }
            }
            break;
            
        case MENU_BLED_TIMEOUT:
            if (enc > 0) {
                if (actualConfig.backlight_timeout < 15) {
                    actualConfig.backlight_timeout += 5;
                } else if (actualConfig.backlight_timeout == 15) {
                    actualConfig.backlight_timeout = 30;
                } else if (actualConfig.backlight_timeout == 30) {
                    actualConfig.backlight_timeout = 60;
                }
                updateMe = 1;
            }
            else if (enc < 0) {
                if (actualConfig.backlight_timeout == 60) {
                    actualConfig.backlight_timeout = 30;
                } else if (actualConfig.backlight_timeout == 30) {
                    actualConfig.backlight_timeout = 15;
                } else if (actualConfig.backlight_timeout > 0) {
                    actualConfig.backlight_timeout -= 5;
                }
                updateMe = 1;
            }
            break;
            
        default:
            break;
    }

    if (updateMe) {
        updateCurrentMenu(&actualConfig);
        lastDirty = millis();
        dirty = 1;
    }
}

void mute(bool value) {
    amplifier->setMute(value);
    // updateInput(actualConfig.currentAudio[LEFT], LEFT);
    // updateInput(actualConfig.currentAudio[RIGHT], RIGHT);
    muteLCD(value);
}

void updateAudioSettings() {
    updateTDA();
    gotoMainMenu(&actualConfig);
    lastDirty = millis();
    dirty = 1;
}

void updateInput(uint8_t audio, uint8_t side) {
    if (audio >= QTY_INPUTS || (side != LEFT && side != RIGHT)) return;

    if (side == LEFT) {
        setAudioInputLeft(amplifier, audioEntries[audio]);
    } else {
        setAudioInputRight(amplifier, audioEntries[audio]);
    }
}

void updateTDA() {
    uint8_t dev;
    if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
        dev = MIXED;
    else dev = actualConfig.currentAudio[LEFT];

    amplifier->setGain(actualConfig.inputValues[dev].gain);
    amplifier->setVolumeTDA(actualConfig.inputValues[dev].volume);
    amplifier->setTreble(actualConfig.inputValues[dev].treble);
    amplifier->setBass(actualConfig.inputValues[dev].bass);
    amplifier->setRight(actualConfig.inputValues[dev].right);
    amplifier->setLeft(actualConfig.inputValues[dev].left);

    updateInput(actualConfig.currentAudio[LEFT], LEFT, actualConfig.inputValues[dev].subwoofer);
    updateInput(actualConfig.currentAudio[RIGHT], RIGHT, actualConfig.inputValues[dev].subwoofer);
}
