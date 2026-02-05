/**
 * Display Manager - TFT Display Control for ILI9341
 * Handles all screen rendering and UI states for 320x240 color display
 */

#include "display_manager.h"
#include "touch_manager.h"
#include "ui_framework.h"
#include <SPI.h>
#include <WiFi.h>

// Swap macro for drawing algorithms
#ifndef swap
#define swap(a, b) { typeof(a) t = a; a = b; b = t; }
#endif

DisplayManager displayMgr;

DisplayManager::DisplayManager() :
    tft(nullptr),
    hspi(nullptr),
    currentState(DISPLAY_BOOT),
    lastUpdate(0),
    brightness(255),
    sleeping(false),
    cursorX(0),
    cursorY(0),
    textColor(COLOR_WHITE),
    textBgColor(COLOR_BLACK),
    textSize(1)
{
}

// Manual SPI command send (exact copy from bare-bones test)
void DisplayManager::writeCommand(uint8_t cmd) {
    digitalWrite(TFT_DC, LOW);   // Command mode
    digitalWrite(TFT_CS, LOW);   // Select display
    hspi->transfer(cmd);         // Use HSPI like bare-bones test
    digitalWrite(TFT_CS, HIGH);  // Deselect
}

// Manual SPI data send (exact copy from bare-bones test)
void DisplayManager::writeData(uint8_t data) {
    digitalWrite(TFT_DC, HIGH);  // Data mode
    digitalWrite(TFT_CS, LOW);   // Select display
    hspi->transfer(data);        // Use HSPI like bare-bones test
    digitalWrite(TFT_CS, HIGH);  // Deselect
}

// Manual SPI 16-bit data send (exact copy from bare-bones test)
void DisplayManager::writeData16(uint16_t data) {
    digitalWrite(TFT_DC, HIGH);  // Data mode
    digitalWrite(TFT_CS, LOW);   // Select display
    hspi->transfer(data >> 8);   // High byte
    hspi->transfer(data & 0xFF); // Low byte
    digitalWrite(TFT_CS, HIGH);  // Deselect
}

// Set address window for drawing (ILI9341 commands)
void DisplayManager::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writeCommand(0x2A);  // Column address set
    writeData16(x0);     // Start column
    writeData16(x1);     // End column

    writeCommand(0x2B);  // Row address set
    writeData16(y0);     // Start row
    writeData16(y1);     // End row

    writeCommand(0x2C);  // Memory write
}

// Fast pixel draw without bounds checking
void DisplayManager::drawPixelFast(int16_t x, int16_t y, uint16_t color) {
    setAddrWindow(x, y, x, y);
    writeData16(color);
}

bool DisplayManager::begin() {
    Serial.println("Display init: BARE BONES MODE (like working test)...");

    // Setup pins
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_BL, OUTPUT);

    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_BL, HIGH);  // Backlight ON
    Serial.println("  ✓ Pins configured");

    // Initialize HSPI at 4MHz (same as working test)
    hspi = new SPIClass(HSPI);
    hspi->begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
    hspi->setFrequency(4000000);  // 4MHz
    hspi->setDataMode(SPI_MODE0);
    hspi->setBitOrder(MSBFIRST);
    Serial.println("  ✓ SPI initialized at 4MHz");

    // Hardware reset (EXACT sequence from working test)
    digitalWrite(TFT_RST, HIGH);
    delay(20);
    digitalWrite(TFT_RST, LOW);
    delay(100);  // Long reset pulse
    digitalWrite(TFT_RST, HIGH);
    delay(300);  // Long recovery time
    Serial.println("  ✓ Reset complete");

    // Manual ILI9341 initialization (EXACT sequence from working test)
    writeCommand(0x01);  // Software reset
    delay(200);

    writeCommand(0x28);  // Display OFF
    delay(20);

    writeCommand(0x11);  // Sleep OUT
    delay(150);

    writeCommand(0x3A);  // Pixel format
    writeData(0x55);  // 16-bit/pixel
    delay(10);

    writeCommand(0x36);  // Memory access control
    writeData(0x28);  // MV, BGR (90 degree rotation - landscape)
    delay(10);

    writeCommand(0x29);  // Display ON
    delay(50);

    Serial.println("  ✓ ILI9341 initialized");

    // Fill screen with black
    fillScreen(0x0000);  // Black
    Serial.println("  ✓ Screen filled black");

    Serial.println("✓ Display initialization complete!");

    return true;
}

// Custom ILI9341 initialization - EXACT sequence from working bare-bones test
// Using our manual writeCommand/writeData functions
void DisplayManager::customILI9341Init() {
    Serial.println("  → Software reset (0x01)...");
    writeCommand(0x01);
    delay(200);

    Serial.println("  → Display OFF (0x28)...");
    writeCommand(0x28);
    delay(20);

    Serial.println("  → Sleep OUT (0x11)...");
    writeCommand(0x11);
    delay(150);

    Serial.println("  → Set pixel format (0x3A)...");
    writeCommand(0x3A);
    writeData(0x55);  // 16-bit/pixel
    delay(10);

    Serial.println("  → Memory access control (0x36)...");
    writeCommand(0x36);
    writeData(0x28);  // MV, BGR (90 degree rotation - landscape)
    delay(10);

    Serial.println("  → Display ON (0x29)...");
    writeCommand(0x29);
    delay(50);

    Serial.println("  ✓ Custom ILI9341 init complete!");
}

// =============================================================================
// Simple 5x7 Bitmap Font (ASCII 32-126)
// =============================================================================

// Font data: 5 bytes per character, each byte is a column (MSB = top pixel)
static const uint8_t font5x7[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x14, 0x08, 0x3E, 0x08, 0x14, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x00, 0x41, 0x22, 0x14, 0x08, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x09, 0x01, // F
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x07, 0x08, 0x70, 0x08, 0x07, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x7F, 0x41, 0x41, 0x00, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // Backslash
    0x00, 0x41, 0x41, 0x7F, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x0C, 0x52, 0x52, 0x52, 0x3E, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x7F, 0x10, 0x28, 0x44, 0x00, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x10, 0x08, 0x08, 0x10, 0x08, // ~
};

