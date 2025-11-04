#include <Arduino.h>
#include <MorseTransmitter.h> 
#include "MorseDisplay.h"
#include <SoftwareSerial.h>

// --- Bluetooth Configuration (HC-05) ---
#define BT_RX 10
#define BT_TX 11
SoftwareSerial BT(BT_RX, BT_TX); // RX, TX

// --- Hardware Configuration ---
#define BUTTON_PIN 2  
#define BUZZER_PIN 8  
#define LED_PIN 13   

// --- LCD Configuration ---
#define LCD_ADDRESS 0x27   // Change to 0x3F if LCD doesn't respond
#define LCD_COLS 16
#define LCD_ROWS 2

// --- Global Instances ---
MorseDisplay display(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
MorseTransmitter transmitter(BUTTON_PIN, LED_PIN, BUZZER_PIN);

// --- Serial Buffer ---
String serialInputBuffer = "";
String bluetoothBuffer = "";

// --- Setup ---
void setup() {
  Serial.begin(9600);
  BT.begin(9600); // Initialize Bluetooth communication
  transmitter.begin(&display);

  display.setStatus(F("Admin Ready"));
  Serial.println("Admin System Initialized - Bluetooth Ready");
}

// --- Main Loop ---
void loop() {
  // --- Mode 1: Manual Morse Input (Button Input) ---
  transmitter.update();

  // --- Mode 2: Serial Text Input (From PC) ---
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();

    if (incomingChar == '\n' || incomingChar == '\r') {
      if (serialInputBuffer.length() > 0) {
        // Transmit Morse Code for text entered via Serial
        transmitter.processText(serialInputBuffer);

        // Also send over Bluetooth to Spy Unit
        BT.println(serialInputBuffer);
        Serial.println("Sent via Bluetooth: " + serialInputBuffer);

        // Clear buffer
        serialInputBuffer = "";
        display.setStatus(F("Ready for Manual"));
      }
    } else {
      serialInputBuffer += incomingChar;
    }
  }

  // --- Mode 3: Bluetooth Input (From Spy Unit) ---
  while (BT.available() > 0) {
    char btChar = BT.read();

    if (btChar == '\n' || btChar == '\r') {
      if (bluetoothBuffer.length() > 0) {
        Serial.println("Received via Bluetooth: " + bluetoothBuffer);

        // Display and process Morse
        display.setStatus(F("BT Msg Received"));
        transmitter.processText(bluetoothBuffer);

        // Optionally, send ACK
        BT.println("ACK: " + bluetoothBuffer);

        bluetoothBuffer = "";
      }
    } else {
      bluetoothBuffer += btChar;
    }
  }
}
