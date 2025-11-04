#include "BluetoothInterface.h"

BluetoothInterface::BluetoothInterface(uint8_t rxPin, uint8_t txPin)
    : btSerial(rxPin, txPin), messageReady(false) {}

void BluetoothInterface::begin(long baudRate) {
    btSerial.begin(baudRate);
    delay(500);
}

void BluetoothInterface::sendMessage(const String& message) {
    btSerial.println(message);
}

void BluetoothInterface::checkForIncoming() {
    while (btSerial.available() > 0) {
        char c = btSerial.read();

        if (c == '\n' || c == '\r') {
            if (incomingBuffer.length() > 0) {
                messageReady = true;
            }
        } else {
            incomingBuffer += c;
        }
    }
}

bool BluetoothInterface::hasMessage() {
    return messageReady;
}

String BluetoothInterface::getMessage() {
    messageReady = false;
    String msg = incomingBuffer;
    incomingBuffer = "";
    return msg;
}

void BluetoothInterface::clearBuffer() {
    incomingBuffer = "";
    messageReady = false;
}