// Draw a single character using manual SPI (5x7 bitmap font)
void DisplayManager::drawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (c < 32 || c > 126) c = '?';  // Replace non-printable with ?

    const uint8_t *charData = &font5x7[(c - 32) * 5];

    // Character is 6x7 pixels (5 pixels + 1 spacing)
    int16_t charWidth = 6 * size;
    int16_t charHeight = 7 * size;

    // Bounds check
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    if (x + charWidth < 0 || y + charHeight < 0) return;

    // Set address window for the entire character area
    writeCommand(0x2A);  // Column address
    writeData16(x);
    writeData16(x + charWidth - 1);

    writeCommand(0x2B);  // Row address
    writeData16(y);
    writeData16(y + charHeight - 1);

    writeCommand(0x2C);  // Memory write

    // Stream all pixel data at once
    digitalWrite(TFT_DC, HIGH);  // Data mode
    digitalWrite(TFT_CS, LOW);   // Select display

    // For each row of pixels
    for (uint8_t row = 0; row < 7; row++) {
        for (uint8_t sy = 0; sy < size; sy++) {
            // For each column (including spacing)
            for (uint8_t col = 0; col < 6; col++) {
                uint8_t columnData = (col < 5) ? pgm_read_byte(&charData[col]) : 0;
                bool pixelOn = (columnData & (1 << row)) != 0;
                uint16_t pixelColor = pixelOn ? color : bg;

                // Draw scaled horizontally
                for (uint8_t sx = 0; sx < size; sx++) {
                    hspi->transfer16(pixelColor);
                }
            }
        }
    }

    digitalWrite(TFT_CS, HIGH);  // Deselect
}

// Print a string at current cursor position
void DisplayManager::print(const char* str) {
    while (*str) {
        if (*str == '\n') {
            cursorX = 0;
            cursorY += 8 * textSize;
        } else if (*str == '\r') {
            cursorX = 0;
        } else {
            drawChar(cursorX, cursorY, *str, textColor, textBgColor, textSize);
            cursorX += 6 * textSize;  // 5 pixels + 1 spacing
        }
        str++;
    }
}

// Print integer
void DisplayManager::print(int value) {
    char buffer[12];
    sprintf(buffer, "%d", value);
    print(buffer);
}

// Print float
void DisplayManager::print(float value, int decimals) {
    char buffer[32];
    dtostrf(value, 1, decimals, buffer);
    print(buffer);
}

void DisplayManager::update() {
    if (sleeping) return;
    if (tft == nullptr) return;  // Safety check

    // Simple frame rate limiting
    // UI Framework calls the show*Screen() functions directly
    // This update() just handles timing
    if (millis() - lastUpdate < 50) {  // 20 FPS
        return;
    }

    lastUpdate = millis();
}

void DisplayManager::clear() {
    fillScreen(COLOR_BACKGROUND);
}

// Manual fillScreen implementation (like bare-bones test)
void DisplayManager::fillScreen(uint16_t color) {
    // Manual SPI fill screen - landscape mode (320x240)
    // Set drawing window to full screen
    writeCommand(0x2A);  // Column address
    writeData16(0);      // Start
    writeData16(319);    // End (320-1) - landscape width

    writeCommand(0x2B);  // Row address
    writeData16(0);      // Start
    writeData16(239);    // End (240-1) - landscape height

    writeCommand(0x2C);  // Memory write

    // Fill entire screen with color
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    for(uint32_t i = 0; i < 320UL*240UL; i++) {
        hspi->transfer16(color);
    }
    digitalWrite(TFT_CS, HIGH);
}

// =============================================================================
// Drawing Primitives (using manual SPI)
// =============================================================================

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;

    // Clip to screen bounds
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;

    // Set drawing window
    writeCommand(0x2A);  // Column address
    writeData16(x);
    writeData16(x + w - 1);

    writeCommand(0x2B);  // Row address
    writeData16(y);
    writeData16(y + h - 1);

    writeCommand(0x2C);  // Memory write

    // Fill rectangle with color
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    for(int32_t i = 0; i < (int32_t)w * h; i++) {
        hspi->transfer16(color);
    }
    digitalWrite(TFT_CS, HIGH);
}

void DisplayManager::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (tft == nullptr) return;
    tft->drawFastHLine(x, y, w, color);
}

void DisplayManager::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (tft == nullptr) return;
    tft->drawFastVLine(x, y, h, color);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (tft == nullptr) return;
    tft->drawRect(x, y, w, h, color);
}

void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (tft == nullptr) return;
    tft->drawLine(x0, y0, x1, y1, color);
}

void DisplayManager::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    if (tft == nullptr) return;
    tft->drawCircle(x0, y0, r, color);
}

void DisplayManager::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    if (tft == nullptr) return;
    tft->fillCircle(x0, y0, r, color);
}

void DisplayManager::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (tft == nullptr) return;
    tft->fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void DisplayManager::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (tft == nullptr) return;
    tft->fillRoundRect(x, y, w, h, r, color);
}

void DisplayManager::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (tft == nullptr) return;
    tft->drawRoundRect(x, y, w, h, r, color);
}

// ============================================================================
// Screen Rendering Functions
// ============================================================================

void DisplayManager::showBootScreen(const char* version) {
    fillScreen(COLOR_BLACK);

    // Large "GAUNTLET" text
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->setTextSize(3);
    drawCenteredText("GAUNTLET", 80, 4);

    // Version text
    tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft->setTextSize(1);
    drawCenteredText(version, 130, 2);

    // Loading animation
    tft->setTextColor(COLOR_GREENYELLOW, COLOR_BLACK);
    drawCenteredText("Initializing...", 160, 2);

    // Draw decorative lines
    drawFastHLine(40, 60, 240, COLOR_CYAN);
    drawFastHLine(40, 62, 240, COLOR_CYAN);
}

void DisplayManager::clearTransitionArtifacts() {
    // Clear entire content area below status bar to prevent overlap
    // Status bar is at Y: 0-20, so clear everything from Y: 20 to 240
    fillRect(0, 20, 320, 220, 0x0000);  // Clear entire content area
}

