#include "MessageLogger.h"

MessageLogger::MessageLogger(BluetoothInterface& btInstance, RTC_DS3231& rtcInstance)
    : bt(btInstance), rtc(rtcInstance) {
    // Constructor initializes references
}

bool MessageLogger::begin() {
    // FIX 1: Wire.begin() is void, so we just call it.
    // We can't check its return value.
    Wire.begin(); 

    if (!rtc.begin()) {
        Serial.println(F("Couldn't find RTC module!"));
        bt.sendMessage(F("LOG_ERROR: RTC NOT FOUND"));
        return false;
    }

    if (rtc.lostPower()) {
        Serial.println(F("RTC lost power, setting time to compile time..."));
        
        // FIX 2: Removed the F() macro from _DATE_ and _TIME_
        rtc.adjust(DateTime(__DATE__, __TIME__));
        
        // You can also set a specific time:
        // rtc.adjust(DateTime(2025, 1, 21, 10, 0, 0));
    }

    Serial.println(F("RTC and Logger initialized."));
    bt.sendMessage(F("LOG_SYSTEM: Logger online."));
    return true;
}

void MessageLogger::formatTimestamp(const DateTime& now) {
    // Formats the timestamp into: "YYYY-MM-DD hh:mm:ss"
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(),
             now.month(),
             now.day(),
             now.hour(),
             now.minute(),
             now.second());
}

void MessageLogger::log(const String& origin, const String& message) {
    // 1. Get the current time
    DateTime now = rtc.now();
    
    // 2. Format the timestamp
    formatTimestamp(now);

    // 3. Create the full log entry
    // Example: "[2025-11-04 09:30:00] [SPY] > SOS"
    String logEntry = "[";
    logEntry += timestamp;
    logEntry += "] [";
    logEntry += origin;
    logEntry += "] > ";
    logEntry += message;

    // 4. Send over Bluetooth
    bt.sendMessage(logEntry);

    // 5. (Optional) Print to local Serial as well
    Serial.println(logEntry);
}