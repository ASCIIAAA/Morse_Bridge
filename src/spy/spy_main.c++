#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "MorseDisplay.h"
#include "MorseTransmitter.h"
#include "RadioInterface.h"
#include <RF24.h>

// --- Global Instances (Spy) ---
MorseDisplay display(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// The transmitter class is used for BOTH:
// 1. Sending Morse (LED only, no buzzer)
// 2. Decoding manual button input
MorseTransmitter transmitter(BUTTON_PIN, LED_PIN, SPY_BUZZER_PIN); // Buzzer pin is -1

RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
RadioInterface nrf(radio, radioPipeAddress);

void setup() {
    Serial.begin(9600); // For debugging
    Wire.begin();       // For LCD

    display.begin();
    transmitter.begin(&display);
    
    if (!nrf.begin()) {
        Serial.println("FATAL: Radio failed!");
        display.setStatus(F("NRF FAIL"));
        while (1); // Halt
    }

    display.setStatus(F("Spy Unit Ready"));
    Serial.println("--- SPY SYSTEM ONLINE ---");
}

void loop() {
    // --- Mode 1: Check for incoming Admin replies via NRF ---
    if (nrf.isMessageAvailable()) {
        String msg = nrf.getMessage();
        Serial.println("ADMIN MSG RX: " + msg);
        
        display.setStatus(F("Admin Msg RX..."));
        
        // Play the message in Morse (LED only, as buzzer pin is -1)
        transmitter.processText(msg);
        
        // (Optional) Send an Acknowledgment
        // nrf.sendMessage("ACK"); 
        
        display.setStatus(F("Spy Unit Ready"));
    }

    // --- Mode 2: Check for manual button input ---
    // transmitter.update() handles button presses and returns a
    // complete message string when the user pauses.
    String messageToSend = transmitter.update();
    
    if (messageToSend.length() > 0) {
        // We have a message to send!
        Serial.println("Sending to Admin: " + messageToSend);
        
        if (nrf.sendMessage(messageToSend)) {
            display.setStatus(F("Msg Sent OK"));
            // The transmitter.update() function already handles
            // clearing the display after a successful send.
        } else {
            display.setStatus(F("Msg Send FAIL"));
        }
        
        // Short delay to show status
        delay(1000);
        display.setStatus(F("Spy Unit Ready"));
    }
}