void DisplayManager::showClockScreen(const char* time, const char* date, float temp, float humidity,
                                    int heartRate,
                                    bool isTransition, int battery, bool bleConnected,
                                    bool wifiConnected, bool gpsLock) {
    // Clear artifacts if transitioning from another screen
    if (isTransition) {
        clearTransitionArtifacts();
    }

    // No full screen clear - just redraw changed areas to prevent flashing
    // Fill background areas not covered by UI elements
    fillRect(0, 30, 320, 20, 0x0000);   // Gap above time
    fillRect(0, 110, 320, 10, 0x0000);  // Gap between time and temp boxes
    fillRect(0, 180, 320, 10, 0x0000);  // Gap between temp boxes and status
    fillRect(0, 195, 320, 45, 0x0000);  // Gap below status line

    // Draw status bar (replaces old title bar)
    drawStatusBar(battery, bleConnected, wifiConnected, gpsLock, "");

    // Screen title below status bar
    textColor = COLOR_CYAN;
    textBgColor = 0x0000;
    textSize = 2;
    cursorX = 90;
    cursorY = 25;
    print("GAUNTLET");

    // Time display area - dark grey rectangle
    fillRect(20, 50, 280, 60, 0x2945);  // Dark grey

    // Draw time (large text)
    textColor = 0xFFFF;  // White
    textBgColor = 0x2945;  // Dark grey
    textSize = 3;
    cursorX = 70;
    cursorY = 60;
    print(time);

    // Draw date (smaller text below time)
    textSize = 2;
    cursorX = 60;
    cursorY = 90;
    print(date);

    // Temperature box - orange/red
    fillRect(20, 120, 140, 60, 0x4208);  // Dark orange

    // Draw temperature label
    textColor = 0xFFFF;
    textBgColor = 0x4208;
    textSize = 1;
    cursorX = 35;
    cursorY = 125;
    print("TEMP");

    // Draw temperature value
    textSize = 2;
    cursorX = 40;
    cursorY = 145;
    char tempBuf[16];
    dtostrf(temp, 4, 1, tempBuf);
    print(tempBuf);
    print("F");

    // Humidity box - blue
    fillRect(170, 120, 130, 60, 0x0208);  // Dark blue

    // Draw humidity label
    textColor = 0xFFFF;
    textBgColor = 0x0208;
    textSize = 1;
    cursorX = 185;
    cursorY = 125;
    print("HUMID");

    // Draw humidity value
    textSize = 2;
    cursorX = 185;
    cursorY = 145;
    print((int)humidity);
    print("%");

    // Heart Rate display at bottom
    textColor = 0xF800;  // Red
    textBgColor = 0x0000;
    textSize = 2;
    cursorX = 100;
    cursorY = 200;

    // Heart icon (simple)
    fillCircle(92, 206, 5, 0xF800);
    fillCircle(100, 206, 5, 0xF800);
    fillTriangle(87, 207, 105, 207, 96, 216, 0xF800);

    // HR value
    if (heartRate > 0) {
        print(" ");
        print(heartRate);
        print(" BPM");
    } else {
        textColor = COLOR_TEXT_DIM;
        print(" --- BPM");
    }
}

void DisplayManager::showSensorScreen(float temp, float hum, float press, float alt, int steps,
                                      bool isTransition, int battery, bool bleConnected,
                                      bool wifiConnected, bool gpsLock) {
    // SENSOR SCREEN - Orange/Yellow theme
    // Clear artifacts if transitioning from another screen
    if (isTransition) {
        clearTransitionArtifacts();
    }

    // No full screen clear - just redraw UI elements to prevent flashing

    // Draw status bar (replaces old title bar)
    drawStatusBar(battery, bleConnected, wifiConnected, gpsLock, "");

    // Screen title below status bar
    textColor = COLOR_ORANGE;
    textBgColor = 0x0000;
    textSize = 2;
    cursorX = 90;
    cursorY = 25;
    print("SENSORS");

    // Temperature box - red
    fillRect(10, 50, 145, 60, 0xF800);  // Red
    textColor = 0xFFFF;
    textBgColor = 0xF800;
    textSize = 1;
    cursorX = 50;
    cursorY = 55;
    print("TEMP");
    textSize = 2;
    cursorX = 30;
    cursorY = 75;
    char tempBuf[16];
    dtostrf(temp, 4, 1, tempBuf);
    print(tempBuf);
    print("C");

    // Humidity box - cyan
    fillRect(165, 50, 145, 60, 0x07FF);  // Cyan
    textColor = 0x0000;  // Black text on cyan
    textBgColor = 0x07FF;
    textSize = 1;
    cursorX = 200;
    cursorY = 55;
    print("HUMID");
    textSize = 2;
    cursorX = 185;
    cursorY = 75;
    print((int)hum);
    print("%");

    // Pressure box - green
    fillRect(10, 120, 145, 60, 0x07E0);  // Green
    textColor = 0x0000;  // Black text on green
    textBgColor = 0x07E0;
    textSize = 1;
    cursorX = 40;
    cursorY = 125;
    print("PRESSURE");
    textSize = 1;
    cursorX = 20;
    cursorY = 145;
    print((int)press);
    print("hPa");

    // Altitude box - yellow
    fillRect(165, 120, 145, 60, 0xFFE0);  // Yellow
    textColor = 0x0000;  // Black text on yellow
    textBgColor = 0xFFE0;
    textSize = 1;
    cursorX = 195;
    cursorY = 125;
    print("ALTITUDE");
    textSize = 2;
    cursorX = 185;
    cursorY = 145;
    print((int)alt);
    print("m");

    // Steps indicator - magenta bar
    fillRect(10, 190, 300, 30, 0xF81F);  // Magenta
    textColor = 0xFFFF;
    textBgColor = 0xF81F;
    textSize = 2;
    cursorX = 50;
    cursorY = 197;
    print("STEPS: ");
    print(steps);
}

