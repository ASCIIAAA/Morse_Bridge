#ifndef BLUETOOTH_INTERFACE_H
#define BLUETOOTH_INTERFACE_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class BluetoothInterface {
private:
    SoftwareSerial btSerial;
    String incomingBuffer;
    bool messageReady;

public:
    BluetoothInterface(uint8_t rxPin, uint8_t txPin);

    void begin(long baudRate = 9600);
    void sendMessage(const String& message);
    void checkForIncoming();
    bool hasMessage();
    String getMessage();
    void clearBuffer();
};

#endif
