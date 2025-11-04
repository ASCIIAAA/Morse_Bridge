#ifndef MORSE_CONFIG_H
#define MORSE_CONFIG_H

// --- Common Hardware Pins (Used by BOTH units) ---
#define LED_PIN 4       // Built-in LED
#define BUTTON_PIN 2     // Manual Morse input button
#define LCD_ADDRESS 0x27 // I2C address for the LCD
#define LCD_COLS 16
#define LCD_ROWS 2

// --- NRF24L01 Radio Configuration (Used by BOTH units) ---
#define NRF_CE_PIN 9  // Chip Enable pin
#define NRF_CSN_PIN 10 // Chip Select (SPI) pin
// Note: SPI pins (SCK, MISO, MOSI) are hardware-specific
// (Uno/Nano: 13, 12, 11)
// (Mega: 52, 50, 51)

// The "pipe" address for communication. Must be identical on both units.
// This 5-character string is the "channel" they talk on.
const byte radioPipeAddress[6] = "MSG01";


// --- ================================== ---
// ---       ADMIN-SPECIFIC PINS        ---
// --- ================================== ---
#define ADMIN_BUZZER_PIN 8      // Buzzer for Morse output
#define ADMIN_BT_RX_PIN 7       // Bluetooth HC-05 RX
#define ADMIN_BT_TX_PIN 6       // Bluetooth HC-05 TX
// RTC (DS3231) uses I2C pins (SDA/SCL), same as the LCD (A4/A5 on Uno)


// --- ================================== ---
// ---         SPY-SPECIFIC PINS          ---
// --- ================================== ---
// The Spy unit uses LED_PIN (13) for Morse output.
// We can define a "buzzer" pin as -1 to disable it in the transmitter.
#define SPY_BUZZER_PIN -1       // -1 signifies no buzzer


#endif // MORSE_CONFIG_H