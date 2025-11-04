#ifndef AUDIO_MORSE_RECEIVER_H
#define AUDIO_MORSE_RECEIVER_H

#include <Arduino.h>

// Forward declaration of the MorseDisplay class (so the receiver can output)
class MorseDisplay; 

// --- TIMING CONSTANTS (Uses the same T_UNIT_MS for consistency) ---
const long T_UNIT_MS_RX = 200; 
const long DOT_MIN_MS = T_UNIT_MS_RX * 0.5; // Minimum pulse length for a dot
const long DOT_MAX_MS = T_UNIT_MS_RX * 2;   // Maximum pulse length for a dot

const long DASH_MIN_MS = T_UNIT_MS_RX * 2.5; // Minimum pulse length for a dash
const long CHAR_GAP_MS_RX = T_UNIT_MS_RX * 5; // Time to wait to decode a full character

// --- AUDIO DETECTION CONSTANTS ---
const int ANALOG_READ_INTERVAL_MS = 10; // How often to sample the microphone
const int NOISE_FLOOR = 30;             // Minimum change in reading to confirm signal (out of 1024)
const int DECODE_THRESHOLD = 200;       // Analog value threshold for sound detection (adjust based on mic/noise)

// --- CLASS DEFINITION ---
class AudioMorseReceiver {
public:
    // Structure for the Morse Code Lookup Table (Duplicated for decoding locally)
    struct MorseCodeEntry {
        char character;
        const char* sequence;
    };

private:
    int audioPin;
    MorseDisplay* display;
    
    // State Management for Pulse Detection
    bool isTonePresent = false;
    unsigned long toneStartTime = 0;
    unsigned long gapStartTime = 0;
    unsigned long lastUpdateTime = 0;
    
    String receivedSequence = "";
    
    // Internal Helpers
    bool isSignalDetected();
    void decodeCurrentSequence();
    const char* getDecodedChar(const String& sequence);

public:
    // Constructor
    AudioMorseReceiver(int audioPin);

    // Core Functions
    void begin(MorseDisplay* displayPtr);
    void update();
};

#endif // AUDIO_MORSE_RECEIVER_H
