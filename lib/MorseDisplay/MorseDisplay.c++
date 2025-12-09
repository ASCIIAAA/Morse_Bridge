#include "MorseDisplay.h"

// --- Constructor (CORRECTED INITIALIZER LIST) ---
MorseDisplay::MorseDisplay(uint8_t lcdAddress, uint8_t lcdCols, uint8_t lcdRows)
    // Initialize member variables using the new names (lcdCols_, lcdRows_)
    : lcd(lcdAddress, lcdCols, lcdRows), lcdCols_(lcdCols), lcdRows_(lcdRows) {
    // Constructor initializes the LiquidCrystal_I2C object
}

// --- Initialization ---
void MorseDisplay::begin() {
    lcd.init();
    lcd.backlight();
    showStartupMessage();
    delay(2000);
    clearAll();
}

// --- Internal Helper: Clears one line ---
void MorseDisplay::clearLine(uint8_t line) {
    lcd.setCursor(0, line);
    // Use the corrected member variable: lcdCols_
    for (uint8_t i = 0; i < lcdCols_; ++i) {
        lcd.print(' ');
    }
}

// --- Internal Helper: Shifts Line 0 left if it gets too long ---
void MorseDisplay::shiftLine0Left() {
    // Use the corrected member variable: lcdCols_
    if (line0Buffer.length() > lcdCols_) {
        // Keep only the latest lcdCols_ characters
        line0Buffer = line0Buffer.substring(line0Buffer.length() - lcdCols_);
        
        // Redraw the entire line from the buffer
        lcd.setCursor(0, 0);
        lcd.print(line0Buffer);
    }
}

// --- Public Display Functions ---

void MorseDisplay::showStartupMessage() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Morse Bridge V1.1"));
    lcd.setCursor(0, 1);
    lcd.print(F("Initializing..."));
}

void MorseDisplay::clearAll() {
    lcd.clear();
    line0Buffer = "";
    line1Buffer = "";
    setStatus(F("Ready"));
}

void MorseDisplay::setStatus(const String& status) {
    line1Buffer = status;
    clearLine(1);
    lcd.setCursor(0, 1);
    lcd.print(line1Buffer);
}

void MorseDisplay::updateInputSequence(const String& sequence) {
    // String concatenation fix applied here
    line1Buffer = String(F("Input: ")) + sequence; 
    
    // Use the corrected member variable: lcdCols_
    if (line1Buffer.length() > lcdCols_) {
        line1Buffer = line1Buffer.substring(0, lcdCols_);
    }
    
    clearLine(1);
    lcd.setCursor(0, 1);
    lcd.print(line1Buffer);
}

void MorseDisplay::appendDecodedCharacter(char c) {
    if (c == ' ') {
        line0Buffer += ' ';
    } else {
        line0Buffer += toupper(c);
    }

    shiftLine0Left(); // Handle overflow
    
    // Display only the appended character to reduce flicker
    // Use the corrected member variable: lcdCols_
    if (line0Buffer.length() <= lcdCols_) {
        lcd.setCursor(line0Buffer.length() - 1, 0);
        lcd.print(line0Buffer.charAt(line0Buffer.length() - 1));
    } else {
        // If we shifted, redraw the whole line
        lcd.setCursor(0, 0);
        lcd.print(line0Buffer);
    }
}