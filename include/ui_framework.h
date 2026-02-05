#ifndef UI_FRAMEWORK_H
#define UI_FRAMEWORK_H

#include <Arduino.h>
#include "display_manager.h"
#include "touch_manager.h"
#include "sensor_manager.h"

// UI Screen IDs
enum ScreenID {
    SCREEN_BOOT,
    SCREEN_CLOCK,
    SCREEN_SENSORS,
    SCREEN_GPS,
    SCREEN_HEALTH,
    SCREEN_NOTIFICATIONS,
    SCREEN_MENU,
    SCREEN_VOICE,
    SCREEN_SMART_HOME,
    SCREEN_SETTINGS
};

// Navigation direction
enum NavDirection {
    NAV_NONE,
    NAV_NEXT,
    NAV_PREV,
    NAV_SELECT,
    NAV_BACK
};

// UI State
struct UIState {
    ScreenID currentScreen;
    ScreenID previousScreen;
    int selectedIndex;
    bool needsRedraw;
    unsigned long lastInteraction;
    unsigned long lastScreenChange;
    bool isScreenTransition;  // True on first render after screen change
};

// WiFi Network Info
struct WiFiNetwork {
    char ssid[32];
    int8_t rssi;
    bool encrypted;
};

// Settings State
struct SettingsState {
    WiFiNetwork networks[10];
    int networkCount;
    int selectedNetworkIndex;
    int scrollOffset;
    bool isScanning;
    bool wifiEnabled;
    bool bleEnabled;
    int brightness;
};

class UIFramework {
public:
    UIFramework();
    void begin();
    void update();

    // Screen navigation
    void navigateTo(ScreenID screen);
    void navigateBack();
    void navigateNext();
    void navigatePrev();

    // Input handling
    void handleTouch();
    void handleButtons();
    void handleSwipe(SwipeDirection direction);

    // Screen state
    ScreenID getCurrentScreen();
    void setNeedsRedraw();
    bool needsRedraw();

    // Interaction tracking
    void resetIdleTimer();
    bool isIdle(unsigned long timeoutMs = 30000);

    // Menu management
    void setMenuSelection(int index);
    int getMenuSelection();

private:
    UIState state;
    SettingsState settingsState;

    // Screen-specific handlers
    void handleClockScreen();
    void handleSensorScreen();
    void handleGPSScreen();
    void handleHealthScreen();
    void handleNotificationScreen();
    void handleMenuScreen();
    void handleSmartHomeScreen();
    void handleSettingsScreen();

    // Touch zones for different screens
    bool isTouchInZone(int zoneX, int zoneY, int zoneW, int zoneH);

    // Settings helpers
    void performWiFiScan();
    void handleNetworkSelection(int index);

    // Constants
    const int BUTTON_HEIGHT = 40;
    const int LIST_ITEM_HEIGHT = 32;  // Reduced from 35 to fit 6 menu items on screen
};

extern UIFramework uiFramework;

#endif // UI_FRAMEWORK_H
