#ifndef _AUDIO_HANDLER_H

    #include "Arduino.h"

    #define _AUDIO_HANDLER_H

    typedef audioEntryCDT *audioEntryADT;
    typedef relayInputCDT *relayInputADT;

    audioEntryADT addAudioEntry(uint8_t tdaInput);
    relayInputADT registerRelay(audioEntryADT audioEntry, int8_t pinLeft, uint8_t pinLeftActive, int8_t pinRight, uint8_t pinRightActive);
    void setAudioInput(audioEntryADT audioEntry);

#endif
