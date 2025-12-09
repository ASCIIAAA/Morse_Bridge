#ifndef MORSE_TRANSMITTER_H
#define MORSE_TRANSMITTER_H

#include <Arduino.h>

class MorseDisplay; 

// --- TIMING CONSTANTS ---
const long T_UNIT_MS = 200; 
const long DOT_DURATION_MS = T_UNIT_MS * 1;
const long DASH_DURATION_MS = T_UNIT_MS * 3;
const long ELEMENT_GAP_MS = T_UNIT_MS * 1;
const long CHAR_GAP_MS = T_UNIT_MS * 3; 
const long WORD_GAP_MS = T_UNIT_MS * 7; 

// Input Timings
const long MIN_DOT_DURATION_MS = 50;
const long MAX_DOT_DURATION_MS = T_UNIT_MS * 2;
const long MIN_DASH_DURATION_MS = T_UNIT_MS * 2.5;
const long DECODE_TIMEOUT_MS = 700; 

// Button 2 Timings
const long ENTER_HOLD_TIME_MS = 1000; // Hold for 1s to SEND

class MorseTransmitter {
public:
    struct MorseCodeEntry {
        char character;
        const char* sequence;
    };

private:
    int btnPin;
    int enterBtnPin; // NEW: Second button
    int ledPin;
    int buzzerPin;

    // State Management for Morse Input
    int lastButtonState = HIGH; 
    unsigned long pressStartTime = 0;
    unsigned long lastActivityTime = 0; 
    String manualSequence = "";

    // State Management for Enter Button
    int lastEnterState = HIGH;
    unsigned long enterPressStartTime = 0;

    String decodedMessageBuffer = "";
    MorseDisplay *display; 

    void generateSignal(char type);
    const char* getMorseCode(char c);
    void decodeCurrentSequence();

public:
    // Updated Constructor
    MorseTransmitter(int btnPin, int enterBtnPin, int ledP, int buzzerP);

    void begin(MorseDisplay* displayPtr); 
    String update(); 
    void processText(const String& text);  
};

#endif