void DisplayManager::showGPSScreen(double lat, double lon, float speed, int sats,
                                   bool isTransition, int battery, bool bleConnected,
                                   bool wifiConnected, bool gpsLock) {
    // GPS SCREEN - Green theme
    // Clear artifacts if transitioning from another screen
    if (isTransition) {
        clearTransitionArtifacts();
    }

    // No full screen clear - just redraw UI elements to prevent flashing

    // Draw status bar (replaces old title bar)
    drawStatusBar(battery, bleConnected, wifiConnected, gpsLock, "");

    // Screen title below status bar
    textColor = COLOR_GREEN;
    textBgColor = 0x0000;
    textSize = 2;
    cursorX = 120;
    cursorY = 25;
    print("GPS");

    // Speed display - large yellow box
    fillRect(10, 50, 300, 80, 0xFFE0);  // Yellow
    textColor = 0x0000;  // Black text on yellow
    textBgColor = 0xFFE0;
    textSize = 1;
    cursorX = 130;
    cursorY = 55;
    print("SPEED");
    textSize = 3;
    cursorX = 80;
    cursorY = 85;
    char speedBuffer[16];
    dtostrf(speed, 4, 1, speedBuffer);
    print(speedBuffer);
    textSize = 2;
    print(" km/h");

    // Latitude box - cyan
    fillRect(10, 140, 145, 40, 0x07FF);  // Cyan
    textColor = 0x0000;  // Black text on cyan
    textBgColor = 0x07FF;
    textSize = 1;
    cursorX = 50;
    cursorY = 145;
    print("LAT");
    cursorX = 15;
    cursorY = 160;
    char latBuffer[16];
    dtostrf(lat, 8, 4, latBuffer);
    print(latBuffer);

    // Longitude box - blue
    fillRect(165, 140, 145, 40, 0x001F);  // Blue
    textColor = 0xFFFF;  // White text on blue
    textBgColor = 0x001F;
    textSize = 1;
    cursorX = 205;
    cursorY = 145;
    print("LON");
    cursorX = 170;
    cursorY = 160;
    char lonBuffer[16];
    dtostrf(lon, 8, 4, lonBuffer);
    print(lonBuffer);

    // Satellite indicator - green bars
    fillRect(10, 190, 300, 30, 0x07E0);  // Green bar
    textColor = 0x0000;  // Black text on green
    textBgColor = 0x07E0;
    textSize = 2;
    cursorX = 50;
    cursorY = 197;
    print("SATELLITES: ");
    print(String(sats).c_str());
}

void DisplayManager::showNotificationScreen(int notifCount) {
    // NOTIFICATIONS SCREEN - Purple/Magenta theme
    fillScreen(0x0000);  // Black

    // Title bar - magenta
    fillRect(0, 0, 320, 30, 0xF81F);  // Magenta
    textColor = 0xFFFF;  // White text
    textBgColor = 0xF81F;
    textSize = 2;
    cursorX = 60;
    cursorY = 8;
    print("NOTIFICATIONS");

    if (notifCount == 0) {
        // No notifications - show empty state
        fillRect(50, 100, 220, 60, 0x4208);  // Dark grey
        textColor = 0xFFFF;
        textBgColor = 0x4208;
        textSize = 2;
        cursorX = 70;
        cursorY = 120;
        print("No new");
        cursorX = 55;
        cursorY = 138;
        print("notifications");
    } else {
        // Show notification placeholders
        // Notification 1 - purple box
        fillRect(10, 40, 300, 40, 0x8010);  // Dark purple
        textColor = 0xFFFF;
        textBgColor = 0x8010;
        textSize = 1;
        cursorX = 15;
        cursorY = 45;
        print("Message 1");
        cursorX = 15;
        cursorY = 60;
        print("Tap to view details...");

        // Notification 2 - purple box
        fillRect(10, 90, 300, 40, 0x8010);  // Dark purple
        textColor = 0xFFFF;
        textBgColor = 0x8010;
        textSize = 1;
        cursorX = 15;
        cursorY = 95;
        print("Message 2");
        cursorX = 15;
        cursorY = 110;
        print("Tap to view details...");

        // Notification 3 - purple box
        fillRect(10, 140, 300, 40, 0x8010);  // Dark purple
        textColor = 0xFFFF;
        textBgColor = 0x8010;
        textSize = 1;
        cursorX = 15;
        cursorY = 145;
        print("Message 3");
        cursorX = 15;
        cursorY = 160;
        print("Tap to view details...");

        // Count indicator at bottom
        fillRect(10, 190, 300, 30, 0x4208);  // Dark grey
        textColor = 0xFFFF;
        textBgColor = 0x4208;
        textSize = 2;
        cursorX = 70;
        cursorY = 197;
        print("Total: ");
        print(notifCount);
    }
}

void DisplayManager::showNotificationDetail(const char* title, const char* message, const char* time) {
    if (tft == nullptr) return;  // Safety check

    fillScreen(COLOR_BACKGROUND);

    // Header bar
    fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_STATUS_BAR);
    tft->setTextColor(COLOR_CYAN);
    tft->setTextSize(2);
    tft->setCursor(10, 12);
    tft->print(title);

    // Time
    tft->setTextColor(COLOR_TEXT_DIM);
    tft->setTextSize(1);
    tft->setCursor(SCREEN_WIDTH - 60, 15);
    tft->print(time);

    // Message
    tft->setTextColor(COLOR_WHITE);
    tft->setTextSize(1);
    int y = 50;
    int x = 10;
    int maxWidth = SCREEN_WIDTH - 20;

    // Word wrap
    char word[64];
    int wordLen = 0;
    int cursorX = x;
    int cursorY = y;

    for (int i = 0; message[i] != '\0'; i++) {
        if (message[i] == ' ' || message[i] == '\n') {
            word[wordLen] = '\0';
            int wordWidth = strlen(word) * 6;

            if (cursorX + wordWidth > maxWidth) {
                cursorY += 12;
                cursorX = x;
            }

            tft->setCursor(cursorX, cursorY);
            tft->print(word);
            tft->print(" ");
            cursorX += wordWidth + 6;
            wordLen = 0;

            if (message[i] == '\n') {
                cursorY += 12;
                cursorX = x;
            }
        } else {
            word[wordLen++] = message[i];
        }
    }

    // Print last word
    if (wordLen > 0) {
        word[wordLen] = '\0';
        tft->setCursor(cursorX, cursorY);
        tft->print(word);
    }

    // Action buttons
    drawButton(20, 200, 130, 30, "DISMISS", false, COLOR_RED);
    drawButton(170, 200, 130, 30, "REPLY", false, COLOR_GREEN);
}

