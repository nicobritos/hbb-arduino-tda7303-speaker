#include "TDA7303.h"
#include <Arduino.h>

#define MAX_ATTENUATION 31
#define MAX_VOLUME 63
#define MAX_TREBBLE 7
#define MAX_BASS 7
#define MAX_GAIN 3

#define MIN_ATTENUATION 0
#define MIN_VOLUME 0
#define MIN_TREBBLE -7
#define MIN_BASS -7
#define MIN_GAIN 0

#define SUB_ADD_VOLUME 0x00
#define SUB_ADD_ATTENUATION_REAR_LEFT 0xc0
#define SUB_ADD_ATTENUATION_REAR_RIGHT 0xe0
#define SUB_ADD_ATTENUATION_FRONT_LEFT 0x80
#define SUB_ADD_ATTENUATION_FRONT_RIGHT 0xa0
#define SUB_ADD_INPUT 0x40
#define SUB_ADD_BASS 0x60
#define SUB_ADD_TREBLE 0x70

#define MUTE_CODE 0x1f
#define HIGH_TREBBLE_BIT 0x08
#define HIGH_BASS_BIT 0x08
#define LOUD_OFF 0x04
#define LOUD_ON 0x00

#define LOUD_POSITION 2
#define GAIN_POSITION 3

static inline uint8_t mapMe(uint8_t value, uint8_t min, uint8_t max);


static inline uint8_t mapMe(uint8_t value, uint8_t min, uint8_t max) {
    if (value > max) return 0;
    return (max - value) + min;
}

TDA7303::TDA7303(uint8_t address) {
    amplifier->address = address;
}

void TDA7303::setTreble(int8_t value) {
    if (value >= MIN_TREBBLE && value <= MAX_TREBBLE) {
        this->treble = value;
        uint8_t data;
        if (this->treble < 0) {
            data = this->treble * -1;
            data = mapMe(data, 0, MIN_TREBBLE * -1);
        } else {
            data = mapMe(this->treble, 0, MAX_TREBBLE);
            data |= HIGH_TREBBLE_BIT;
        }
        this->send(SUB_ADD_TREBLE, data);
    }
}

void TDA7303::setBass(int8_t value) {
    if (value >= MIN_BASS && value <= MAX_BASS) {
        this->bass = value;

        uint8_t data;
        if (this->bass < 0) {
            data = this->bass * -1;
            data = mapMe(data, 0, MIN_BASS * -1);
        } else {
            data = mapMe(this->bass, 0, MAX_BASS);
            data |= HIGH_BASS_BIT;
        }
        this->send(SUB_ADD_BASS, data);
    }
}

void TDA7303::setVolumeTDA(uint8_t value) {
    if (value >= MIN_VOLUME && value <= MAX_VOLUME) {
        this->volume = value;
        this->send(SUB_ADD_VOLUME, mapMe(this->volume, MIN_VOLUME, MAX_VOLUME));
    }
}

void TDA7303::setLeft(uint8_t value) {
    if (value >= MIN_ATTENUATION && value <= MAX_ATTENUATION) {
        this->left = value;
        this->send(SUB_ADD_ATTENUATION_REAR_LEFT, mapMe(this->left, MIN_ATTENUATION, MAX_ATTENUATION));
        this->send(SUB_ADD_ATTENUATION_FRONT_LEFT, mapMe(this->left, MIN_ATTENUATION, MAX_ATTENUATION));
    }
}

void TDA7303::setRight(uint8_t value) {
    if (value >= MIN_ATTENUATION && value <= MAX_ATTENUATION) {
        this->right = value;
        this->send(SUB_ADD_ATTENUATION_REAR_RIGHT, mapMe(this->right, MIN_ATTENUATION, MAX_ATTENUATION));
        this->send(SUB_ADD_ATTENUATION_FRONT_RIGHT, mapMe(this->right, MIN_ATTENUATION, MAX_ATTENUATION));
    }
}

void TDA7303::setInput(uint8_t input) {
    if (input == TDA_INPUT_1 || input == TDA_INPUT_2 || input == TDA_INPUT_3) {
        this->input = input;
        this->update(amplifier);
    }
}

void TDA7303::setGain(uint8_t gain) {
    if (gain >= MIN_GAIN && gain <= MAX_GAIN) {
        this->gain = gain;
        this->update(amplifier);
    }
}

void TDA7303::setMute(bool value) {
    if (value && !mute) {
        mute = 1;
        this->send(SUB_ADD_ATTENUATION_REAR_LEFT, MUTE_CODE);
        this->send(SUB_ADD_ATTENUATION_REAR_RIGHT, MUTE_CODE);
        this->send(SUB_ADD_ATTENUATION_FRONT_LEFT, MUTE_CODE);
        this->send(SUB_ADD_ATTENUATION_FRONT_RIGHT, MUTE_CODE);
    } else if (!value && mute) {
        mute = 0;
        this->send(SUB_ADD_ATTENUATION_REAR_LEFT, mapMe(left, MIN_ATTENUATION, MAX_ATTENUATION));
        this->send(SUB_ADD_ATTENUATION_REAR_RIGHT, mapMe(right, MIN_ATTENUATION, MAX_ATTENUATION));
        this->send(SUB_ADD_ATTENUATION_FRONT_LEFT, mapMe(left, MIN_ATTENUATION, MAX_ATTENUATION));
        this->send(SUB_ADD_ATTENUATION_FRONT_RIGHT, mapMe(right, MIN_ATTENUATION, MAX_ATTENUATION));
    }
}

void TDA7303::setLoud(bool value) {
    if (value != loud) {
        loud = value ? 1 : 0;
        this->update();
    }
}

// Private
void TDA7303::send(uint8_t subAddress, uint8_t data) {
    Wire.beginTransmission(address);
    Wire.write(subAddress | data);
    Wire.endTransmission();
}

void TDA7303::update() {
    uint8_t data = input;
    data |= (loud? LOUD_ON : LOUD_OFF);
    data |= (mapMe(gain, MIN_GAIN, MAX_GAIN) << GAIN_POSITION);
    this->send(SUB_ADD_INPUT, data);
}
