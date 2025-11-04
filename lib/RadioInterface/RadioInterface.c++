#include "RadioInterface.h"

RadioInterface::RadioInterface(RF24& radioInstance, const byte* address)
    : radio(radioInstance), pipeAddress(address) {
    // Constructor initializes the reference and pipe address
}

bool RadioInterface::begin() {
    // Initialize the radio
    if (!radio.begin()) {
        Serial.println(F("Radio hardware not responding!!"));
        return false;
    }

    // Set the RF channel (e.g., 76) - 0-125
    radio.setChannel(76); 
    
    // Set the PA Level to high for better range
    // (use RF24_PA_MIN for testing at close range)
    radio.setPALevel(RF24_PA_HIGH);
    
    // Set Data Rate
    radio.setDataRate(RF24_250KBPS); // Slower rate for more reliability/range

    // Set payload size (32 bytes is default)
    radio.setPayloadSize(32);

    // Set the address
    // Use openReadingPipe(1, ...) for multi-pipe listening
    radio.openReadingPipe(0, pipeAddress);
    
    // Set the writing pipe to the same address for replies
    radio.openWritingPipe(pipeAddress);

    // Set retries (15 max) and delay (5*250us = 1250us)
    radio.setRetries(15, 15);

    // Start in listening mode
    startListening();
    
    return true;
}

void RadioInterface::startListening() {
    radio.startListening();
}

bool RadioInterface::sendMessage(const String& message) {
    radio.stopListening(); // Stop listening to transmit

    // Ensure message fits in 32 bytes
    if (message.length() > 32) {
        Serial.println(F("Error: Message too long."));
        startListening(); // Go back to listening
        return false;
    }

    // Convert String to char array
    char sendBuffer[33];
    message.toCharArray(sendBuffer, 33);

    // Send the message
    Serial.print(F("Sending message: "));
    Serial.println(sendBuffer);

    bool tx_ok = radio.write(&sendBuffer, strlen(sendBuffer) + 1); // +1 for null terminator

    if (!tx_ok) {
        Serial.println(F("Transmission failed."));
    } else {
        Serial.println(F("Transmission successful."));
    }

    startListening(); // Always return to listening mode
    return tx_ok;
}

bool RadioInterface::isMessageAvailable() {
    return radio.available();
}

String RadioInterface::getMessage() {
    if (radio.available()) {
        // Read the data into the buffer
        radio.read(&receiveBuffer, sizeof(receiveBuffer));
        
        // Ensure null-termination for safety
        receiveBuffer[32] = '\0'; 
        
        return String(receiveBuffer);
    }
    
    return ""; // Return empty string if no message
}