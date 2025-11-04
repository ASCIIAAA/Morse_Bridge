#ifndef RADIO_INTERFACE_H
#define RADIO_INTERFACE_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h> // Make sure you have the 'RF24' library by TMRh20

class RadioInterface {
private:
    RF24& radio; // A reference to the RF24 object
    const byte* pipeAddress;
    char receiveBuffer[33]; // Holds incoming messages (32 bytes + null terminator)

public:
    /**
     * @brief Constructor for the Radio Interface.
     * @param radioInstance An initialized RF24 object.
     * @param pipeAddress A 5-byte array representing the pipe address.
     */
    RadioInterface(RF24& radioInstance, const byte* pipeAddress);

    /**
     * @brief Initializes the radio hardware.
     */
    bool begin();

    /**
     * @brief Switches the radio into receiver mode.
     */
    void startListening();

    /**
     * @brief Sends a message.
     * @param message The String message to send (max 32 bytes).
     * @return True on success, false on failure.
     */
    bool sendMessage(const String& message);

    /**
     * @brief Checks if a message is available from the radio.
     * @return True if a message is waiting.
     */
    bool isMessageAvailable();

    /**
     * @brief Reads the available message.
     * @return The received message as a String.
     */
    String getMessage();
};

#endif // RADIO_INTERFACE_H