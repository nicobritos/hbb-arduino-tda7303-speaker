#ifndef TDA7303_H
    
    #include <Wire.h>
    #define TDA7303_H

    class TDA7303 {
    private:
        uint8_t mute;
        uint8_t volume;
        uint8_t left;
        uint8_t right;
        int8_t bass;
        int8_t treble;
        uint8_t input;
        uint8_t mutePin;
        uint8_t address;
        uint8_t gain;
        uint8_t loud;

        void send(uint8_t subAddress, uint8_t data);
        void update();

    public:
        TDA7303(uint8_t address);

        void setTreble(int8_t value);
        void setBass(int8_t value);
        void setVolume(uint8_t value);
        void setLeft(uint8_t value);
        void setRight(uint8_t value);
        void setInput(uint8_t input);
        void setGain(uint8_t gain);
        void setMute(bool value);
        void setLoud(bool value);
    };

#endif // TDA7303_H
