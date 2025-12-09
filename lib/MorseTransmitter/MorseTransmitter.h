#ifndef MORSE_TRANSMITTER_H
#define MORSE_TRANSMITTER_H

#include <Arduino.h>

class MorseDisplay; 

// --- TIMING CONSTANTS ---
const long T_UNIT_MS = 150; 
const long DOT_DURATION_MS = T_UNIT_MS * 1;
const long DASH_DURATION_MS = T_UNIT_MS * 3;
const long ELEMENT_GAP_MS = T_UNIT_MS * 1;
const long CHAR_GAP_MS = T_UNIT_MS * 3; 
const long WORD_GAP_MS = T_UNIT_MS * 7; 

// INPUT TIMINGS
const long MAX_DOT_DURATION_MS = 280; 
const long MIN_DASH_DURATION_MS = 281;
const long DECODE_TIMEOUT_MS = 1500; 

// Button 2 Timings
const long ENTER_HOLD_TIME_MS = 1000; 

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

    // --- SECURITY CONFIGURATION ---
    bool isLocked = true;               
    const String PASSCODE = "...---..."; // SOS to Unlock
    String unlockBuffer = "";           

    // FEATURE 2: SILENT DURESS
    // Trigger: "..--" (mapped to '!')
    // This is short, distinct, and not a standard letter.
    const String DURESS_TRIGGER = "!"; 
    const String DURESS_MESSAGE = "!!! HOSTAGE ALERT - LOCATION UNKNOWN !!!";

    // FEATURE 3: SEMANTIC MACROS
    // "S1" -> "SECTOR 1 SECURE"
    
    // State Management
    int lastButtonState = HIGH; 
    unsigned long pressStartTime = 0;
    unsigned long lastActivityTime = 0; 
    String manualSequence = "";

    int lastEnterState = HIGH;
    unsigned long enterPressStartTime = 0;

    String decodedMessageBuffer = "";
    MorseDisplay *display; 

    void generateSignal(char type);
    const char* getMorseCode(char c);
    void decodeCurrentSequence();
    void checkUnlock(); 
    
    // Helper for Macros
    String expandMacro(String input);

public:
    MorseTransmitter(int btnPin, int enterBtnPin, int ledP, int buzzerP);

    void begin(MorseDisplay* displayPtr); 
    String update(); 
    void processText(const String& text);  
};

#endif