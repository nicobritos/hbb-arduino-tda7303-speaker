#include "AudioInputHandler.h"
#include "LinkedList.h"

typedef struct relayInputCDT {
    int8_t pinLeft;
    int8_t pinRight;
    int8_t pinLeftSW;
    int8_t pinRightSW;
    uint8_t pinLeftActive;
    uint8_t pinRightActive;
    uint8_t pinLeftSWActive;
    uint8_t pinRightSWActive;
} relayInputCDT;

typedef struct audioEntryCDT {
    uint8_t tdaInput;
    LinkedList relayInputs;
} audioEntryCDT;

audioEntryADT addAudioEntry(uint8_t tdaInput) {
    audioEntryADT audioEntry = malloc(sizeof(*audioEntry));
    if (audioEntry == NULL) return NULL;

    audioEntry->tdaInput = tdaInput;
    audioEntry->relayInputs = new LinkedList();
    return audioEntry;
}

relayInputADT registerRelay(
    audioEntryADT audioEntry, 
    int8_t pinLeft, 
    uint8_t pinLeftActive, 
    int8_t pinRight, 
    uint8_t pinRightActive, 
    int8_t pinLeftSW, 
    uint8_t pinLeftSWActive, 
    int8_t pinRightSW, 
    uint8_t pinRightSWActive) {
    if (audioEntry == NULL || (pinLeft < 0 && pinRight < 0)) return NULL;
    relayInputADT relayInput = malloc(sizeof(*relayInput));
    if (relayInput == NULL) return NULL;

    relayInput->pinLeft = pinLeft;
    relayInput->pinRight = pinRight;
    relayInput->pinLeftActive = pinLeftActive;
    relayInput->pinRightActive = pinRightActive;

    relayInput->pinLeftSW = pinLeftSW;
    relayInput->pinRightSW = pinRightSW;
    relayInput->pinLeftSWActive = pinLeftSWActive;
    relayInput->pinRightSWActive = pinRightSWActive;

    audioEntry->relayInputs.add(relayInput);
}

void setAudioInput(audioEntryADT audioEntry) {
    if (audioEntry == NULL) return NULL;
    audioEntry->relayInputs.setIterator();
    while (audioEntry->relayInputs.hasNext()) {
        relayInputADT relayInput = audioEntry->relayInputs.next();
        if (relayInput->pinLeft >= 0) {
            digitalWrite(relayInput->pinLeft, relayInput->pinLeftActive);
        }
        if (relayInput->pinRight >= 0) {
            digitalWrite(relayInput->pinRight, relayInput->pinRightActive);
        }

        if (relayInput->pinLeftSW >= 0) {
            digitalWrite(relayInput->pinLeftSW, relayInput->pinLeftSWActive);
        }
        if (relayInput->pinRightSW >= 0) {
            digitalWrite(relayInput->pinRightSW, relayInput->pinRightSWActive);
        }
    }
}

uint8_t getTDAInput(audioEntryADT audioEntry) {
    if (audioEntry == NULL) return 0;
    return audioEntry->tdaInput;
}
