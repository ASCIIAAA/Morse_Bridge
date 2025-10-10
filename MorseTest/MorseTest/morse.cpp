#include <Arduino.h>   // Needed for String
#include "morse.h"

// Pin definitions (you can also move to .ino if preferred)
const int ledPin = 13;      
const int buzzerPin = 8;    

// Morse lookup table (A–Z + 0–9)
const char* morseTable[36] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",    
    ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",  
    "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",         
    "-----", ".----", "..---", "...--", "....-", ".....", 
    "-....", "--...", "---..", "----."                                
};

const char* getMorse(char c) {
    if (c >= 'a' && c <= 'z') c -= 32;  
    if (c >= 'A' && c <= 'Z') return morseTable[c - 'A'];
    if (c >= '0' && c <= '9') return morseTable[26 + (c - '0')];
    return "";
}

char decodeMorse(String morse) {
    for (int i = 0; i < 36; i++) {
        if (morse.equals(morseTable[i])) {
            if (i < 26) return 'A' + i;
            else return '0' + (i - 26);
        }
    }
    return '?';
}

// 🔔 Blink LED + Beep buzzer for Morse code
void blinkBeepMorse(const char* code) {
    for (int i = 0; code[i] != '\0'; i++) {
        if (code[i] == '.') {
            digitalWrite(ledPin, HIGH);
            tone(buzzerPin, 1000);   // 1 kHz beep
            delay(200);              // dot duration
            digitalWrite(ledPin, LOW);
            noTone(buzzerPin);
        } else if (code[i] == '-') {
            digitalWrite(ledPin, HIGH);
            tone(buzzerPin, 1000);
            delay(600);              // dash duration
            digitalWrite(ledPin, LOW);
            noTone(buzzerPin);
        }
        delay(200); // gap between symbols
    }
    delay(600); // gap between letters
}
