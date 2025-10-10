#include <Arduino.h>
#include "morse.h"

const int buttonPin = 2;   // Button input
const int ledPin = 13;     // LED
const int buzzerPin = 8;   // Active buzzer

String currentMorse = "";
unsigned long pressStart = 0;
bool buttonPressed = false;
unsigned long lastRelease = 0;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Morse Decoder Ready: Tap button");
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  // Button just pressed
  if (buttonState == LOW && !buttonPressed) {
    buttonPressed = true;
    pressStart = millis();
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);   // Active buzzer ON
  }

  // Button just released
  if (buttonState == HIGH && buttonPressed) {
    buttonPressed = false;
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);    // Active buzzer OFF

    unsigned long pressDuration = millis() - pressStart;

    if (pressDuration < 300) {
      currentMorse += ".";
      Serial.print(".");
    } else {
      currentMorse += "-";
      Serial.print("-");
    }
    lastRelease = millis();
  }

  // Decode after short pause (end of a letter)
  if (!buttonPressed && currentMorse.length() > 0 && (millis() - lastRelease > 600)) {
    char decoded = decodeMorse(currentMorse);
    Serial.print(" -> ");
    Serial.println(decoded);
    currentMorse = "";
  }

  // Print space after longer pause (end of a word)
  if (!buttonPressed && (millis() - lastRelease > 1200)) {
    Serial.print(" ");
    lastRelease = millis();
  }
}