void DisplayManager::showMenuScreen(const char** items, int count, int selected) {
    // MENU SCREEN - White/Grey theme
    fillScreen(0x0000);  // Black

    // Title bar - white (smaller)
    fillRect(0, 0, 320, 25, 0xFFFF);  // White, reduced height

    // Title text - MANUAL RENDERING
    textColor = 0x0000;  // Black text on white background
    textBgColor = 0xFFFF;
    textSize = 2;
    cursorX = 115;
    cursorY = 5;
    print("MENU");

    // Draw menu items with text labels - SMALLER ITEMS to fit all 6
    int itemHeight = 32;  // Reduced from 35 to fit all items on screen
    int y = 35;  // Start right after title bar

    for (int i = 0; i < count; i++) {
        // Draw item background
        uint16_t bgColor = (i == selected) ? 0x07FF : 0x8410;  // Cyan if selected, grey otherwise
        fillRect(10, y, 300, itemHeight, bgColor);

        // Draw item text - MANUAL RENDERING
        textColor = 0x0000;  // Black text
        textBgColor = bgColor;
        textSize = 2;
        cursorX = 20;
        cursorY = y + 8;  // Centered vertically in item
        print(items[i]);

        y += itemHeight;
    }
}

void DisplayManager::showVoiceListening(bool listening, const char* text) {
    if (tft == nullptr) return;  // Safety check

    fillScreen(COLOR_BACKGROUND);

    // Animated listening indicator
    int centerX = SCREEN_WIDTH / 2;
    int centerY = 80;

    if (listening) {
        // Animated circles
        static unsigned long lastAnim = 0;
        static int animFrame = 0;

        if (millis() - lastAnim > 100) {
            animFrame = (animFrame + 1) % 10;
            lastAnim = millis();
        }

        for (int i = 0; i < 3; i++) {
            int radius = 20 + (i * 15) + animFrame;
            drawCircle(centerX, centerY, radius, COLOR_CYAN);
        }

        // Microphone icon (simple)
        fillCircle(centerX, centerY, 15, COLOR_GREEN);
    }

    // Status text
    tft->setTextColor(COLOR_WHITE);
    tft->setTextSize(2);
    drawCenteredText(listening ? "Listening..." : "Voice AI Ready", 150, 2);

    // Command text
    if (text != nullptr) {
        tft->setTextColor(COLOR_GREENYELLOW);
        tft->setTextSize(1);
        drawCenteredText(text, 180, 1);
    }
}

void DisplayManager::showDeviceControl(const char** devices, bool* states, int count, int selected) {
    if (tft == nullptr) return;  // Safety check

    fillScreen(COLOR_BACKGROUND);
    drawStatusBar(85, true, true, false, "");

    // Title
    tft->setTextColor(COLOR_ORANGE);
    tft->setTextSize(2);
    tft->setCursor(10, 30);
    tft->print("SMART HOME");

    int y = 70;
    int itemHeight = 40;

    for (int i = 0; i < count && i < 4; i++) {
        bool isSelected = (i == selected);
        bool isOn = states[i];

        // Background
        if (isSelected) {
            fillRoundRect(10, y, 300, itemHeight - 5, 8, COLOR_STATUS_BAR);
        }

        // Device name
        tft->setTextColor(COLOR_WHITE);
        tft->setTextSize(2);
        tft->setCursor(20, y + 10);
        tft->print(devices[i]);

        // Status indicator
        uint16_t statusColor = isOn ? COLOR_GREEN : COLOR_RED;
        fillCircle(280, y + 17, 10, statusColor);

        y += itemHeight;
    }
}

void DisplayManager::showSettingsScreen(SettingsState& settings,
                                       int battery, bool bleConnected,
                                       bool wifiConnected, bool gpsLock) {
    if (tft == nullptr) return;  // Safety check

    fillScreen(COLOR_BACKGROUND);
    drawStatusBar(battery, bleConnected, wifiConnected, gpsLock, "");

    // Title - MANUAL TEXT RENDERING
    textColor = COLOR_CYAN;
    textBgColor = COLOR_BACKGROUND;
    textSize = 2;
    cursorX = 10;
    cursorY = 30;
    print("SETTINGS");

    int y = 60;

    // WiFi section - MANUAL TEXT RENDERING
    textColor = COLOR_WHITE;
    textSize = 1;
    cursorX = 10;
    cursorY = y;
    print("WiFi:");

    drawButton(70, y - 5, 80, 25, settings.wifiEnabled ? "ON" : "OFF", false,
               settings.wifiEnabled ? COLOR_GREEN : COLOR_RED);

    drawButton(160, y - 5, 70, 25, "SCAN", settings.isScanning, COLOR_BLUE);
    y += 35;

    // WiFi network list
    fillRect(10, y, 300, 105, COLOR_BACKGROUND);
    drawRect(10, y, 300, 105, COLOR_DARKGREY);

    if (settings.isScanning) {
        textColor = COLOR_YELLOW;
        textSize = 1;
        cursorX = 100;
        cursorY = y + 50;
        print("Scanning...");
    } else if (settings.networkCount > 0) {
        int listY = y + 5;
        int maxVisible = 3;
        for (int i = settings.scrollOffset;
             i < settings.networkCount && i < settings.scrollOffset + maxVisible;
             i++) {
            drawNetworkListItem(15, listY, settings.networks[i],
                               i == settings.selectedNetworkIndex);
            listY += 32;
        }
    } else {
        textColor = COLOR_TEXT_DIM;
        textSize = 1;
        cursorX = 80;
        cursorY = y + 50;
        print("Tap SCAN");
    }

    y += 110;

    // WiFi connection status - MANUAL TEXT RENDERING
    if (wifiConnected) {
        textColor = COLOR_GREEN;
        textSize = 1;
        cursorX = 10;
        cursorY = y;
        print("Connected: ");
        print(WiFi.SSID().c_str());
    } else if (WiFi.status() == WL_DISCONNECTED && settings.wifiEnabled) {
        textColor = COLOR_YELLOW;
        cursorX = 10;
        cursorY = y;
        print("Disconnected");
    }
    y += 20;

    // Bluetooth section - MANUAL TEXT RENDERING
    textColor = COLOR_WHITE;
    cursorX = 10;
    cursorY = y;
    print("Bluetooth: ");

    if (bleConnected) {
        textColor = COLOR_GREEN;
        print("Connected");
    } else {
        textColor = COLOR_YELLOW;
        print("Advertising");
    }
}

