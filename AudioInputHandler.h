#ifndef _AUDIO_HANDLER_H

    #include "Arduino.h"

    #define _AUDIO_HANDLER_H

    typedef audioEntryCDT *audioEntryADT;
    typedef relayInputCDT *relayInputADT;
    
    audioEntryADT addAudioEntry(uint8_t tdaInput);
    relayInputADT registerRelay(
        audioEntryADT audioEntry, 
        int8_t pinLeft, 
        uint8_t pinLeftActive, 
        int8_t pinRight, 
        uint8_t pinRightActive);
    void setAudioInput(TDA7303 amplifier, audioEntryADT audioEntry);
    void setAudioInputLeft(TDA7303 amplifier, audioEntryADT audioEntry);
    void setAudioInputRight(TDA7303 amplifier, audioEntryADT audioEntry);
    uint8_t getTDAInput(audioEntryADT audioEntry);
    
#endif
