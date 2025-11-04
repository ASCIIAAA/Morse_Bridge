#ifndef MORSE_TRANSMITTER_H
#define MORSE_TRANSMITTER_H

#include <Arduino.h>

// Forward declaration of the MorseDisplay class
class MorseDisplay; 

// --- TIMING CONSTANTS ---
const long T_UNIT_MS = 200; 

// Output Timings
const long DOT_DURATION_MS = T_UNIT_MS * 1;
const long DASH_DURATION_MS = T_UNIT_MS * 3;

// Gap Timings
const long ELEMENT_GAP_MS = T_UNIT_MS * 1;
const long CHAR_GAP_MS = T_UNIT_MS * 3; 
const long WORD_GAP_MS = T_UNIT_MS * 7; 

// Input Timings
const long MIN_DOT_DURATION_MS = 50;
const long MAX_DOT_DURATION_MS = T_UNIT_MS * 2;
const long MIN_DASH_DURATION_MS = T_UNIT_MS * 2.5;
const long DECODE_TIMEOUT_MS = 700; // Time since last button release to trigger CHAR decode

// *** NEW: Time to wait before "sending" the full message ***
const long MESSAGE_TIMEOUT_MS = 2000; // 2 seconds of inactivity = message complete

// --- CLASS DEFINITION ---
class MorseTransmitter {
public:
    struct MorseCodeEntry {
    char character;
    const char* sequence;};

private:
    int btnPin;
    int ledPin;
    int buzzerPin;

    // State Management for Manual Input
    int lastButtonState = HIGH; 
    unsigned long pressStartTime = 0;
    unsigned long lastActivityTime = 0; 
    String manualSequence = "";

    // *** NEW: Buffer for the outgoing message ***
    String decodedMessageBuffer = "";

    // Pointer to the external display object
    MorseDisplay *display; 

    // Internal Helper Functions
    void generateSignal(char type);
    const char* getMorseCode(char c);
    void decodeCurrentSequence();

public:
    // Constructor
    MorseTransmitter(int btnPin, int ledP, int buzzerP);

    // Core Functions
    void begin(MorseDisplay* displayPtr); 
    
    // *** MODIFIED: update() now returns a String ***
    String update(); 

    void processText(const String& text);  
};

#endif // MORSE_TRANSMITTER_H