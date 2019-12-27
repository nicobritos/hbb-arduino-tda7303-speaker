# TDA7303 amplifier control with Arduino Nano
Speaker using an Atmega328p with a TDA7303 chip, relays, a 16x2 LCD screen, some buttons and a volume knob.

# Materials:
* Arduino Nano with an Atmega 328p (old bootloader).
* Edifier C200 speaker system (which includes a TDA7303 as an amplifier).
* 16x2 LCD Screen (parallel interface with 6 cables) with LED backlight.
* RC circuit in order to smooth out the LED when it's controlled via PWM.
* 3 buttons: menu select, input select and reset (wired directly to the Arduino Nano's reset circuit).
* 8 relays in order to switch inputs and subwoofer output.
* 1 rotary encoder with button which serves as a volume knob and toggles mute.
