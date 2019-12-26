#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Wire.h>
#include "TDA7303.h"

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

#define MAX_BKL 255
#define MIN_BKL 0
#define ENCODER_QTY 5

#define RCA34_AUX 0
#define RCA2_AUX 1
#define RCA3_RCA4 2
#define AUDIO_OUT_LENGTH 3

#define MENU_MAIN 0
#define MENU_TREB 1
#define MENU_BASS 2
#define MENU_EXTBASS 3
#define MENU_LOUD 4
#define MENU_OFFS 5
#define MENU_GAIN 6
#define MENU_BLED 7
#define MENU_BLED_TIMEOUT 8
#define MENU_MUTE 9
#define QTY_MENU 10

#define RCA_0 0
#define RCA_2 1
#define RCA_3 2
#define RCA_4 3
#define AUX_1 4
#define MIXED 5
#define QTY_INPUTS 6

#define TDA7303_ADD 0x44

#define BUTTON_I 250
#define DIRTY_I 60000

#define BYTE_SIZE 8
#define LEFT 0
#define RIGHT 1

#define EE_START 10
#define EE_ADDRESS_BKLIGHT (EE_START + 0)
#define EE_ADDRESS_BKLIGHT_TIMEOUT (EE_START + 1)
#define EE_ADDRESS_CURR_AUDIO_LEFT (EE_START + 2)
#define EE_ADDRESS_CURR_AUDIO_RIGHT (EE_START + 3)
#define EE_ADDRESS_INPUT_CONFIG_START (EE_START + 4)

//const uint8_t audioOutL[AUDIO_OUT_LENGTH] = {
//        9,
//        10,
//        11
//};
//
//const uint8_t audioOutR[AUDIO_OUT_LENGTH] = {
//        7,
//        8,
//        12
//};

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

actualConfigType actualConfig;

LiquidCrystal lcd(4, 2, A0, A1, A2, A3);
amplifierADT amplifier;
InputHandler inputHandler();

uint8_t dirty = 0; // Save EEPROM when dirty is 1
uint8_t backlight = 0;
uint32_t lastDirty = 0;
uint32_t lastChangedMenu = 0;
uint32_t lastInteraction = 0;

uint8_t currentMenu = MENU_MAIN, configSelected = LEFT;
uint8_t readable[BUTTONS] = {0};
uint8_t state[BUTTONS] = {0};
uint8_t lastEncoded = 0, encoderCount = 0, encoderCount2 = 0;
uint32_t lastRead[BUTTONS] = {0};

uint8_t readButtons(uint8_t readable[BUTTONS], uint8_t state[BUTTONS], uint32_t lastRead[BUTTONS]);
void updateMenu(uint8_t menu, int8_t clearLcd);
void updateAudio(uint8_t audio, uint8_t side, uint8_t subwoofer, uint8_t silent);
void printAudioMenu(uint8_t audio);
void setBacklight(uint8_t lvl);
void restoreConfig();
void saveSettings();
void muteMe();
void unMuteMe();
int8_t updateEncoder();
void processEncoder(int8_t enc);
void updateTDA();
void fadeIn();
void fadeOut();

void setup() {
    restoreConfig();
    lcd.begin(16, 2);
    Wire.begin();
    amplifier = newAmplifier(TDA7303_ADD);

    pinMode(BKLED_PIN, OUTPUT);
    
    pinMode(SW_R, OUTPUT);
    pinMode(SW_L, OUTPUT);
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);
    pinMode(RELAY_5, OUTPUT);
    pinMode(RELAY_6, OUTPUT);
    
    digitalWrite(SW_R, LOW);
    digitalWrite(SW_L, LOW);
    digitalWrite(RELAY_1, LOW);
    digitalWrite(RELAY_2, LOW);
    digitalWrite(RELAY_3, LOW);
    digitalWrite(RELAY_4, LOW);
    digitalWrite(RELAY_5, LOW);
    digitalWrite(RELAY_6, LOW);

    updateTDA();
    if (actualConfig.mute) {
        setMute(amplifier);
        updateAudio(actualConfig.currentAudio[LEFT], LEFT, false, true);
        updateAudio(actualConfig.currentAudio[RIGHT], RIGHT, false, true);
        backlight = 0;
        updateMenu(MENU_MUTE, 1);
        setBacklight(0);
    }
    else {
        unMuteMe();
        fadeIn();
        backlight = 1;
        updateMenu(MENU_MAIN, 1);
    }
}

