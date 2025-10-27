#ifndef MORSE_DISPLAY_H
#define MORSE_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class MorseDisplay {
private:
    LiquidCrystal_I2C lcd;
    const uint8_t LCD_COLS;
    const uint8_t LCD_ROWS;

    // We will use two separate strings to manage the content of the two lines
    String line0Buffer; // Decoded/Sent Message
    String line1Buffer; // Current Input/Status

    void clearLine(uint8_t line);
    void shiftLine0Left();

public:
    // Constructor initializes the LCD object and dimensions
    MorseDisplay(uint8_t lcdAddress, uint8_t lcdCols, uint8_t lcdRows);

    // Initialization routine
    void begin();

    // Core Display Methods
    void showStartupMessage();
    void setStatus(const String& status);
    
    // Updates Line 1 with the current sequence (e.g., ".-.")
    void updateInputSequence(const String& sequence);

    // Appends a character to the final message (Line 0)
    void appendDecodedCharacter(char c);

    // Clears the message history (Line 0) and the input status (Line 1)
    void clearAll();
};

#endif // MORSE_DISPLAY_H