void DisplayManager::drawNetworkListItem(int x, int y, const WiFiNetwork& network, bool selected) {
    // Background highlight if selected
    if (selected) {
        fillRect(x, y, 285, 30, 0x2945);  // Dark highlight
    }

    // SSID - MANUAL TEXT RENDERING
    textColor = COLOR_WHITE;
    textBgColor = selected ? 0x2945 : COLOR_BACKGROUND;
    textSize = 1;
    cursorX = x + 5;
    cursorY = y + 5;
    print(network.ssid);

    // Signal strength - MANUAL TEXT RENDERING
    textColor = COLOR_TEXT_DIM;
    cursorX = x + 200;
    cursorY = y + 5;
    char rssiBuf[8];
    sprintf(rssiBuf, "%ddBm", network.rssi);
    print(rssiBuf);

    // Lock icon (simple text indicator) - MANUAL TEXT RENDERING
    if (network.encrypted) {
        cursorX = x + 185;
        cursorY = y + 5;
        textColor = COLOR_YELLOW;
        print("L");
    }

    // Divider line
    drawFastHLine(x, y + 29, 285, COLOR_DARKGREY);
}

// ============================================================================
// Status Bar
// ============================================================================

void DisplayManager::drawStatusBar(int battery, bool bleConnected, bool wifiConnected,
                                   bool gpsLock, const char* time) {
    // Background
    fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_HEIGHT, COLOR_STATUS_BAR);

    // Battery icon
    drawBatteryIcon(5, 3, battery, COLOR_WHITE);

    // Battery percentage - MANUAL TEXT RENDERING
    textColor = COLOR_WHITE;
    textBgColor = COLOR_STATUS_BAR;
    textSize = 1;
    cursorX = 35;
    cursorY = 6;
    char batteryBuf[8];
    sprintf(batteryBuf, "%d%%", battery);
    print(batteryBuf);

    // Connection icons - ALWAYS show them (grey when disconnected, colored when connected)
    int iconX = SCREEN_WIDTH - 15;

    // GPS icon (rightmost)
    drawGPSIcon(iconX - 15, 4, gpsLock, gpsLock ? COLOR_GREEN : COLOR_DARKGREY);
    iconX -= 20;

    // WiFi icon
    drawWiFiIcon(iconX - 15, 4, wifiConnected, wifiConnected ? COLOR_WHITE : COLOR_DARKGREY);
    iconX -= 20;

    // BLE icon (leftmost)
    drawBLEIcon(iconX - 15, 4, bleConnected, bleConnected ? COLOR_CYAN : COLOR_DARKGREY);
    iconX -= 20;

    // Time (if provided) - MANUAL TEXT RENDERING
    if (time && strlen(time) > 0) {
        textColor = COLOR_WHITE;
        textBgColor = COLOR_STATUS_BAR;
        textSize = 1;
        int timeWidth = strlen(time) * 6;
        cursorX = (SCREEN_WIDTH - timeWidth) / 2;
        cursorY = 6;
        print(time);
    }

    // Border line
    drawFastHLine(0, STATUS_BAR_HEIGHT - 1, SCREEN_WIDTH, COLOR_CYAN);
}

// ============================================================================
// UI Elements
// ============================================================================

void DisplayManager::drawButton(int x, int y, int w, int h, const char* label,
                                bool pressed, uint16_t color) {
    uint16_t bgColor = pressed ? color : COLOR_BACKGROUND;
    uint16_t borderColor = color;
    uint16_t textColor = pressed ? COLOR_WHITE : color;

    // Draw button
    fillRoundRect(x, y, w, h, 8, bgColor);
    drawRoundRect(x, y, w, h, 8, borderColor);
    drawRoundRect(x + 1, y + 1, w - 2, h - 2, 7, borderColor);

    // Draw label (centered)
    tft->setTextColor(textColor);
    tft->setTextSize(2);
    int labelWidth = strlen(label) * 12;
    int labelX = x + (w - labelWidth) / 2;
    int labelY = y + (h - 16) / 2;
    tft->setCursor(labelX, labelY);
    tft->print(label);
}

void DisplayManager::drawProgressBar(int x, int y, int width, int height, int percentage, uint16_t color) {
    // Border
    drawRect(x, y, width, height, COLOR_WHITE);

    // Fill
    int fillWidth = (width - 4) * constrain(percentage, 0, 100) / 100;
    fillRect(x + 2, y + 2, fillWidth, height - 4, color);
}

void DisplayManager::drawSlider(int x, int y, int width, int value, int maxValue) {
    int trackHeight = 8;
    int trackY = y + 4;

    // Track
    fillRoundRect(x, trackY, width, trackHeight, 4, COLOR_DARKGREY);

    // Fill
    int fillWidth = width * value / maxValue;
    fillRoundRect(x, trackY, fillWidth, trackHeight, 4, COLOR_CYAN);

    // Thumb
    int thumbX = x + fillWidth - 6;
    fillCircle(thumbX + 6, trackY + 4, 8, COLOR_WHITE);
}

void DisplayManager::drawGraph(int x, int y, int width, int height, float* data,
                              int points, const char* label) {
    // Border
    drawRect(x, y, width, height, COLOR_WHITE);

    // Label
    if (label) {
        tft->setTextColor(COLOR_WHITE);
        tft->setTextSize(1);
        tft->setCursor(x + 5, y + 5);
        tft->print(label);
    }

    // Find min/max for scaling
    float minVal = data[0];
    float maxVal = data[0];
    for (int i = 1; i < points; i++) {
        if (data[i] < minVal) minVal = data[i];
        if (data[i] > maxVal) maxVal = data[i];
    }

    float range = maxVal - minVal;
    if (range == 0) range = 1;

    // Draw data points
    int plotWidth = width - 10;
    int plotHeight = height - 20;
    int plotX = x + 5;
    int plotY = y + 15;

    for (int i = 0; i < points - 1; i++) {
        int x1 = plotX + (i * plotWidth / (points - 1));
        int y1 = plotY + plotHeight - ((data[i] - minVal) * plotHeight / range);
        int x2 = plotX + ((i + 1) * plotWidth / (points - 1));
        int y2 = plotY + plotHeight - ((data[i + 1] - minVal) * plotHeight / range);

        drawLine(x1, y1, x2, y2, COLOR_GREEN);
    }
}

