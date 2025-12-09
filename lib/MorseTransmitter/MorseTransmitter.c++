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

// --- Updated Constructor ---
MorseTransmitter::MorseTransmitter(int btnPin, int enterBtnPin, int ledP, int buzzerP)
  : btnPin(btnPin), enterBtnPin(enterBtnPin), ledPin(ledP), buzzerPin(buzzerP), display(nullptr) {}

void MorseTransmitter::begin(MorseDisplay* displayPtr) {
    display = displayPtr; 
    if (display) display->begin(); 

    pinMode(btnPin, INPUT_PULLUP);
    
    // Setup Enter Button
    if (enterBtnPin != -1) {
        pinMode(enterBtnPin, INPUT_PULLUP);
    }

    pinMode(ledPin, OUTPUT);
    if (buzzerPin != -1) pinMode(buzzerPin, OUTPUT);
    
    Serial.begin(9600);
    Serial.println(F("--- MORSE BRIDGE V1.2 (Buffered Input) ---"));
}

void MorseTransmitter::generateSignal(char type) {
  long duration;
  if (type == '.') duration = DOT_DURATION_MS; 
  else if (type == '-') duration = DASH_DURATION_MS; 
  else return;

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
  Serial.print(F("[TX] Sending: '")); Serial.print(text); Serial.println(F("'"));
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
             String txStatus = F("TX: "); txStatus += morseSeq;
             display->setStatus(txStatus);
        }
        for (int j = 0; morseSeq[j] != '\0'; ++j) {
            generateSignal(morseSeq[j]);
        }
        if (c != ' ') delay(CHAR_GAP_MS - ELEMENT_GAP_MS); 
        else delay(WORD_GAP_MS - ELEMENT_GAP_MS);
    }
  }
  if (display) {
      delay(1000);
      display->setStatus(F("Ready"));
  }
}

void MorseTransmitter::decodeCurrentSequence() {
  if (manualSequence.length() == 0) return;

  // Search Table
  for (int i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (manualSequence.equals(morseCodeTable[i].sequence)) {
      char decodedChar = morseCodeTable[i].character;
      
      if (display) display->appendDecodedCharacter(decodedChar);
      decodedMessageBuffer += decodedChar;
      
      manualSequence = "";
      // Update display with "Building" status
      if (display) display->setStatus(String(F("Msg: ")) + decodedMessageBuffer);
      return;
    }
  }
  // Error
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
    
    // Ignore extremely short glitches (<20ms)
    if (pressDuration > 20) {
        if (pressDuration >= MIN_DASH_DURATION_MS) signalType = '-';
        else if (pressDuration >= MIN_DOT_DURATION_MS) signalType = '.';
        
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
          enterPressStartTime = currentTime; // Enter Pressed
      }
      else if (enterReading == HIGH && lastEnterState == LOW) { // Enter Released
          unsigned long enterDuration = currentTime - enterPressStartTime;

          if (enterDuration >= ENTER_HOLD_TIME_MS) {
              // --- LONG PRESS: SEND MESSAGE ---
              if (decodedMessageBuffer.length() > 0) {
                  messageToSend = decodedMessageBuffer;
                  decodedMessageBuffer = ""; // Clear
                  if (display) {
                      display->setStatus(F("Sending..."));
                      delay(500);
                      display->clearAll();
                  }
              }
          } else if (enterDuration > 50) {
              // --- SHORT PRESS: ADD SPACE ---
              decodedMessageBuffer += ' ';
              if (display) {
                  display->appendDecodedCharacter('_'); // Show underscore for space
                  display->setStatus(F("Space Added"));
              }
          }
      }
      lastEnterState = enterReading;
  }

  return messageToSend;
}