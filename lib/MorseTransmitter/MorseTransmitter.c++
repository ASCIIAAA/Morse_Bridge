#include "MorseTransmitter.h"
#include "MorseDisplay.h" 
#include <Arduino.h>
#include <ctype.h> 

// --- UPDATED MORSE TABLE ---
// We added '!' mapped to "..--" for your Duress Code
const MorseTransmitter::MorseCodeEntry morseCodeTable[] = {
  {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},  {'E', "."},
  {'F', "..-."}, {'G', "--."},  {'H', "...."}, {'I', ".."},  {'J', ".---"},
  {'K', "-.-"},  {'L', ".-.."}, {'M', "--"},  {'N', "-."},  {'O', "---"},
  {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
  {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"}, {'Y', "-.--"},
  {'Z', "--.."},
  {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
  {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
  {' ', " "},
  {'!', "..--"} // <--- NEW: The Duress Code (Not a standard letter)
};
const int MORSE_TABLE_SIZE = sizeof(morseCodeTable) / sizeof(morseCodeTable[0]);

MorseTransmitter::MorseTransmitter(int btnPin, int enterBtnPin, int ledP, int buzzerP)
  : btnPin(btnPin), enterBtnPin(enterBtnPin), ledPin(ledP), buzzerPin(buzzerP), display(nullptr) {}

void MorseTransmitter::begin(MorseDisplay* displayPtr) {
    display = displayPtr; 
    if (display) display->begin(); 

    pinMode(btnPin, INPUT_PULLUP);
    if (enterBtnPin != -1) pinMode(enterBtnPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    if (buzzerPin != -1) pinMode(buzzerPin, OUTPUT);
    
    if (!Serial) Serial.begin(9600);

    // Startup Lock
    isLocked = true;
    Serial.println(F("--- SYSTEM LOCKED ---"));
    if (display) display->setStatus(F("LOCKED: Enter PW"));
}

void MorseTransmitter::generateSignal(char type) {
  long duration;
  if (type == '.') duration = DOT_DURATION_MS; 
  else if (type == '-') duration = DASH_DURATION_MS; 
  else return;

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

void MorseTransmitter::checkUnlock() {
    if (manualSequence.length() == 0) return;
    unlockBuffer += manualSequence;
    Serial.print(F(" [Checking PW] So far: ")); Serial.println(unlockBuffer);
    
    if (unlockBuffer.equals(PASSCODE)) {
        isLocked = false;
        Serial.println(F("--- ACCESS GRANTED ---"));
        if (display) {
            display->clearAll();
            display->setStatus(F("ACCESS GRANTED"));
            delay(1000);
            display->setStatus(F("Ready"));
        }
        unlockBuffer = "";
    } else if (unlockBuffer.length() > PASSCODE.length()) {
        if (display) display->setStatus(F("WRONG PASSCODE"));
        delay(500);
        if (display) display->setStatus(F("LOCKED: Enter PW"));
        unlockBuffer = "";
    }
    manualSequence = "";
}

// --- FEATURE 3: MACRO EXPANSION ---
String MorseTransmitter::expandMacro(String input) {
    // You can add more codes here!
    if (input == "S1") return "SECTOR 1 SECURE";
    if (input == "S2") return "SECTOR 2 COMPROMISED";
    if (input == "RTB") return "RETURNING TO BASE";
    if (input == "B9") return "BATTERY CRITICAL";
    if (input == "73") return "BEST REGARDS";
    return input; // Return original if no match
}

void MorseTransmitter::processText(const String& text) {
  Serial.print(F("[TX] Playing: '")); Serial.print(text); Serial.print(F("' -> "));
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
        for (int j = 0; morseSeq[j] != '\0'; ++j) generateSignal(morseSeq[j]); 
        Serial.print(" "); 
        if (c != ' ') delay(CHAR_GAP_MS - ELEMENT_GAP_MS); 
        else delay(WORD_GAP_MS - ELEMENT_GAP_MS);
    }
  }
  Serial.println(F(" [DONE]"));
  if (display) { delay(1000); display->setStatus(F("Ready")); }
}

void MorseTransmitter::decodeCurrentSequence() {
  if (isLocked) { checkUnlock(); return; }

  if (manualSequence.length() == 0) return;
  Serial.print(F(" -> Seq: ")); Serial.print(manualSequence);
  if (manualSequence.startsWith("......")) {
      Serial.println(F(" -> [CMD] CLEAR BUFFER"));
      decodedMessageBuffer = ""; // Wipe the memory
      manualSequence = "";       // Wipe the current sequence
      
      if (display) {
          display->setStatus(F("CLEARED!"));
          delay(1000);
          display->clearAll();
          display->setStatus(F("Ready"));
      }
      return;
  }
  for (int i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (manualSequence.equals(morseCodeTable[i].sequence)) {
      char decodedChar = morseCodeTable[i].character;
      Serial.print(F(" -> Char: ")); Serial.println(decodedChar);
      
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

String MorseTransmitter::update() {
  unsigned long currentTime = millis();
  String messageToSend = ""; 

  // --- 1. HANDLE MORSE BUTTON ---
  int reading = digitalRead(btnPin);
  if (reading == LOW && lastButtonState == HIGH) { pressStartTime = currentTime; } 
  else if (reading == HIGH && lastButtonState == LOW) { 
    unsigned long pressDuration = currentTime - pressStartTime;
    char signalType = 0;
    
    if (pressDuration > 50) {
        if (pressDuration <= MAX_DOT_DURATION_MS) signalType = '.';
        else signalType = '-';
        
        Serial.print(F("[DEBUG] ")); Serial.print(pressDuration);
        Serial.print(F("ms -> ")); Serial.println(signalType);
        
        if (signalType) {
          manualSequence += signalType;
          generateSignal(signalType); 
          lastActivityTime = currentTime; 
          if (display && !isLocked) display->updateInputSequence(manualSequence);
        }
    }
  }
  lastButtonState = reading;

  if (reading == HIGH && manualSequence.length() > 0 && lastActivityTime > 0) {
    if (currentTime - lastActivityTime >= DECODE_TIMEOUT_MS) {
      decodeCurrentSequence(); lastActivityTime = 0; 
    }
  }

  // --- 2. HANDLE ENTER/SPACE BUTTON ---
  if (enterBtnPin != -1 && !isLocked) { 
      int enterReading = digitalRead(enterBtnPin);

      if (enterReading == LOW && lastEnterState == HIGH) { enterPressStartTime = currentTime; }
      else if (enterReading == HIGH && lastEnterState == LOW) { 
          unsigned long enterDuration = currentTime - enterPressStartTime;

          if (enterDuration > 50) {
              if (enterDuration >= 1000) { 
                  // --- LONG PRESS: SEND MESSAGE ---
                  if (decodedMessageBuffer.length() > 0) {
                      
                      // === FEATURE 2: SILENT DURESS CHECK ===
                      if (decodedMessageBuffer == DURESS_TRIGGER) {
                          Serial.println(F("[ALERT] DURESS TRIGGERED!"));
                          messageToSend = DURESS_MESSAGE; 
                          
                          // DECEPTION: Tell user it worked normally
                          if (display) {
                              display->setStatus(F("Sending..."));
                              delay(500);
                              display->clearAll();
                              display->setStatus(F("Msg Sent OK")); 
                          }
                          decodedMessageBuffer = ""; 
                          return messageToSend; // Exit immediately
                      }

                      // === FEATURE 3: MACRO EXPANSION ===
                      // Try to expand short code (e.g. "S1")
                      String expanded = expandMacro(decodedMessageBuffer);
                      
                      if (expanded != decodedMessageBuffer) {
                          Serial.print(F("[MACRO] Expanded: "));
                          Serial.println(expanded);
                          messageToSend = expanded;
                      } else {
                          messageToSend = decodedMessageBuffer;
                      }
                      
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