void loop() {
    if (readButtons(readable, state, lastRead)) {
        if (!actualConfig.mute) {
            if (!backlight && actualConfig.backlight_timeout != 0 && millis() - lastInteraction > actualConfig.backlight_timeout*1000) {
                fadeIn();
                backlight = 1;
            }
            if (state[BUTTON_CFG_IX]) {
                if ((currentMenu == MENU_OFFS && configSelected == LEFT) || (currentMenu == MENU_GAIN && configSelected == LEFT && actualConfig.currentAudio[RIGHT] != actualConfig.currentAudio[LEFT])) {
                    configSelected = RIGHT;
                    updateMenu(currentMenu, 0);
                } else {
                    configSelected = LEFT;
                    updateMenu((currentMenu + 1) % (QTY_MENU - 1), currentMenu == MENU_MAIN);  // MENU - 1 porque no queremos ciclar a MUTE (no deberia de aparecer y sabemos que es el ultimo)
                }
                state[BUTTON_CFG_IX] = 0;
            }
            if (state[BUTTON_INP_IX]) {
                actualConfig.currentAudio[LEFT] = actualConfig.currentAudio[RIGHT] = (actualConfig.currentAudio[LEFT] + 1) % (QTY_INPUTS-1);  // Cicla todos menos el MIXED (que no es un input en si)
                updateAudio(actualConfig.currentAudio[LEFT], LEFT, actualConfig.inputValues[actualConfig.currentAudio[LEFT]].subwoofer, false);
                updateAudio(actualConfig.currentAudio[RIGHT], RIGHT, actualConfig.inputValues[actualConfig.currentAudio[LEFT]].subwoofer, false);
                state[BUTTON_INP_IX] = 0;
            }
            lastInteraction = millis();
        }
        if (state[BUTTON_MUTE_IX]) {
            actualConfig.mute = !actualConfig.mute;
            if (actualConfig.mute) {
                muteMe();
                updateMenu(MENU_MUTE, 1);
            }
            else {
                unMuteMe();
                lastInteraction = millis();
                updateMenu(MENU_MAIN, 1);
            }
            state[BUTTON_MUTE_IX] = 0;
        }
    }
    if (dirty && millis() - lastDirty > DIRTY_I) {
        saveSettings();
        dirty = 0;
    }
    int8_t enc = updateEncoder();
    if (enc != 0) {
        if (!backlight && actualConfig.backlight_timeout != 0 && millis() - lastInteraction > actualConfig.backlight_timeout*1000) {
            fadeIn();
            backlight = 1;
        }
        processEncoder(enc);
        lastInteraction = millis();
    }
    if (backlight && actualConfig.backlight_timeout != 0 && millis() - lastInteraction > actualConfig.backlight_timeout*1000) {
        fadeOut();
        backlight = 0;
    }
}

void fadeIn() {
    for (int16_t i = 0; i <= actualConfig.backlight; i++) {
        analogWrite(BKLED_PIN, i);
        delayMicroseconds(500);
    }
}

void fadeOut() {
    for (int16_t i = actualConfig.backlight; i >= 0; i--) {
        analogWrite(BKLED_PIN, i);
        delayMicroseconds(500);
    }
}

