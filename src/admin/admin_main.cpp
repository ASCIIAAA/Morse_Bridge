#include <Arduino.h>
#include <Wire.h>
#include "Config.h"           // Your new config file
#include "MorseDisplay.h"
#include "MorseTransmitter.h"
#include "BluetoothInterface.h"
#include "RadioInterface.h"
#include "MessageLogger.h"
#include <RF24.h>
#include <RTClib.h>

// --- Global Instances (Admin) ---
MorseDisplay display(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
// Change this line in your Admin Global Instances:
MorseTransmitter transmitter(BUTTON_PIN, -1, LED_PIN, ADMIN_BUZZER_PIN);
//MorseTransmitter transmitter(BUTTON_PIN, LED_PIN, ADMIN_BUZZER_PIN);
BluetoothInterface bt(ADMIN_BT_RX_PIN, ADMIN_BT_TX_PIN);
RTC_DS3231 rtc;
MessageLogger logger(bt, rtc);

RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
RadioInterface nrf(radio, radioPipeAddress);

String serialInputBuffer = "";

void setup() {
    Serial.begin(9600);
    Wire.begin(); // Initialize I2C for LCD and RTC
    
    bt.begin(9600);
    display.begin();
    transmitter.begin(&display); // Pass display to transmitter
    
    if (!logger.begin()) {
        Serial.println("FATAL: Logger (RTC/BT) failed!");
        display.setStatus(F("RTC FAIL"));
        while (1); // Halt
    } else {
        Serial.println("Logger OK.");
    }
    
    if (!nrf.begin()) {
        Serial.println("FATAL: Radio failed!");
        display.setStatus(F("NRF FAIL"));
        while (1); // Halt
    }

    display.setStatus(F("Admin Ready"));
    Serial.println("--- ADMIN SYSTEM ONLINE ---");
    logger.log("SYSTEM", "Admin Unit Online");
}

void loop() {
    // --- Mode 1: Check for incoming Spy messages via NRF ---
    if (nrf.isMessageAvailable()) {
        String msg = nrf.getMessage();
        Serial.println("NRF MSG RX: " + msg);
        
        display.setStatus(F("Spy Msg RX..."));
        logger.log("SPY", msg);
        
        // Play the message in Morse
        transmitter.processText(msg);
        
        display.setStatus(F("Admin Ready"));
    }

    // --- Mode 2: Check for Serial input (to reply to Spy) ---
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (serialInputBuffer.length() > 0) {
                Serial.println("Sending to Spy: " + serialInputBuffer);
                
                // 1. Stop radio listening to send
                display.setStatus(F("Sending Reply..."));
                if (nrf.sendMessage(serialInputBuffer)) {
                    display.setStatus(F("Reply Sent OK"));
                    logger.log("ADMIN", serialInputBuffer);
                } else {
                    display.setStatus(F("Reply FAIL"));
                    logger.log("ERROR", "Reply send failed");
                }
                
                // 2. Play Morse locally
                transmitter.processText(serialInputBuffer);
                
                serialInputBuffer = "";
                display.setStatus(F("Admin Ready"));
            }
        } else {
            serialInputBuffer += c;
        }
    }
    
    // --- Mode 3: Manual Button Input (Optional) ---
    // Uncomment this if you want the Admin to also send via button
    String adminMsg = transmitter.update();
    if (adminMsg.length() > 0) {
        Serial.println("Sending manual msg to Spy: " + adminMsg);
        if (nrf.sendMessage(adminMsg)) {
            logger.log("ADMIN", adminMsg);
        } else {
            logger.log("ERROR", "Manual send failed");
        }
    }
}