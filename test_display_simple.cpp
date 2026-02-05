/**
 * SIMPLE DISPLAY TEST
 * Tests basic SPI communication and pin connectivity
 * Upload this instead of main.cpp to test display isolation
 */

#include <Arduino.h>
#include <SPI.h>

// Pin definitions
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4
#define TFT_MOSI 11
#define TFT_MISO 19
#define TFT_SCLK 18
#define TFT_BL   21

void writeCommand(uint8_t cmd) {
    digitalWrite(TFT_DC, LOW);   // Command mode
    digitalWrite(TFT_CS, LOW);   // Select display
    SPI.transfer(cmd);
    digitalWrite(TFT_CS, HIGH);  // Deselect
}

void writeData(uint8_t data) {
    digitalWrite(TFT_DC, HIGH);  // Data mode
    digitalWrite(TFT_CS, LOW);   // Select display
    SPI.transfer(data);
    digitalWrite(TFT_CS, HIGH);  // Deselect
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n=== SIMPLE DISPLAY TEST ===");

    // Setup pins
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_BL, OUTPUT);

    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_BL, HIGH);

    // Initialize SPI
    SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
    SPI.setFrequency(10000000);  // 10MHz
    SPI.setDataMode(SPI_MODE0);

    Serial.println("Pins initialized, SPI started");

    // Hardware reset
    Serial.println("Performing hardware reset...");
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(50);
    digitalWrite(TFT_RST, HIGH);
    delay(200);

    Serial.println("Sending initialization commands...");

    // Software reset
    writeCommand(0x01);  // SWRESET
    delay(150);

    // Sleep out
    writeCommand(0x11);  // SLPOUT
    delay(150);

    // Display on
    writeCommand(0x29);  // DISPON
    delay(50);

    Serial.println("Basic init complete. Display should show something now.");
    Serial.println("Backlight is ON (GPIO 21)");
    Serial.println("\nIf still white:");
    Serial.println("1. Display may be defective");
    Serial.println("2. Wiring issue (check with multimeter)");
    Serial.println("3. Wrong voltage (should be 3.3V NOT 5V)");
    Serial.println("4. Display needs parallel mode not SPI");
}

void loop() {
    // Blink backlight to show we're running
    static unsigned long lastBlink = 0;
    static bool blState = true;

    if (millis() - lastBlink > 1000) {
        blState = !blState;
        digitalWrite(TFT_BL, blState ? HIGH : LOW);
        Serial.print("Backlight: ");
        Serial.println(blState ? "ON" : "OFF");
        lastBlink = millis();
    }
}
