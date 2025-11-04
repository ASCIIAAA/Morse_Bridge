#ifndef MESSAGE_LOGGER_H
#define MESSAGE_LOGGER_H

#include <Arduino.h>
#include "BluetoothInterface.h"
#include <RTClib.h>
#include <Wire.h> // RTC modules use I2C

class MessageLogger {
private:
    BluetoothInterface& bt; // Reference to the BT module
    RTC_DS3231& rtc;        // Reference to the RTC module
    char timestamp[25];     // Buffer to hold "YYYY-MM-DD hh:mm:ss"

    /**
     * @brief Formats the current time into the timestamp buffer.
     */
    void formatTimestamp(const DateTime& now);

public:
    /**
     * @brief Constructor for the Message Logger.
     * @param btInstance A reference to your BluetoothInterface object.
     * @param rtcInstance A reference to your RTC_DS3231 object.
     */
    MessageLogger(BluetoothInterface& btInstance, RTC_DS3231& rtcInstance);

    /**
     * @brief Initializes the logger and the RTC module.
     * @return True if RTC initialization was successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Logs a message with a timestamp.
     * @param origin A string like "SPY" or "ADMIN".
     * @param message The message content to log.
     */
    void log(const String& origin, const String& message);
};

#endif // MESSAGE_LOGGER_H