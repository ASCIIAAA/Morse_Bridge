#ifndef MORSE_H
#define MORSE_H

#include <Arduino.h>   // Needed for String type

extern const char* morseTable[36];

// Encoder
const char* getMorse(char c);

// Decoder
char decodeMorse(String morse);

// Blink + Beep function (LED + Buzzer for Morse playback)
void blinkBeepMorse(const char* code);

#endif
