#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

// Forward declarations
struct SettingsState;
struct WiFiNetwork;

// Display states
enum DisplayState {
    DISPLAY_BOOT,
    DISPLAY_CLOCK,
    DISPLAY_SENSORS,
    DISPLAY_NOTIFICATIONS,
    DISPLAY_GPS,
    DISPLAY_MENU,
    DISPLAY_VOICE_LISTENING,
    DISPLAY_DEVICE_CONTROL,
    DISPLAY_SETTINGS
};

// TFT Color definitions (RGB565)
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_YELLOW      0xFFE0
#define COLOR_ORANGE      0xFD20
#define COLOR_GREENYELLOW 0xAFE5
#define COLOR_DARKGREY    0x7BEF
#define COLOR_LIGHTGREY   0xC618
#define COLOR_BACKGROUND  0x0841  // Dark blue-grey
#define COLOR_STATUS_BAR  0x1082  // Darker blue-grey
#define COLOR_TEXT        0xFFFF  // White
#define COLOR_TEXT_DIM    0x8410  // Grey

// Button structure
struct Button {
    int x;
    int y;
    int w;
    int h;
    const char* label;
    bool pressed;
    uint16_t color;
};

class DisplayManager {
public:
    DisplayManager();
    bool begin();
    void update();
    void clear();
    void fillScreen(uint16_t color);

    // Screen rendering
    void showBootScreen(const char* version);
    void showClockScreen(const char* time, const char* date, float temp, float humidity,
                         int heartRate,
                         bool isTransition, int battery, bool bleConnected,
                         bool wifiConnected, bool gpsLock);
    void showSensorScreen(float temp, float hum, float press, float alt, int steps,
                          bool isTransition, int battery, bool bleConnected,
                          bool wifiConnected, bool gpsLock);
    void showNotificationScreen(int notifCount);
    void showNotificationDetail(const char* title, const char* message, const char* time);
    void showGPSScreen(double lat, double lon, float speed, int sats,
                       bool isTransition, int battery, bool bleConnected,
                       bool wifiConnected, bool gpsLock);
    void showHealthScreen(int heartRate, int spo2, bool fingerDetected, uint8_t signalQuality,
                         bool isTransition, int battery, bool bleConnected,
                         bool wifiConnected, bool gpsLock);
    void showMenuScreen(const char** items, int count, int selected);
    void showVoiceListening(bool listening, const char* text = nullptr);
    void showDeviceControl(const char** devices, bool* states, int count, int selected);
    void showSettingsScreen(SettingsState& settings, int battery, bool bleConnected,
                           bool wifiConnected, bool gpsLock);

    // Status bar
    void drawStatusBar(int battery, bool bleConnected, bool wifiConnected, bool gpsLock, const char* time);

    // UI elements
    void drawButton(int x, int y, int w, int h, const char* label, bool pressed, uint16_t color = COLOR_BLUE);
    void drawProgressBar(int x, int y, int width, int height, int percentage, uint16_t color = COLOR_GREEN);
    void drawSlider(int x, int y, int width, int value, int maxValue);
    void drawGraph(int x, int y, int width, int height, float* data, int points, const char* label);
    void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color);
    void drawIcon(int x, int y, const char* iconName, uint16_t color);

    // Text rendering
    void drawCenteredText(const char* text, int y, uint8_t font = 2);
    void drawRightAlignedText(const char* text, int x, int y, uint8_t font = 2);

    // State management
    void setState(DisplayState state);
    DisplayState getState();
    void setBrightness(uint8_t brightness);  // 0-255
    uint8_t getBrightness();

    // Screen management
    void sleep();
    void wake();
    bool isSleeping();
    void setRotation(uint8_t rotation);

private:
    TFT_eSPI* tft;
    SPIClass* hspi;  // HSPI bus for display (like bare-bones test)
    DisplayState currentState;
    unsigned long lastUpdate;
    uint8_t brightness;
    bool sleeping;

    // Text rendering state
    int16_t cursorX;
    int16_t cursorY;
    uint16_t textColor;
    uint16_t textBgColor;
    uint8_t textSize;

    // Status bar height
    const int STATUS_BAR_HEIGHT = 20;

    // Helper functions
    void renderCurrentState();
    void clearTransitionArtifacts();  // Clear regions that may have artifacts from previous screen
    void drawNetworkListItem(int x, int y, const WiFiNetwork& network, bool selected);  // Draw WiFi network list item
    void customILI9341Init();  // Custom initialization bypassing TFT_eSPI init
    void writeCommand(uint8_t cmd);  // Manual SPI command (bare-bones style)
    void writeData(uint8_t data);    // Manual SPI data (bare-bones style)
    void writeData16(uint16_t data); // Manual SPI 16-bit data
    void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);  // Set drawing window
    void drawPixelFast(int16_t x, int16_t y, uint16_t color);  // Fast pixel draw (no window set)

    // Manual drawing primitives (replacing TFT_eSPI functions)
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawBatteryIcon(int x, int y, int percentage, uint16_t color);
    void drawWiFiIcon(int x, int y, bool connected, uint16_t color);
    void drawBLEIcon(int x, int y, bool connected, uint16_t color);
    void drawGPSIcon(int x, int y, bool locked, uint16_t color);
    void drawListItem(int y, const char* text, bool selected);
    uint16_t interpolateColor(uint16_t color1, uint16_t color2, float ratio);

    // Manual text rendering (5x7 bitmap font)
    void drawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
    void print(const char* str);
    void print(int value);
    void print(float value, int decimals = 2);
};

extern DisplayManager displayMgr;

#endif // DISPLAY_MANAGER_H
