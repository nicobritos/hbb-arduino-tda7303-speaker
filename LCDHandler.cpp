#include "LCDHandler.h"
#include "defines.h"

LiquidCrystal lcd(4, 2, A0, A1, A2, A3);

uint8_t currentMenu = MENU_MAIN, configSelected = LEFT;
uint32_t lastInteraction = 0;
uint8_t backlight = 0;

void fadeIn(actualConfigType *actualConfig);
void fadeOut(actualConfigType *actualConfig);
void printAudioMenu(uint8_t audio);
void updateMenu(actualConfigType *actualConfig, uint8_t menu, int8_t printLcd);

void initializeLCDHandler(actualConfigType *actualConfig) {
    lcd.begin(16, 2);

    pinMode(BKLED_PIN, OUTPUT);

    backlight = 0;
    updateMenu(actualConfig, MENU_MUTE, 1);
    setBacklight(0);
}

void setBacklight(uint8_t level) {
    analogWrite(BKLED_PIN, level);
}

void muteLCD(bool value) {
    if (value) {
        fadeOut();
        updateMenu(MENU_MUTE, 1);
    } else {
        fadeIn();
        updateMenu(MENU_MAIN, 1);
    }
}

void refresh(actualConfigType *actualConfig) {
    if (backlight && actualConfig->backlight_timeout != 0 && millis() - lastInteraction > actualConfig->backlight_timeout*1000) {
        fadeOut(actualConfig);
    }
}

void interact(actualConfigType *actualConfig) {
    if (!backlight && actualConfig->backlight_timeout != 0 && millis() - lastInteraction > actualConfig->backlight_timeout*1000) {
        fadeIn(actualConfig);
    }
    lastInteraction = millis();
}

void processConfigButton(actualConfigType *actualConfig) {
    if ((currentMenu == MENU_OFFS && configSelected == LEFT) || (currentMenu == MENU_GAIN && configSelected == LEFT && actualConfig.currentAudio[RIGHT] != actualConfig.currentAudio[LEFT])) {
        configSelected = RIGHT;
        updateMenu(actualConfig, currentMenu, 0);
    } else {
        configSelected = LEFT;
        updateMenu(actualConfig, (currentMenu + 1) % (QTY_MENU - 1), currentMenu == MENU_MAIN);  // MENU - 1 porque no queremos ciclar a MUTE (no deberia de aparecer y sabemos que es el ultimo)
    }
}

void updateCurrentMenu(actualConfigType *actualConfig) {
    updateMenu(actualConfig, currentMenu, 0);
}

void gotoMuteMenu(actualConfigType *actualConfig) {
    updateMenu(actualConfig, MENU_MUTE, 1)
}

void gotoMainMenu(actualConfigType *actualConfig) {
    updateMenu(actualConfig, MENU_MUTE, 1)
}

uint8_t getCurrentMenu() {
    return currentMenu;
}

uint8_t getSelectedConfig() {
    return configSelected;
}

// Private
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

