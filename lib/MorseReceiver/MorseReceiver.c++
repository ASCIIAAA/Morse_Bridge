#include "MorseReceiver.h"
#include "MorseDisplay.h" // FIX: Added full class definition
#include <ctype.h>

// --- MORSE CODE LOOKUP TABLE (Required for decoding the pulses) ---
const AudioMorseReceiver::MorseCodeEntry morseCodeTable[] = {
    // Letters 
    {'A', ".-"},    {'B', "-..."},  {'C', "-.-."},  {'D', "-.."},   {'E', "."},
    {'F', "..-."},  {'G', "--."},   {'H', "...."},  {'I', ".."},    {'J', ".---"},
    {'K', "-.-"},   {'L', ".-.."},  {'M', "--"},    {'N', "-."},    {'O', "---"},
    {'P', ".--."},  {'Q', "--.-"},  {'R', ".-."},   {'S', "..."},   {'T', "-"},
    {'U', "..-"},   {'V', "...-"},  {'W', ".--"},   {'X', "-..-"},  {'Y', "-.--"},
    {'Z', "--.."},

    // Numbers
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
    {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},

    // Space
    {' ', " "}
};
const int MORSE_TABLE_SIZE_RX = sizeof(morseCodeTable) / sizeof(morseCodeTable[0]);

// --- Constructor (FIXED: Use 'display' member variable name) ---
AudioMorseReceiver::AudioMorseReceiver(int audioPin) 
    : audioPin(audioPin), display(nullptr) {}

// --- Initialization (FIXED: Use 'display' member variable) ---
void AudioMorseReceiver::begin(MorseDisplay* displayPtr) {
    display = displayPtr;
    pinMode(audioPin, INPUT);
    Serial.println(F("[RX] Audio Receiver Initialized on pin A0."));
    if (display) {
        display->setStatus(F("RX Mode Ready"));
    }
}

// --- Audio Detection Logic (Unchanged) ---
bool AudioMorseReceiver::isSignalDetected() {
    // Read the current sensor value
    int value = analogRead(audioPin);

    // Simple Threshold Detection (Adjust DECODE_THRESHOLD based on testing)
    if (value > DECODE_THRESHOLD) {
        // Signal detected
        return true;
    }
    
    // No signal detected
    return false;
}

// --- Finds decoded character from sequence (Unchanged) ---
const char* AudioMorseReceiver::getDecodedChar(const String& sequence) {
    for (int i = 0; i < MORSE_TABLE_SIZE_RX; ++i) {
        if (sequence.equals(morseCodeTable[i].sequence)) {
            return &morseCodeTable[i].character; // Return pointer to the character
        }
    }
    return nullptr;
}

// --- Decodes the current received sequence (FIXED: Use 'display' member variable) ---
void AudioMorseReceiver::decodeCurrentSequence() {
    if (receivedSequence.length() == 0) return;

    const char* decodedCharPtr = getDecodedChar(receivedSequence);
    char decodedChar = (decodedCharPtr) ? *decodedCharPtr : '?'; // Use '?' for unknown
    
    Serial.print(F("[RX DECODE] Seq: "));
    Serial.print(receivedSequence);
    Serial.print(F(" -> Char: "));
    Serial.println(decodedChar);

    if (display) {
        // FIX: The String concatenation F("Decoded: ") + receivedSequence needs the String() cast if used this way
        String status = String(F("Decoded: ")) + receivedSequence; 
        display->setStatus(status);
        delay(100);
        display->appendDecodedCharacter(decodedChar);
    }
    
    // Reset sequence after decoding
    receivedSequence = "";
}

// --- Main Update Loop (Timing and Classification) (FIXED: Use 'display' member variable) ---
void AudioMorseReceiver::update() {
    unsigned long currentTime = millis();
    
    // Part 1: Sample the input periodically
    if (currentTime - lastUpdateTime < ANALOG_READ_INTERVAL_MS) {
        return;
    }
    lastUpdateTime = currentTime;
    
    bool signalNow = isSignalDetected();

    // --- State Transition: Tone START ---
    if (signalNow && !isTonePresent) {
        // Tone just started
        isTonePresent = true;
        toneStartTime = currentTime;
        gapStartTime = 0; // Reset gap timer

        Serial.println(F("Tone ON!"));
        // FIXED: Use 'display'
        if (display) display->updateInputSequence(receivedSequence + "!"); // Visual cue
    }
    
    // --- State Transition: Tone END (Pulse classification) ---
    else if (!signalNow && isTonePresent) {
        // Tone just ended. Classify the pulse duration.
        isTonePresent = false;
        unsigned long pulseDuration = currentTime - toneStartTime;

        char signalType = 0;

        if (pulseDuration >= DASH_MIN_MS) {
            signalType = '-'; // Dash
        } else if (pulseDuration >= DOT_MIN_MS && pulseDuration <= DOT_MAX_MS) {
            signalType = '.'; // Dot
        }
        
        if (signalType) {
            receivedSequence += signalType;
            Serial.print(F("[RX PULSE] Duration: "));
            Serial.print(pulseDuration);
            Serial.print(F("ms -> "));
            Serial.println(signalType);
        } else {
             Serial.print(F("[RX PULSE] Duration: "));
             Serial.print(pulseDuration);
             Serial.println(F("ms -> UNCLASSIFIED (Ignored)"));
        }
        
        gapStartTime = currentTime; // Start the gap timer
        // FIXED: Use 'display'
        if (display) display->updateInputSequence(receivedSequence);
    }
    
    // --- Part 2: Character Gap Timeout Check ---
    else if (!signalNow && gapStartTime > 0) {
        // No tone, and we are currently in a gap
        unsigned long gapDuration = currentTime - gapStartTime;
        
        if (gapDuration >= CHAR_GAP_MS_RX) {
            // Gap is long enough to signify the end of a character
            decodeCurrentSequence();
            gapStartTime = 0; // Reset gap timer
            // FIXED: Use 'display'
            if (display) display->setStatus(F("RX Char Decoded"));
        }
    }
}