void processEncoder(int8_t enc) {
    uint8_t dev, updateMe = 0;
    if (enc == 0)
        return;
    if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
        dev = MIXED;
    else
        dev = actualConfig.currentAudio[LEFT];

    switch (currentMenu) {
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
                updateAudio(actualConfig.currentAudio[LEFT], LEFT, actualConfig.inputValues[dev].subwoofer, true);
                updateAudio(actualConfig.currentAudio[RIGHT], RIGHT, actualConfig.inputValues[dev].subwoofer, true);
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
            if (configSelected == RIGHT) {
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
            } else if (configSelected == LEFT) {
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
            if (configSelected == RIGHT) {
                dev = actualConfig.currentAudio[RIGHT];
            } else if (configSelected == LEFT) {
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
        updateMenu(currentMenu, 0);
        lastDirty = millis();
        dirty = 1;
    }
}

void setBacklight(uint8_t lvl) {
    analogWrite(BKLED_PIN, lvl);
}

void muteMe() {
    setMute(amplifier);
    updateAudio(actualConfig.currentAudio[LEFT], LEFT, false, true);
    updateAudio(actualConfig.currentAudio[RIGHT], RIGHT, false, true);
    fadeOut();
    backlight = 0;
    updateMenu(MENU_MUTE, 1);
}

void unMuteMe() {
    unsetMute(amplifier);
    uint8_t dev = actualConfig.currentAudio[LEFT] == actualConfig.currentAudio[RIGHT] ? actualConfig.currentAudio[LEFT] : MIXED; 
    updateAudio(actualConfig.currentAudio[LEFT], LEFT, actualConfig.inputValues[dev].subwoofer, true);
    updateAudio(actualConfig.currentAudio[RIGHT], RIGHT, actualConfig.inputValues[dev].subwoofer, true);
    fadeIn();
    backlight = 1;
    updateMenu(MENU_MAIN, 1);
}

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

void saveSettings() {
    EEPROM.update(EE_ADDRESS_BKLIGHT, actualConfig.backlight);
    EEPROM.update(EE_ADDRESS_BKLIGHT_TIMEOUT, actualConfig.backlight_timeout);
    EEPROM.update(EE_ADDRESS_CURR_AUDIO_LEFT, actualConfig.currentAudio[LEFT]);
    EEPROM.update(EE_ADDRESS_CURR_AUDIO_RIGHT, actualConfig.currentAudio[RIGHT]);
    for (uint8_t i = 0; i < QTY_INPUTS; ++i) {
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8, actualConfig.inputValues[i].bass);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 1, actualConfig.inputValues[i].treble);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 2, actualConfig.inputValues[i].loud);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 3, actualConfig.inputValues[i].left);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 4, actualConfig.inputValues[i].right);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 5, actualConfig.inputValues[i].gain);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 6, actualConfig.inputValues[i].volume);
        EEPROM.update(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 7, actualConfig.inputValues[i].subwoofer);
    }
}

void restoreConfig() {
    actualConfig.backlight = EEPROM.read(EE_ADDRESS_BKLIGHT);
    actualConfig.backlight_timeout = EEPROM.read(EE_ADDRESS_BKLIGHT_TIMEOUT);
    actualConfig.currentAudio[LEFT] = EEPROM.read(EE_ADDRESS_CURR_AUDIO_LEFT);
    actualConfig.currentAudio[RIGHT] = EEPROM.read(EE_ADDRESS_CURR_AUDIO_RIGHT);
    for (uint8_t i = 0; i < QTY_INPUTS; ++i) {
        actualConfig.inputValues[i].bass = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8);
        actualConfig.inputValues[i].treble = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 1);
        actualConfig.inputValues[i].loud = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 2);
        actualConfig.inputValues[i].left = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 3);
        actualConfig.inputValues[i].right = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 4);
        actualConfig.inputValues[i].gain = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 5);
        actualConfig.inputValues[i].volume = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 6);
        actualConfig.inputValues[i].subwoofer = EEPROM.read(EE_ADDRESS_INPUT_CONFIG_START + i*8 + 7);
    }
    actualConfig.mute = 1;
}

void printAudioMenu(uint8_t audio) {
    switch (audio) {
        case RCA_0:
            lcd.print("RCA 1");
            break;
        case AUX_1:
            lcd.print("AUX");
            break;
        case RCA_2:
            lcd.print("RCA 2");
            break;
        case RCA_3:
            lcd.print("RCA 3");
            break;
        case RCA_4:
            lcd.print("RCA 4");
            break;
        case MIXED:
            lcd.print("Mixed");
            break;
    }
}

