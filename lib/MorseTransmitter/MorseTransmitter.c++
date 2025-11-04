#include "MorseTransmitter.h"
#include "MorseDisplay.h" 
#include <Arduino.h>
#include <ctype.h> 

// --- MORSE CODE LOOKUP TABLE (Unchanged) --- 
const MorseTransmitter::MorseCodeEntry morseCodeTable[] = {
  // ... (table is identical) ...
  {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},  {'E', "."},
  {'F', "..-."}, {'G', "--."},  {'H', "...."}, {'I', ".."},  {'J', ".---"},
  {'K', "-.-"},  {'L', ".-.."}, {'M', "--"},  {'N', "-."},  {'O', "---"},
  {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
  {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"}, {'Y', "-.--"},
  {'Z', "--.."},
  {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
  {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
  {' ', " "}
};
const int MORSE_TABLE_SIZE = sizeof(morseCodeTable) / sizeof(morseCodeTable[0]);

// --- Constructor (UPDATED: Initialize display pointer) ---
MorseTransmitter::MorseTransmitter(int btnPin, int ledP, int buzzerP)
  : btnPin(btnPin), ledPin(ledP), buzzerPin(buzzerP), display(nullptr) {}

// --- Begin Function (UPDATED SIGNATURE) ---
void MorseTransmitter::begin(MorseDisplay* displayPtr) {
    // ... (This function is unchanged from your uploaded file) ...
    display = displayPtr; 
    if (display) {
        display->begin(); 
    }
    pinMode(btnPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    // *** NEW: Check for -1 to disable buzzer ***
    if (buzzerPin != -1) {
        pinMode(buzzerPin, OUTPUT);
    }
    Serial.begin(9600);
    Serial.println(F("--- MORSE BRIDGE V1.1 INITIALIZED (Modular Display) ---"));
}

// --- Signal Generation, getMorseCode (Unchanged) ---
void MorseTransmitter::generateSignal(char type) {
  long duration;
  if (type == '.') { duration = DOT_DURATION_MS; } 
  else if (type == '-') { duration = DASH_DURATION_MS; } 
  else { return; }
  digitalWrite(ledPin, HIGH);
  // *** NEW: Check for -1 ***
  if (buzzerPin != -1) {
      digitalWrite(buzzerPin, HIGH);
  }
  delay(duration);
  digitalWrite(ledPin, LOW);
  if (buzzerPin != -1) {
      digitalWrite(buzzerPin, LOW);
  }
  delay(ELEMENT_GAP_MS);
}

const char* MorseTransmitter::getMorseCode(char c) {
  // ... (This function is unchanged) ...
  c = toupper(c); 
  for (int i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (morseCodeTable[i].character == c) { return morseCodeTable[i].sequence; }
  }
  return nullptr;
}

// --- processText (Unchanged) ---
void MorseTransmitter::processText(const String& text) {
  // ... (This function is unchanged from your uploaded file) ...
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

// --- Decodes the current manual sequence (MODIFIED) ---
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
      
            // *** NEW: Add to the message buffer ***
            decodedMessageBuffer += decodedChar;

      manualSequence = "";
      return;
    }
  }

  // ... (Error handling is unchanged) ...
  Serial.print(F("[DECODE] Sequence: \""));
  Serial.print(manualSequence);
  Serial.println(F("\" -> ERROR: Unknown sequence."));
    if (display) {
        display->setStatus(F("Sequence ERROR"));
    }
  manualSequence = "";
}

// --- Main update function (MODIFIED) ---
String MorseTransmitter::update() {
  int reading = digitalRead(btnPin);
  unsigned long currentTime = millis();
  String messageToSend = ""; // Our return value

  // --- Part 1 & 2: Button Press/Release (Unchanged) ---
  if (reading == LOW && lastButtonState == HIGH) {
    pressStartTime = currentTime;
  } 
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
      decodeCurrentSequence(); // This decodes the char and adds to buffer
      lastActivityTime = 0; 
    }
  }

  // *** NEW: Part 4: Message Gap Detection (Timeout) ***
  if (reading == HIGH && decodedMessageBuffer.length() > 0 && lastActivityTime == 0) {
      // We check for lastActivityTime == 0 to ensure we're between characters
      unsigned long timeSinceLastChar = currentTime - pressStartTime;
      
      // We check 'pressStartTime' because it holds the time of the last release
      if (timeSinceLastChar >= MESSAGE_TIMEOUT_MS) {
          Serial.print(F("[SEND] Message complete: "));
          Serial.println(decodedMessageBuffer);

          messageToSend = decodedMessageBuffer;
          decodedMessageBuffer = ""; // Clear the buffer
          
          if (display) {
              display->setStatus(F("Message Sent!"));
              delay(1000);
              display->clearAll(); // Clear display for next message
          }
      }
  }

  lastButtonState = reading;
  return messageToSend; // Return the message (or "" if not ready)
}