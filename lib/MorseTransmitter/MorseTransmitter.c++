#include "MorseTransmitter.h"
#include "MorseDisplay.h" // NEW: Include the new display component
#include <Arduino.h>
#include <ctype.h> 

// --- MORSE CODE LOOKUP TABLE --- 
const MorseTransmitter::MorseCodeEntry morseCodeTable[] = {
  // Letters
{'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},  {'E', "."},
{'F', "..-."}, {'G', "--."},  {'H', "...."}, {'I', ".."},  {'J', ".---"},
{'K', "-.-"},  {'L', ".-.."}, {'M', "--"},  {'N', "-."},  {'O', "---"},
{'P', ".--."}, {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
{'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"}, {'Y', "-.--"},
{'Z', "--.."},

  // Numbers
  {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
  {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},

  // Space
  {' ', " "}
};
const int MORSE_TABLE_SIZE = sizeof(morseCodeTable) / sizeof(morseCodeTable[0]);

// --- Constructor (UPDATED: Initialize display pointer) ---
MorseTransmitter::MorseTransmitter(int btnPin, int ledP, int buzzerP)
  : btnPin(btnPin), ledPin(ledP), buzzerPin(buzzerP), display(nullptr) {}

// --- Begin Function (UPDATED SIGNATURE) ---
void MorseTransmitter::begin(MorseDisplay* displayPtr) {
    display = displayPtr; // Store the pointer to the external display object
    
    // Display initialization handled by the MorseDisplay component
    if (display) {
        display->begin(); 
    }

  // Pin setup
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
  Serial.println(F("--- MORSE BRIDGE V1.1 INITIALIZED (Modular Display) ---"));
  Serial.println(F("Ready."));
}

// --- Signal Generation, getMorseCode (Unchanged) ---
void MorseTransmitter::generateSignal(char type) {
  long duration;
  if (type == '.') { duration = DOT_DURATION_MS; } 
  else if (type == '-') { duration = DASH_DURATION_MS; } 
  else { return; }
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzerPin, HIGH);
  delay(duration);
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
  delay(ELEMENT_GAP_MS);
}

const char* MorseTransmitter::getMorseCode(char c) {
  c = toupper(c); 
  for (int i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (morseCodeTable[i].character == c) { return morseCodeTable[i].sequence; }
  }
  return nullptr;
}

// --- Encodes and transmits a full string via Serial Input (UPDATED: Uses display) ---
void MorseTransmitter::processText(const String& text) {
  Serial.print(F("[TX] Encoding and transmitting: '"));
  Serial.print(text);
  Serial.println(F("'"));
    
    if (display) {
        display->clearAll();
        display->setStatus(F("TX Mode..."));
    }

  for (unsigned int i = 0; i < text.length(); ++i) {
    char c = text[i];
    const char* morseSeq = getMorseCode(c);

    if (morseSeq) {
            
            if (display) {
                 display->appendDecodedCharacter(c);
                 String txStatus = F("TX: ");
                 txStatus += morseSeq;
                 display->setStatus(txStatus);
            }
      
      for (int j = 0; morseSeq[j] != '\0'; ++j) {
        generateSignal(morseSeq[j]);
      }

      if (c != ' ') { delay(CHAR_GAP_MS - ELEMENT_GAP_MS); } 
      else { delay(WORD_GAP_MS - ELEMENT_GAP_MS); }
      
      Serial.print(F(" [")); Serial.print(c); Serial.println(F("] transmitted."));
    } else {
      Serial.print(F(" [")); Serial.print(c); Serial.println(F("] (Character not found/skipped)"));
    }
  }
  Serial.println(F("[TX] Transmission Complete."));
    if (display) {
        delay(1000);
        display->setStatus(F("Complete. Ready!"));
    }
}

// --- Decodes the current manual sequence (UPDATED: Uses display) ---
void MorseTransmitter::decodeCurrentSequence() {
  if (manualSequence.length() == 0) return;

    if (display) {
        String seqStatus = String(F("Dec: ")) + manualSequence;
        display->setStatus(seqStatus);
    }

  for (int i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (manualSequence.equals(morseCodeTable[i].sequence)) {
      char decodedChar = morseCodeTable[i].character;
      
      Serial.print(F("[DECODE] Sequence: \""));
      Serial.print(manualSequence);
      Serial.print(F("\" -> Decoded Character: '"));
      Serial.print(decodedChar);
      Serial.println(F("'"));
      
            if (display) {
                display->appendDecodedCharacter(decodedChar);
            }
      manualSequence = "";
      return;
    }
  }

  Serial.print(F("[DECODE] Sequence: \""));
  Serial.print(manualSequence);
  Serial.println(F("\" -> ERROR: Unknown sequence."));
    if (display) {
        display->setStatus(F("Sequence ERROR"));
    }
  manualSequence = "";
}

// --- Main update function (Button Input and Decoder Trigger) (UPDATED: Uses display) ---
void MorseTransmitter::update() {
  int reading = digitalRead(btnPin);
  unsigned long currentTime = millis();

  // --- Part 1: Button Press Detection ---
  if (reading == LOW && lastButtonState == HIGH) {
    pressStartTime = currentTime;
  } 
  
  // --- Part 2: Button Release Detection ---
  else if (reading == HIGH && lastButtonState == LOW) {
    unsigned long pressDuration = currentTime - pressStartTime;
    char signalType = 0;
    
    if (pressDuration >= MIN_DASH_DURATION_MS) {
      signalType = '-';
    } else if (pressDuration >= MIN_DOT_DURATION_MS && pressDuration <= MAX_DOT_DURATION_MS) {
      signalType = '.';
    }
    
    if (signalType) {
      
      Serial.print(F("[INPUT]... "));
      manualSequence += signalType;
      generateSignal(signalType); 
      lastActivityTime = currentTime; 

      Serial.print(F(" Sequence: "));
      Serial.println(manualSequence);
            
            if (display) {
                display->updateInputSequence(manualSequence);
            }
    } else if (pressDuration > 0) {
      Serial.print(F("[INPUT]... Unrecognized (Ignored)"));
    }
  }

  // --- Part 3: Character Gap Detection (Timeout) ---
  if (reading == HIGH && manualSequence.length() > 0 && lastActivityTime > 0) {
    unsigned long timeSinceLastActivity = currentTime - lastActivityTime;

    if (timeSinceLastActivity >= DECODE_TIMEOUT_MS) {
      decodeCurrentSequence();
      lastActivityTime = 0; 
    }
  }

  lastButtonState = reading;
}
