#ifndef _InputHandler_H_
#define _InputHandler_H_

#include "Arduino.h"

class InputHandler {
  private:
    uint8_t activePinsCount;
    uint8_t currentLenght;
    uint8_t * activeTypes;
    uint8_t * activePins;
    uint8_t * pins;
    uint8_t count;

    struct encoderCDT * encoders;
    struct encoderCDT * activeEncoders;
    uint8_t activeEncoderCount;
    uint8_t encoderCount;
    uint8_t currentEncoderLength;

  public:
    InputHandler();
    
    void registerPin(uint8_t pin, uint8_t activeType, bool internalPullup);
    void registerPin(uint8_t pin, uint8_t activeType);
    void registerEncoder(uint8_t pin1, uint8_t pin2, uint8_t code, uint8_t limit, bool internalPullup1, bool internalPullup2);
    void registerEncoder(uint8_t pin1, uint8_t pin2, uint8_t code, uint8_t limit);

    uint8_t getActivePinsCount();
    uint8_t getActiveEncoderCount();
    const uint8_t * readInputs();
    const uint8_t * readEncoders();
    int8_t getEncoderDirection(encoderCDT* encoder);
    uint8_t getEncoderCode(encoderCDT* encoder);
};
#endif