void updateMenu(uint8_t menu, int8_t printLcd) {
    uint8_t dev;
    if (printLcd)
        lcd.clear();
    else
        printLcd = currentMenu != menu;
    switch (menu) {
        case MENU_MAIN:
            lcd.noBlink();
            if (printLcd) {
                lcd.setCursor(0, 0);
                lcd.print("L:              ");
            }
            
            lcd.setCursor(3, 0);
            printAudioMenu(actualConfig.currentAudio[LEFT]);

            if (printLcd) {
                lcd.setCursor(8, 0);
                lcd.print("VOL:");
            }
            lcd.setCursor(12, 0);
            lcd.print("   ");
            
            if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig.currentAudio[LEFT];
            if (actualConfig.inputValues[dev].volume < 10)
                lcd.setCursor(15, 0);
            else
                lcd.setCursor(14, 0);
            lcd.print(actualConfig.inputValues[dev].volume, DEC);

            if (printLcd) {
                lcd.setCursor(0, 1);
                lcd.print("R:              ");
            }
            lcd.setCursor(3, 1);
            printAudioMenu(actualConfig.currentAudio[RIGHT]);

            if (printLcd) {
                lcd.setCursor(8, 1);
                lcd.print("L:");
            }
            if (actualConfig.inputValues[actualConfig.currentAudio[LEFT]].left < 10)
                lcd.setCursor(11, 1);
            else
                lcd.setCursor(10, 1);
            lcd.print(actualConfig.inputValues[actualConfig.currentAudio[LEFT]].left, DEC);
            
            if (printLcd)
                lcd.print("R:");
            if (actualConfig.inputValues[actualConfig.currentAudio[RIGHT]].right < 10)
                lcd.setCursor(15, 1);
            else
                lcd.setCursor(14, 1);
            lcd.print(actualConfig.inputValues[actualConfig.currentAudio[RIGHT]].right, DEC);
            break;

        case MENU_TREB:
            if (printLcd) {
                lcd.setCursor(0, 0);
                printAudioMenu(actualConfig.currentAudio[LEFT]);
                lcd.print("  -  ");
                printAudioMenu(actualConfig.currentAudio[RIGHT]);
                lcd.setCursor(0, 1);
                lcd.print("Treble:         ");
            } else {
                lcd.setCursor(8, 1);
                lcd.print("   ");
            }
            lcd.setCursor(8, 1);
            if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig.currentAudio[LEFT];
            lcd.print(actualConfig.inputValues[dev].treble, DEC);
            break;

        case MENU_BASS:
            if (printLcd) {
                lcd.setCursor(0, 1);
                lcd.print("Bass:           ");
            } else {
                lcd.setCursor(6, 1);
                lcd.print("   ");
            }
            lcd.setCursor(6, 1);
            if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig.currentAudio[LEFT];
            lcd.print(actualConfig.inputValues[dev].bass, DEC);
            break;

        case MENU_EXTBASS:
            if (printLcd) {
                lcd.setCursor(0, 1);
                lcd.print("Subwoofer:      ");
            } else {
                lcd.setCursor(11, 1);
                lcd.print("   ");
            }
            lcd.setCursor(11, 1);
            if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig.currentAudio[LEFT];
            
            if (actualConfig.inputValues[dev].subwoofer)
                lcd.print("ON");
            else
                lcd.print("OFF");
            break;
        
        case MENU_LOUD:
            if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig.currentAudio[LEFT];
            if (printLcd) {
                lcd.setCursor(0, 1);
                lcd.print("Loud:           ");
            } else {
                lcd.setCursor(6, 1);
                lcd.print("   ");
            }
            lcd.setCursor(6, 1);
            if (actualConfig.inputValues[dev].loud)
                lcd.print("ON");
            else
                lcd.print("OFF");
            break;

        case MENU_OFFS:
            if (printLcd) {
                lcd.setCursor(0, 1);
                lcd.print("O: L:    - R:   ");
                lcd.setCursor(0, 1);
            }
            lcd.setCursor(6, 1);
            lcd.print("   ");
            lcd.setCursor(6, 1);
            if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig.currentAudio[LEFT];
            lcd.print(actualConfig.inputValues[dev].left, DEC);

            lcd.setCursor(14, 1);
            lcd.print("  ");
            lcd.setCursor(14, 1);
            lcd.print(actualConfig.inputValues[dev].right, DEC);
            if (configSelected == LEFT) {
                lcd.setCursor(3, 1);
            } else {
                lcd.setCursor(11, 1);
            }
            lcd.blink();
            break;

        case MENU_GAIN:
            if (printLcd) {
                lcd.setCursor(0, 1);
                lcd.print("G: L:    - R:   ");
                lcd.setCursor(0, 1);
            }
            lcd.setCursor(6, 1);
            lcd.print("   ");
            lcd.setCursor(6, 1);
            lcd.print(actualConfig.inputValues[actualConfig.currentAudio[LEFT]].gain, DEC);

            lcd.setCursor(14, 1);
            lcd.print("  ");
            lcd.setCursor(14, 1);
            lcd.print(actualConfig.inputValues[actualConfig.currentAudio[RIGHT]].gain, DEC);
            if (configSelected == LEFT) {
                lcd.setCursor(3, 1);
            } else {
                lcd.setCursor(11, 1);
            }
            lcd.blink();
            break;
            
        case MENU_BLED:
            lcd.noBlink();
            if (printLcd) {
                lcd.setCursor(0, 0);
                lcd.print("Global Settings");
                lcd.setCursor(0, 1);
                lcd.print("LED:            ");
            }
            lcd.setCursor(5, 1);
            lcd.print("    ");
            lcd.setCursor(5, 1);
            if (!actualConfig.backlight) {
                lcd.print("Off");
            } else if (actualConfig.backlight < 255) {
                lcd.print(actualConfig.backlight, DEC);
            } else {
                lcd.print("Full");
            }
            break;

        case MENU_BLED_TIMEOUT:
            if (printLcd) {
                lcd.setCursor(0, 0);
                lcd.print("Global Settings");
                lcd.setCursor(0, 1);
                lcd.print("LED Timeout:");
                lcd.setCursor(0, 1);
            }
            lcd.setCursor(12, 1);
            lcd.print("    ");
            lcd.setCursor(12, 1);
            if (actualConfig.backlight_timeout == 0) {
                lcd.print("None");
            } else if (actualConfig.backlight_timeout == 5) {
                lcd.print("  5s");
            } else if (actualConfig.backlight_timeout == 10) {
                lcd.print(" 10s");
            } else if (actualConfig.backlight_timeout == 15) {
                lcd.print(" 15s");
            } else if (actualConfig.backlight_timeout == 30) {
                lcd.print(" 30s");
            } else if (actualConfig.backlight_timeout == 60) {
                lcd.print("  1m");
            }
            break;
            
        case MENU_MUTE:
            lcd.setCursor(5, 0);
            lcd.print("Muted.");
            break;

        default:
            break;
    }
    currentMenu = menu;
}