void DisplayManager::drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
    drawRoundRect(x, y, w, h, r, color);
}

void DisplayManager::drawIcon(int x, int y, const char* iconName, uint16_t color) {
    // Simple text-based icons for now
    tft->setTextColor(color);
    tft->setTextSize(1);
    tft->setCursor(x, y);

    if (strcmp(iconName, "battery") == 0) {
        tft->print("[=]");
    } else if (strcmp(iconName, "wifi") == 0) {
        tft->print("(W)");
    } else if (strcmp(iconName, "bluetooth") == 0) {
        tft->print("<B>");
    } else if (strcmp(iconName, "gps") == 0) {
        tft->print("(*)");
    }
}

// ============================================================================
// Icon Drawing Helpers
// ============================================================================

void DisplayManager::drawBatteryIcon(int x, int y, int percentage, uint16_t color) {
    int w = 24;
    int h = 12;

    // Battery body
    drawRect(x, y, w, h, color);

    // Battery terminal
    fillRect(x + w, y + 3, 2, h - 6, color);

    // Fill level
    int fillWidth = (w - 4) * constrain(percentage, 0, 100) / 100;
    uint16_t fillColor = percentage > 20 ? COLOR_GREEN : COLOR_RED;
    fillRect(x + 2, y + 2, fillWidth, h - 4, fillColor);
}

void DisplayManager::drawWiFiIcon(int x, int y, bool connected, uint16_t color) {
    // WiFi bars - 3 bars getting taller (larger and more visible)
    fillRect(x, y + 9, 4, 5, color);        // Short bar
    fillRect(x + 5, y + 6, 4, 8, color);    // Medium bar
    fillRect(x + 10, y + 2, 4, 12, color);  // Tall bar
}

void DisplayManager::drawBLEIcon(int x, int y, bool connected, uint16_t color) {
    // Bluetooth symbol - larger and thicker
    drawLine(x + 6, y + 1, x + 6, y + 13, color);
    drawLine(x + 7, y + 1, x + 7, y + 13, color);  // Thicker vertical line
    drawLine(x + 6, y + 1, x + 11, y + 5, color);
    drawLine(x + 11, y + 5, x + 6, y + 7, color);
    drawLine(x + 6, y + 7, x + 11, y + 9, color);
    drawLine(x + 11, y + 9, x + 6, y + 13, color);
}

void DisplayManager::drawGPSIcon(int x, int y, bool locked, uint16_t color) {
    // Location pin - larger
    fillCircle(x + 6, y + 5, 5, color);
    fillCircle(x + 6, y + 5, 3, COLOR_STATUS_BAR);  // Center hole
    fillTriangle(x + 6, y + 10, x + 3, y + 14, x + 9, y + 14, color);  // Pin point
}

void DisplayManager::drawListItem(int y, const char* text, bool selected) {
    if (selected) {
        fillRect(0, y, SCREEN_WIDTH, 30, COLOR_BLUE);
        tft->setTextColor(COLOR_BLACK);
    } else {
        tft->setTextColor(COLOR_WHITE);
    }

    tft->setTextSize(2);
    tft->setCursor(10, y + 6);
    tft->print(text);
}

// ============================================================================
// Text Helpers
// ============================================================================

void DisplayManager::drawCenteredText(const char* text, int y, uint8_t font) {
    tft->setTextSize(font);
    int textWidth = strlen(text) * 6 * font;
    int x = (SCREEN_WIDTH - textWidth) / 2;
    tft->setCursor(x, y);
    tft->print(text);
}

void DisplayManager::drawRightAlignedText(const char* text, int x, int y, uint8_t font) {
    tft->setTextSize(font);
    int textWidth = strlen(text) * 6 * font;
    tft->setCursor(x - textWidth, y);
    tft->print(text);
}

// ============================================================================
// State Management
// ============================================================================

void DisplayManager::setState(DisplayState state) {
    currentState = state;
}

DisplayState DisplayManager::getState() {
    return currentState;
}

void DisplayManager::setBrightness(uint8_t bright) {
    brightness = bright;
    if (!sleeping) {
        analogWrite(TFT_BL, brightness);
    }
}

uint8_t DisplayManager::getBrightness() {
    return brightness;
}

void DisplayManager::sleep() {
    sleeping = true;
    analogWrite(TFT_BL, 0);  // Turn off backlight
    Serial.println("Display sleeping");
}

void DisplayManager::wake() {
    sleeping = false;
    analogWrite(TFT_BL, brightness);
    Serial.println("Display awake");
}

bool DisplayManager::isSleeping() {
    return sleeping;
}

void DisplayManager::setRotation(uint8_t rotation) {
    tft->setRotation(rotation);
}

void DisplayManager::renderCurrentState() {
    // This is called automatically by update()
    // Individual screens are rendered by explicit calls to show*() functions
}

uint16_t DisplayManager::interpolateColor(uint16_t color1, uint16_t color2, float ratio) {
    // Extract RGB components
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;

    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    // Interpolate
    uint8_t r = r1 + (r2 - r1) * ratio;
    uint8_t g = g1 + (g2 - g1) * ratio;
    uint8_t b = b1 + (b2 - b1) * ratio;

    // Combine back to RGB565
    return (r << 11) | (g << 5) | b;
}

