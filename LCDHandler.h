#ifndef _LCD_HANDLER_H
    
    #include "Arduino.h"
    #include <LiquidCrystal.h>

    #define _LCD_HANDLER_H

    // MENU
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

    void initializeLCDHandler(actualConfigType *actualConfig);
    void setBacklight(uint8_t level);
    void muteLCD(bool value);
    void refresh(actualConfigType *actualConfig);
    void interact(actualConfigType *actualConfig);
    void processConfigButton(actualConfigType *actualConfig);
    void updateCurrentMenu(actualConfigType *actualConfig);
    void gotoMainMenu(actualConfigType *actualConfig);
    void gotoMuteMenu(actualConfigType *actualConfig);
    uint8_t getCurrentMenu();
    uint8_t getSelectedConfig();

#endif