void updateMenu(actualConfigType *actualConfig, uint8_t menu, int8_t printLcd) {
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
            printAudioMenu(actualConfig->currentAudio[LEFT]);

            if (printLcd) {
                lcd.setCursor(8, 0);
                lcd.print("VOL:");
            }
            lcd.setCursor(12, 0);
            lcd.print("   ");
            
            if (actualConfig->currentAudio[LEFT] != actualConfig->currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig->currentAudio[LEFT];
            if (actualConfig->inputValues[dev].volume < 10)
                lcd.setCursor(15, 0);
            else
                lcd.setCursor(14, 0);
            lcd.print(actualConfig->inputValues[dev].volume, DEC);

            if (printLcd) {
                lcd.setCursor(0, 1);
                lcd.print("R:              ");
            }
            lcd.setCursor(3, 1);
            printAudioMenu(actualConfig->currentAudio[RIGHT]);

            if (printLcd) {
                lcd.setCursor(8, 1);
                lcd.print("L:");
            }
            if (actualConfig->inputValues[actualConfig->currentAudio[LEFT]].left < 10)
                lcd.setCursor(11, 1);
            else
                lcd.setCursor(10, 1);
            lcd.print(actualConfig->inputValues[actualConfig->currentAudio[LEFT]].left, DEC);
            
            if (printLcd)
                lcd.print("R:");
            if (actualConfig->inputValues[actualConfig->currentAudio[RIGHT]].right < 10)
                lcd.setCursor(15, 1);
            else
                lcd.setCursor(14, 1);
            lcd.print(actualConfig->inputValues[actualConfig->currentAudio[RIGHT]].right, DEC);
            break;

        case MENU_TREB:
            if (printLcd) {
                lcd.setCursor(0, 0);
                printAudioMenu(actualConfig->currentAudio[LEFT]);
                lcd.print("  -  ");
                printAudioMenu(actualConfig->currentAudio[RIGHT]);
                lcd.setCursor(0, 1);
                lcd.print("Treble:         ");
            } else {
                lcd.setCursor(8, 1);
                lcd.print("   ");
            }
            lcd.setCursor(8, 1);
            if (actualConfig->currentAudio[LEFT] != actualConfig->currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig->currentAudio[LEFT];
            lcd.print(actualConfig->inputValues[dev].treble, DEC);
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
            if (actualConfig->currentAudio[LEFT] != actualConfig->currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig->currentAudio[LEFT];
            lcd.print(actualConfig->inputValues[dev].bass, DEC);
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
            if (actualConfig->currentAudio[LEFT] != actualConfig->currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig->currentAudio[LEFT];
            
            if (actualConfig->inputValues[dev].subwoofer)
                lcd.print("ON");
            else
                lcd.print("OFF");
            break;
        
        case MENU_LOUD:
            if (actualConfig->currentAudio[LEFT] != actualConfig->currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig->currentAudio[LEFT];
            if (printLcd) {
                lcd.setCursor(0, 1);
                lcd.print("Loud:           ");
            } else {
                lcd.setCursor(6, 1);
                lcd.print("   ");
            }
            lcd.setCursor(6, 1);
            if (actualConfig->inputValues[dev].loud)
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
            if (actualConfig->currentAudio[LEFT] != actualConfig->currentAudio[RIGHT])
                dev = MIXED;
            else
                dev = actualConfig->currentAudio[LEFT];
            lcd.print(actualConfig->inputValues[dev].left, DEC);

            lcd.setCursor(14, 1);
            lcd.print("  ");
            lcd.setCursor(14, 1);
            lcd.print(actualConfig->inputValues[dev].right, DEC);
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
            lcd.print(actualConfig->inputValues[actualConfig->currentAudio[LEFT]].gain, DEC);

            lcd.setCursor(14, 1);
            lcd.print("  ");
            lcd.setCursor(14, 1);
            lcd.print(actualConfig->inputValues[actualConfig->currentAudio[RIGHT]].gain, DEC);
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
            if (!actualConfig->backlight) {
                lcd.print("Off");
            } else if (actualConfig->backlight < 255) {
                lcd.print(actualConfig->backlight, DEC);
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
            if (actualConfig->backlight_timeout == 0) {
                lcd.print("None");
            } else if (actualConfig->backlight_timeout == 5) {
                lcd.print("  5s");
            } else if (actualConfig->backlight_timeout == 10) {
                lcd.print(" 10s");
            } else if (actualConfig->backlight_timeout == 15) {
                lcd.print(" 15s");
            } else if (actualConfig->backlight_timeout == 30) {
                lcd.print(" 30s");
            } else if (actualConfig->backlight_timeout == 60) {
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

void fadeIn(actualConfigType *actualConfig) {
    for (int16_t i = 0; i <= actualConfig->backlight; i++) {
        analogWrite(BKLED_PIN, i);
        delayMicroseconds(500);
    }
    backlight = 1;
}

void fadeOut(actualConfigType *actualConfig) {
    for (int16_t i = actualConfig->backlight; i >= 0; i--) {
        analogWrite(BKLED_PIN, i);
        delayMicroseconds(500);
    }
    backlight = 0;
}