// TODO: Check TDA_INPUT
void updateAudio(uint8_t audio, uint8_t side, uint8_t subwoofer, uint8_t silent) {
    if (side == LEFT) {
        switch (audio) {
            case RCA_0:
                digitalWrite(RELAY_2, LOW);
                digitalWrite(RELAY_3, LOW);
                digitalWrite(RELAY_5, LOW);
                if (subwoofer)
                    digitalWrite(SW_L, LOW);
                else
                    digitalWrite(SW_L, HIGH);
                setInput(amplifier, TDA_INPUT_2);
                break;
            case RCA_2:
                digitalWrite(RELAY_2, LOW);
                digitalWrite(RELAY_3, HIGH);
                digitalWrite(RELAY_5, LOW);
                if (subwoofer)
                    digitalWrite(SW_L, HIGH);
                else
                    digitalWrite(SW_L, LOW);
                setInput(amplifier, TDA_INPUT_1);
                break;
            case RCA_3:
                digitalWrite(RELAY_2, LOW);
                digitalWrite(RELAY_3, HIGH);
                digitalWrite(RELAY_5, HIGH);
                if (subwoofer)
                    digitalWrite(SW_L, HIGH);
                else
                    digitalWrite(SW_L, LOW);
                setInput(amplifier, TDA_INPUT_2);
                break;
            case RCA_4:
                digitalWrite(RELAY_2, LOW);
                digitalWrite(RELAY_3, LOW);
                digitalWrite(RELAY_5, LOW);
                if (subwoofer)
                    digitalWrite(SW_L, HIGH);
                else
                    digitalWrite(SW_L, LOW);
                setInput(amplifier, TDA_INPUT_2);
                break;
            case AUX_1:
                digitalWrite(RELAY_2, HIGH);
                digitalWrite(RELAY_3, HIGH);
                digitalWrite(RELAY_5, LOW);
                if (subwoofer)
                    digitalWrite(SW_L, HIGH);
                else
                    digitalWrite(SW_L, LOW);
                setInput(amplifier, TDA_INPUT_3);
                break;
        }
    } else if (side == RIGHT) {
        switch (audio) {
            case RCA_0:
                digitalWrite(RELAY_1, LOW);
                digitalWrite(RELAY_4, LOW);
                digitalWrite(RELAY_6, LOW);
                if (subwoofer)
                    digitalWrite(SW_R, LOW);
                else
                    digitalWrite(SW_R, HIGH);
                setInput(amplifier, TDA_INPUT_2);
                break;
            case RCA_2:
                digitalWrite(RELAY_1, LOW);
                digitalWrite(RELAY_4, LOW);
                digitalWrite(RELAY_6, HIGH);
                if (subwoofer)
                    digitalWrite(SW_R, HIGH);
                else
                    digitalWrite(SW_R, LOW);
                setInput(amplifier, TDA_INPUT_1);
                break;
            case RCA_3:
                digitalWrite(RELAY_1, LOW);
                digitalWrite(RELAY_4, HIGH);
                digitalWrite(RELAY_6, HIGH);
                if (subwoofer)
                    digitalWrite(SW_R, HIGH);
                else
                    digitalWrite(SW_R, LOW);
                setInput(amplifier, TDA_INPUT_2);
                break;
            case RCA_4:
                digitalWrite(RELAY_1, LOW);
                digitalWrite(RELAY_4, LOW);
                digitalWrite(RELAY_6, LOW);
                if (subwoofer)
                    digitalWrite(SW_R, HIGH);
                else
                    digitalWrite(SW_R, LOW);
                setInput(amplifier, TDA_INPUT_2);
                break;
            case AUX_1:
                digitalWrite(RELAY_1, HIGH);
                digitalWrite(RELAY_4, LOW);
                digitalWrite(RELAY_6, HIGH);
                if (subwoofer)
                    digitalWrite(SW_R, HIGH);
                else
                    digitalWrite(SW_R, LOW);
                setInput(amplifier, TDA_INPUT_3);
                break;
        }
    }
    
    if (silent)
        return;
    updateTDA();
    updateMenu(MENU_MAIN, 1);
    lastDirty = millis();
    dirty = 1;
}

void updateTDA() {
    uint8_t dev;
    if (actualConfig.currentAudio[LEFT] != actualConfig.currentAudio[RIGHT])
        dev = MIXED;
    else
        dev = actualConfig.currentAudio[LEFT];
    setGain(amplifier, actualConfig.inputValues[dev].gain);
    setVolumeTDA(amplifier, actualConfig.inputValues[dev].volume);
    setTreble(amplifier, actualConfig.inputValues[dev].treble);
    setBass(amplifier, actualConfig.inputValues[dev].bass);
    setRight(amplifier, actualConfig.inputValues[dev].right);
    setLeft(amplifier, actualConfig.inputValues[dev].left);
    updateAudio(actualConfig.currentAudio[LEFT], LEFT, actualConfig.inputValues[dev].subwoofer, true);
    updateAudio(actualConfig.currentAudio[RIGHT], RIGHT, actualConfig.inputValues[dev].subwoofer, true);
}
