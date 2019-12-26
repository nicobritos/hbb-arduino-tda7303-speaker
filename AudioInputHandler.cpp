#include "AudioInputHandler.h"
#include "LinkedList.h"

typedef struct relayInputCDT {
    int8_t pinLeft;
    int8_t pinRight;
    uint8_t pinLeftActive;
    uint8_t pinRightActive;
} relayInputCDT;

typedef struct audioEntryCDT {
    uint8_t tdaInput;
    LinkedList relayInputs;
} audioEntryCDT;

LinkedList audioEntries();

audioEntryADT addAudioEntry(uint8_t tdaInput) {
    audioEntryADT audioEntry = malloc(sizeof(*audioEntry));
    if (audioEntry == NULL) return NULL;

    audioEntry->tdaInput = tdaInput;
    audioEntry->relayInputs = new LinkedList();
    if (!audioEntries.add(audioEntry)) {
        delete audioEntry;
        return NULL;
    }
    return audioEntry;
}

relayInputADT registerRelay(audioEntryADT audioEntry, int8_t pinLeft, uint8_t pinLeftActive, int8_t pinRight, uint8_t pinRightActive) {
    if (audioEntry == NULL || (pinLeft < 0 && pinRight < 0)) return NULL;
    relayInputADT relayInput = malloc(sizeof(*relayInput));
    if (relayInput == NULL) return NULL;

    if (pinLeft >= 0) {
        pinMode(pinLeft, OUTPUT);
    }
    if (pinRight >= 0) {
        pinMode(pinRight, OUTPUT);
    }
    relayInput->pinLeft = pinLeft;
    relayInput->pinRight = pinRight;
    relayInput->pinLeftActive = pinLeftActive;
    relayInput->pinRightActive = pinRightActive;

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
            digitalWrite(relayInput->pinLeft, relayInput->pinRightActive);
        }
    }
}

uint8_t getTDAInput(audioEntryADT audioEntry) {
    if (audioEntry == NULL) return 0;
    return audioEntry->tdaInput;
}
