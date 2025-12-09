#ifndef MORSE_TRANSMITTER_H
#define MORSE_TRANSMITTER_H

#include <Arduino.h>

class MorseDisplay; 

// --- TIMING CONSTANTS (CALIBRATED FOR HUMAN INPUT) ---
// We removed the strict "Time Unit" math to make it more forgiving.

// OUTPUT TIMINGS (How fast the machine blinks/beeps at you)
// These don't affect your button pressing, only how it plays back.
const long T_UNIT_MS = 150; 
const long DOT_DURATION_MS = T_UNIT_MS * 1;
const long DASH_DURATION_MS = T_UNIT_MS * 3;
const long ELEMENT_GAP_MS = T_UNIT_MS * 1;
const long CHAR_GAP_MS = T_UNIT_MS * 3; 
const long WORD_GAP_MS = T_UNIT_MS * 7; 

// INPUT TIMINGS (The "Split Point")
// Your logs showed your Dots are ~100-220ms and Dashes are ~320-460ms.
// We set the "Split Point" at 280ms.
// < 280ms = DOT
// > 280ms = DASH
const long MAX_DOT_DURATION_MS = 280; 
const long MIN_DASH_DURATION_MS = 281; // Not strictly used in new logic, but kept for reference

// TIMEOUTS (How long the code waits for you)
// Increased from 700ms to 1500ms to stop letters from breaking apart.
const long DECODE_TIMEOUT_MS = 1500; 

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
    int enterBtnPin; 
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
    MorseTransmitter(int btnPin, int enterBtnPin, int ledP, int buzzerP);

    void begin(MorseDisplay* displayPtr); 
    String update(); 
    void processText(const String& text);  
};

#endif