void DisplayManager::showHealthScreen(int heartRate, int spo2, bool fingerDetected, 
                                     uint8_t signalQuality,
                                     bool isTransition, int battery, bool bleConnected,
                                     bool wifiConnected, bool gpsLock) {
    // Clear artifacts if transitioning
    if (isTransition) {
        fillRect(0, 20, 320, 220, 0x0000);
    }

    // Draw status bar
    drawStatusBar(battery, bleConnected, wifiConnected, gpsLock, "");

    // Title
    textColor = COLOR_CYAN;
    textBgColor = 0x0000;
    textSize = 2;
    cursorX = 70;
    cursorY = 25;
    print("HEALTH MONITOR");

    if (!fingerDetected) {
        // Show "Place finger" message
        textColor = COLOR_YELLOW;
        textSize = 2;
        cursorX = 35;
        cursorY = 100;
        print("Place finger");
        
        cursorX = 45;
        cursorY = 120;
        print("on sensor");

        // Draw simple finger icon
        fillCircle(160, 160, 15, COLOR_YELLOW);
        fillRect(155, 160, 10, 20, COLOR_YELLOW);
        return;
    }

    // ========== HEART RATE SECTION ==========
    int y = 55;

    // Draw animated heart icon
    static bool heartBeat = false;
    static unsigned long lastBeat = 0;
    if (heartRate > 0 && millis() - lastBeat > (60000 / heartRate)) {
        heartBeat = !heartBeat;
        lastBeat = millis();
    }

    uint16_t heartColor = heartBeat ? 0xF800 : 0xC800;  // Bright red / Dark red
    int heartSize = heartBeat ? 10 : 8;

    // Simple heart shape using circles and triangle
    fillCircle(25 - heartSize/2, y + 8, heartSize, heartColor);
    fillCircle(35 + heartSize/2, y + 8, heartSize, heartColor);
    fillTriangle(15, y + 10, 45, y + 10, 30, y + 25, heartColor);

    // Heart Rate BPM - Large display
    textColor = COLOR_WHITE;
    textBgColor = 0x0000;
    textSize = 4;
    cursorX = 55;
    cursorY = y;
    
    if (heartRate > 0) {
        char hrBuf[8];
        sprintf(hrBuf, "%3d", heartRate);
        print(hrBuf);
    } else {
        print("---");
    }

    // "BPM" label
    textSize = 2;
    cursorX = 180;
    cursorY = y + 8;
    print("BPM");

    // HR Status bar and label
    y += 35;
    int barWidth = map(constrain(heartRate, 40, 120), 40, 120, 0, 280);
    uint16_t barColor = COLOR_GREEN;
    if (heartRate > 100 || heartRate < 60) barColor = COLOR_YELLOW;
    if (heartRate > 120 || heartRate < 50) barColor = 0xF800;  // Red
    
    fillRect(20, y, barWidth, 8, barColor);
    fillRect(20 + barWidth, y, 280 - barWidth, 8, 0x2945);  // Dark grey remainder

    // HR Status text
    textSize = 1;
    textColor = barColor;
    cursorX = 20;
    cursorY = y + 12;
    
    if (heartRate >= 60 && heartRate <= 100) {
        print("Normal - Resting");
    } else if (heartRate > 100 && heartRate <= 120) {
        print("Elevated - Active");
    } else if (heartRate > 120) {
        print("High - Exercise");
    } else if (heartRate < 60 && heartRate > 40) {
        print("Low - Very Fit");
    } else {
        print("Unusual");
    }

    // ========== LIVE GRAPH SECTION ==========
    y += 28;
    
    // Graph title
    textColor = COLOR_CYAN;
    textSize = 1;
    cursorX = 20;
    cursorY = y;
    print("Heart Rate Trend");

    y += 12;
    
    // Graph box
    int graphX = 20;
    int graphY = y;
    int graphW = 280;
    int graphH = 50;
    
    drawRect(graphX, graphY, graphW, graphH, COLOR_DARKGREY);

    // Draw live graph (scrolling line chart)
    static int hrHistory[140] = {0};  // Store last 140 readings (280px / 2)
    static int historyIndex = 0;

    // Add current reading to history
    if (heartRate > 0) {
        hrHistory[historyIndex] = heartRate;
        historyIndex = (historyIndex + 1) % 140;
    }

    // Draw graph lines
    for (int i = 1; i < 140; i++) {
        int idx1 = (historyIndex + i - 1) % 140;
        int idx2 = (historyIndex + i) % 140;
        
        if (hrHistory[idx1] > 0 && hrHistory[idx2] > 0) {
            int x1 = graphX + 2 + (i - 1) * 2;
            int x2 = graphX + 2 + i * 2;
            
            // Map HR to graph height (40-120 BPM range)
            int y1 = graphY + graphH - 4 - map(constrain(hrHistory[idx1], 40, 120), 40, 120, 0, graphH - 8);
            int y2 = graphY + graphH - 4 - map(constrain(hrHistory[idx2], 40, 120), 40, 120, 0, graphH - 8);
            
            drawLine(x1, y1, x2, y2, 0xF800);  // Red line
        }
    }

    // ========== SPO2 SECTION ==========
    y += graphH + 15;

    // O2 icon (simple circle)
    textColor = 0x07FF;  // Cyan
    textSize = 2;
    cursorX = 20;
    cursorY = y;
    print("O");
    
    textSize = 1;
    cursorX = 32;
    cursorY = y + 6;
    print("2");

    // SpO2 percentage
    textColor = COLOR_WHITE;
    textSize = 3;
    cursorX = 50;
    cursorY = y;
    
    if (spo2 > 0) {
        char spo2Buf[8];
        sprintf(spo2Buf, "%3d%%", spo2);
        print(spo2Buf);
    } else {
        print("--%");
    }

    // SpO2 bar
    int spo2BarWidth = map(constrain(spo2, 85, 100), 85, 100, 0, 150);
    uint16_t spo2Color = COLOR_GREEN;
    if (spo2 < 95 && spo2 >= 90) spo2Color = COLOR_YELLOW;
    if (spo2 < 90) spo2Color = 0xF800;  // Red

    fillRect(150, y + 5, spo2BarWidth, 12, spo2Color);
    fillRect(150 + spo2BarWidth, y + 5, 150 - spo2BarWidth, 12, 0x2945);

    // ========== SIGNAL QUALITY SECTION ==========
    y += 28;
    
    textColor = COLOR_TEXT_DIM;
    textSize = 1;
    cursorX = 20;
    cursorY = y;
    print("Signal:");

    // Signal quality bars
    int barCount = signalQuality / 20;  // 0-5 bars
    for (int i = 0; i < 5; i++) {
        uint16_t color = (i < barCount) ? COLOR_GREEN : 0x2945;
        fillRect(70 + i * 10, y + 2 - i * 2, 6, 6 + i * 2, color);
    }

    // Signal quality text
    cursorX = 140;
    cursorY = y;
    textColor = (signalQuality > 60) ? COLOR_GREEN : COLOR_YELLOW;
    
    if (signalQuality > 80) {
        print("Excellent");
    } else if (signalQuality > 60) {
        print("Good");
    } else if (signalQuality > 40) {
        print("Fair");
    } else {
        print("Poor");
    }
}
