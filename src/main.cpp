#include <Arduino.h>
#include <MorseTransmitter.h> 
#include "MorseDisplay.h" // FIX: Include the display header

// --- Hardware Configuration ---
#define BUTTON_PIN 2  
#define BUZZER_PIN 8  
#define LED_PIN 13   

// --- LCD Configuration (Standard I2C Address 0x27) ---
// IMPORTANT: Change 0x27 to 0x3F if your LCD doesn't work.
#define LCD_ADDRESS 0x27 
#define LCD_COLS 16
#define LCD_ROWS 2

// --- Global Component Instances ---
MorseDisplay display(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

MorseTransmitter transmitter(BUTTON_PIN, LED_PIN, BUZZER_PIN);

// --- State for Serial Input Handling ---
String serialInputBuffer = "";

// --- Setup Function ---
void setup() {
  transmitter.begin(&display);
}

// --- Main Loop Function (Runs repeatedly) ---
void loop() {
  // Mode 1: Manual Morse Input (Decoding)
  transmitter.update();

  // Mode 2: Serial Text Input (Encoding)
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();

    if (incomingChar == '\n' || incomingChar == '\r') {
      
      if (serialInputBuffer.length() > 0) {
        // Process the complete line of text
        transmitter.processText(serialInputBuffer);
        
        // Clear the buffer after transmission
        serialInputBuffer = "";
        
        // Set status back to ready after processing text input
        display.setStatus(F("Ready for Manual"));
      }
    } 
    // Otherwise, accumulate the character
    else {
      serialInputBuffer += incomingChar;
    }
  }
}
