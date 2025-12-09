#include "MorseTransmitter.h"
#include "MorseDisplay.h" 
#include <Arduino.h>
#include <ctype.h> 

const MorseTransmitter::MorseCodeEntry morseCodeTable[] = {
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

// --- Constructor ---
MorseTransmitter::MorseTransmitter(int btnPin, int enterBtnPin, int ledP, int buzzerP)
  : btnPin(btnPin), enterBtnPin(enterBtnPin), ledPin(ledP), buzzerPin(buzzerP), display(nullptr) {}

void MorseTransmitter::begin(MorseDisplay* displayPtr) {
    display = displayPtr; 
    if (display) display->begin(); 

    pinMode(btnPin, INPUT_PULLUP);
    if (enterBtnPin != -1) pinMode(enterBtnPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    if (buzzerPin != -1) pinMode(buzzerPin, OUTPUT);
    
    // Ensure Serial is on for debugging
    if (!Serial) Serial.begin(9600);
}

void MorseTransmitter::generateSignal(char type) {
  long duration;
  if (type == '.') duration = DOT_DURATION_MS; 
  else if (type == '-') duration = DASH_DURATION_MS; 
  else return;

  // VISUAL DEBUG: Print the dot/dash being played
  Serial.print(type); 

  digitalWrite(ledPin, HIGH);
  if (buzzerPin != -1) digitalWrite(buzzerPin, HIGH);
  delay(duration);
  digitalWrite(ledPin, LOW);
  if (buzzerPin != -1) digitalWrite(buzzerPin, LOW);
  delay(ELEMENT_GAP_MS);
}

const char* MorseTransmitter::getMorseCode(char c) {
  c = toupper(c); 
  for (int i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (morseCodeTable[i].character == c) return morseCodeTable[i].sequence;
  }
  return nullptr;
}

void MorseTransmitter::processText(const String& text) {
  Serial.print(F("[TX] Playing: '")); 
  Serial.print(text); 
  Serial.print(F("' -> "));

  if (display) {
      display->clearAll();
      display->setStatus(F("RX Mode..."));
  }
  
  for (unsigned int i = 0; i < text.length(); ++i) {
    char c = text[i];
    const char* morseSeq = getMorseCode(c);
    if (morseSeq) {
        if (display) {
             display->appendDecodedCharacter(c);
             String txStatus = F("RX: "); txStatus += morseSeq;
             display->setStatus(txStatus);
        }
        
        // Play the sequence
        for (int j = 0; morseSeq[j] != '\0'; ++j) {
            generateSignal(morseSeq[j]); // This now prints dots/dashes too
        }
        
        Serial.print(" "); // Space between letters in Serial Monitor

        if (c != ' ') delay(CHAR_GAP_MS - ELEMENT_GAP_MS); 
        else delay(WORD_GAP_MS - ELEMENT_GAP_MS);
    }
  }
  Serial.println(F(" [DONE]"));
  
  if (display) {
      delay(1000);
      display->setStatus(F("Ready"));
  }
}

void MorseTransmitter::decodeCurrentSequence() {
  if (manualSequence.length() == 0) return;

  Serial.print(F(" -> Seq: "));
  Serial.print(manualSequence);

  for (int i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (manualSequence.equals(morseCodeTable[i].sequence)) {
      char decodedChar = morseCodeTable[i].character;
      
      Serial.print(F(" -> Char: "));
      Serial.println(decodedChar);
      
      if (display) display->appendDecodedCharacter(decodedChar);
      decodedMessageBuffer += decodedChar;
      
      manualSequence = "";
      if (display) display->setStatus(String(F("Msg: ")) + decodedMessageBuffer);
      return;
    }
  }
  Serial.println(F(" -> UNKNOWN"));
  if (display) display->setStatus(F("Unknown Char"));
  manualSequence = "";
}

// --- MAIN LOOP ---
String MorseTransmitter::update() {
  unsigned long currentTime = millis();
  String messageToSend = ""; 

  // --- 1. HANDLE MORSE BUTTON (Pin 2) ---
  int reading = digitalRead(btnPin);

  if (reading == LOW && lastButtonState == HIGH) {
    pressStartTime = currentTime; // Button Pressed
  } 
  else if (reading == HIGH && lastButtonState == LOW) { // Button Released
    unsigned long pressDuration = currentTime - pressStartTime;
    char signalType = 0;
    
    // INCREASED DEBOUNCE: Ignore anything less than 50ms
    if (pressDuration > 50) {
        // LOGIC FIX: Removed "Dead Zone" between 400ms and 500ms
        // < 400ms = DOT
        // > 400ms = DASH
        if (pressDuration <= MAX_DOT_DURATION_MS) {
            signalType = '.';
        } else {
            signalType = '-';
        }
        
        // DEBUG PRINT: Tell user exactly how long they pressed
        Serial.print(F("[DEBUG] "));
        Serial.print(pressDuration);
        Serial.print(F("ms -> "));
        Serial.println(signalType);
        
        if (signalType) {
          manualSequence += signalType;
          generateSignal(signalType); 
          lastActivityTime = currentTime; 
          if (display) display->updateInputSequence(manualSequence);
        }
    }
  }
  lastButtonState = reading;

  // Auto-decode character after timeout (Char Gap)
  if (reading == HIGH && manualSequence.length() > 0 && lastActivityTime > 0) {
    if (currentTime - lastActivityTime >= DECODE_TIMEOUT_MS) {
      decodeCurrentSequence(); 
      lastActivityTime = 0; 
    }
  }

  // --- 2. HANDLE ENTER/SPACE BUTTON (Pin 3) ---
  if (enterBtnPin != -1) {
      int enterReading = digitalRead(enterBtnPin);

      if (enterReading == LOW && lastEnterState == HIGH) {
          enterPressStartTime = currentTime; 
      }
      else if (enterReading == HIGH && lastEnterState == LOW) { 
          unsigned long enterDuration = currentTime - enterPressStartTime;

          // Ignore short glitches
          if (enterDuration > 50) {
              if (enterDuration >= 1000) { 
                  // --- LONG PRESS (>1s): SEND MESSAGE ---
                  if (decodedMessageBuffer.length() > 0) {
                      messageToSend = decodedMessageBuffer;
                      decodedMessageBuffer = ""; 
                      if (display) {
                          display->setStatus(F("Sending..."));
                          delay(500);
                          display->clearAll();
                      }
                  }
              } else {
                  // --- SHORT PRESS: ADD SPACE ---
                  decodedMessageBuffer += ' ';
                  Serial.println(F("[INPUT] Space Added"));
                  if (display) {
                      display->appendDecodedCharacter('_'); 
                      display->setStatus(F("Space Added"));
                  }
              }
          }
      }
      lastEnterState = enterReading;
  }

  return messageToSend;
}