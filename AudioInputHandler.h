#ifndef _AUDIO_HANDLER_H

    #include "Arduino.h"
    #include "TDA7303.h"

    #define _AUDIO_HANDLER_H

    typedef struct audioEntryCDT *audioEntryADT;
    typedef struct relayInputCDT *relayInputADT;
    
    audioEntryADT addAudioEntry(uint8_t tdaInput);
    relayInputADT registerRelay(
        audioEntryADT audioEntry, 
        int8_t pinLeft, 
        uint8_t pinLeftActive, 
        int8_t pinRight, 
        uint8_t pinRightActive);
    void setAudioInput(TDA7303 *amplifier, audioEntryADT audioEntry);
    void setAudioInputLeft(TDA7303 *amplifier, audioEntryADT audioEntry);
    void setAudioInputRight(TDA7303 *amplifier, audioEntryADT audioEntry);
    uint8_t getTDAInput(audioEntryADT audioEntry);
    
#endif
