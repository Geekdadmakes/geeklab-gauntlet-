#include "ui_framework.h"
#include "sensor_manager.h"
#include "power_manager.h"
#include "bluetooth_service.h"
#include <WiFi.h>

UIFramework uiFramework;

UIFramework::UIFramework() {
    state.currentScreen = SCREEN_BOOT;
    state.previousScreen = SCREEN_BOOT;
    state.selectedIndex = 0;
    state.needsRedraw = true;
    state.lastInteraction = 0;
    state.lastScreenChange = 0;
    state.isScreenTransition = false;
}

void UIFramework::begin() {
    Serial.println("UI Framework initialized");
    state.currentScreen = SCREEN_CLOCK;  // Start with clock screen
    state.needsRedraw = true;  // Trigger initial redraw
    resetIdleTimer();
}

void UIFramework::update() {
    // Handle button inputs
    handleButtons();

    // Handle touch inputs
    if (touchMgr.isTouched()) {
        handleTouch();
        resetIdleTimer();
    }

    // Handle swipe gestures
    SwipeDirection swipe = touchMgr.detectSwipe();
    if (swipe != SWIPE_NONE) {
        handleSwipe(swipe);
        resetIdleTimer();
    }

    // Update current screen
    switch (state.currentScreen) {
        case SCREEN_CLOCK:
            handleClockScreen();
            break;
        case SCREEN_SENSORS:
            handleSensorScreen();
            break;
        case SCREEN_GPS:
            handleGPSScreen();
            break;
        case SCREEN_HEALTH:
            handleHealthScreen();
            break;
        case SCREEN_NOTIFICATIONS:
            handleNotificationScreen();
            break;
        case SCREEN_MENU:
            handleMenuScreen();
            break;
        case SCREEN_SMART_HOME:
            handleSmartHomeScreen();
            break;
        case SCREEN_SETTINGS:
            handleSettingsScreen();
            break;
        default:
            break;
    }

    // Auto-sleep if idle
    if (isIdle(60000)) {  // 60 seconds
        if (!displayMgr.isSleeping()) {
            displayMgr.sleep();
        }
    }
}

void UIFramework::navigateTo(ScreenID screen) {
    if (screen != state.currentScreen) {
        state.previousScreen = state.currentScreen;
        state.currentScreen = screen;
        state.selectedIndex = 0;
        state.needsRedraw = true;
        state.lastScreenChange = millis();
        state.isScreenTransition = true;  // Mark as screen transition

        Serial.printf("Navigated to screen: %d\n", screen);
    } else {
        // Force redraw even if same screen (for time updates)
        state.needsRedraw = true;
        state.isScreenTransition = false;  // Not a transition, just refresh
    }
}

void UIFramework::navigateBack() {
    navigateTo(state.previousScreen);
}

void UIFramework::navigateNext() {
    // Cycle through main screens: Clock → Sensors → GPS → Health → Notifications
    switch (state.currentScreen) {
        case SCREEN_CLOCK:
            navigateTo(SCREEN_SENSORS);
            break;
        case SCREEN_SENSORS:
            navigateTo(SCREEN_GPS);
            break;
        case SCREEN_GPS:
            navigateTo(SCREEN_HEALTH);
            break;
        case SCREEN_HEALTH:
            navigateTo(SCREEN_NOTIFICATIONS);
            break;
        case SCREEN_NOTIFICATIONS:
            navigateTo(SCREEN_CLOCK);
            break;
        default:
            navigateTo(SCREEN_CLOCK);
            break;
    }
}

void UIFramework::navigatePrev() {
    // Cycle backwards through main screens
    switch (state.currentScreen) {
        case SCREEN_CLOCK:
            navigateTo(SCREEN_NOTIFICATIONS);
            break;
        case SCREEN_NOTIFICATIONS:
            navigateTo(SCREEN_HEALTH);
            break;
        case SCREEN_HEALTH:
            navigateTo(SCREEN_GPS);
            break;
        case SCREEN_GPS:
            navigateTo(SCREEN_SENSORS);
            break;
        case SCREEN_SENSORS:
            navigateTo(SCREEN_CLOCK);
            break;
        default:
            navigateTo(SCREEN_CLOCK);
            break;
    }
}

void UIFramework::handleTouch() {
    TouchPoint point = touchMgr.getPoint();

    // Wake display if sleeping
    if (displayMgr.isSleeping()) {
        displayMgr.wake();
        return;
    }

    // Common touch zones (status bar)
    if (isTouchInZone(0, 0, SCREEN_WIDTH, 20)) {
        // Tapped status bar - show menu
        navigateTo(SCREEN_MENU);
        return;
    }

    // Screen-specific touch handling
    switch (state.currentScreen) {
        case SCREEN_CLOCK:
            // Tap anywhere to show menu
            if (point.y > 100) {
                navigateTo(SCREEN_MENU);
            }
            break;

        case SCREEN_MENU: {
            // Check which menu item was tapped
            int itemY = 35;  // Match new menu layout (starts right after 25px title bar)
            const char* menuItems[] = {"Clock", "Sensors", "GPS", "Notifications", "Smart Home", "Settings"};
            const ScreenID screens[] = {SCREEN_CLOCK, SCREEN_SENSORS, SCREEN_GPS, SCREEN_NOTIFICATIONS, SCREEN_SMART_HOME, SCREEN_SETTINGS};
            int itemCount = 6;

            TouchPoint point = touchMgr.getPoint();
            Serial.printf("Menu touch at: X=%d, Y=%d\n", point.x, point.y);

            for (int i = 0; i < itemCount; i++) {
                if (isTouchInZone(0, itemY, SCREEN_WIDTH, LIST_ITEM_HEIGHT)) {
                    Serial.printf("Tapped item %d (%s) at Y=%d-%d\n", i, menuItems[i], itemY, itemY + LIST_ITEM_HEIGHT);
                    navigateTo(screens[i]);
                    return;
                }
                itemY += LIST_ITEM_HEIGHT;
            }
            Serial.println("No menu item matched");
            break;
        }

        case SCREEN_SETTINGS: {
            TouchPoint point = touchMgr.getPoint();

            // WiFi toggle button (70, 55, 80, 25)
            if (isTouchInZone(70, 55, 80, 25)) {
                settingsState.wifiEnabled = !settingsState.wifiEnabled;
                if (settingsState.wifiEnabled) {
                    WiFi.mode(WIFI_STA);
                } else {
                    WiFi.disconnect(true);
                    WiFi.mode(WIFI_OFF);
                }
                state.needsRedraw = true;
            }
            // Scan button (160, 55, 70, 25)
            else if (isTouchInZone(160, 55, 70, 25)) {
                performWiFiScan();
            }
            // Network list area (10, 95, 300, 105)
            else if (isTouchInZone(10, 95, 300, 105)) {
                int itemIndex = (point.y - 100) / 32 + settingsState.scrollOffset;
                if (itemIndex >= 0 && itemIndex < settingsState.networkCount) {
                    handleNetworkSelection(itemIndex);
                }
            }
            break;
        }

        default:
            break;
    }
}

void UIFramework::handleButtons() {
    // Note: Physical buttons not implemented yet
    // Will read GPIO pins for BUTTON_1, BUTTON_2, BUTTON_3
}

void UIFramework::handleSwipe(SwipeDirection direction) {
    Serial.printf("Swipe detected: %d\n", direction);

    switch (direction) {
        case SWIPE_LEFT:
            navigateNext();
            break;
        case SWIPE_RIGHT:
            navigatePrev();
            break;
        case SWIPE_UP:
            // Could be used for other actions
            break;
        case SWIPE_DOWN:
            // Show menu or go back
            if (state.currentScreen != SCREEN_CLOCK) {
                navigateBack();
            } else {
                navigateTo(SCREEN_MENU);
            }
            break;
        default:
            break;
    }
}

void UIFramework::handleClockScreen() {
    // Read current time from RTC (hardware)
    DateTime now = sensorMgr.getCurrentTime();

    if (state.needsRedraw) {
        // Convert to 12-hour format with AM/PM
        int hour12 = now.hour();
        const char* ampm = "AM";

        if (hour12 == 0) {
            hour12 = 12;  // Midnight = 12 AM
        } else if (hour12 == 12) {
            ampm = "PM";  // Noon = 12 PM
        } else if (hour12 > 12) {
            hour12 -= 12;  // 13:00 = 1 PM
            ampm = "PM";
        }

        char timeStr[12];
        sprintf(timeStr, "%2d:%02d %s", hour12, now.minute(), ampm);

        char dateStr[16];
        sprintf(dateStr, "%02d/%02d/%04d", now.month(), now.day(), now.year());

        // Get real environmental data from BME280
        float tempC = sensorMgr.getTemperature();
        float tempF = (tempC * 9.0 / 5.0) + 32.0;  // Convert C to F
        float humidity = sensorMgr.getHumidity();

        // Get heart rate from health sensor
        int heartRate = sensorMgr.getHeartRate();

        // Get status data for status bar
        int batteryPct = powerMgr.getBatteryPercentage();
        bool bleConnected = bluetooth.isConnected();
        bool wifiConnected = (WiFi.status() == WL_CONNECTED);
        bool gpsLock = sensorMgr.hasGPSFix();

        // Pass all data including transition flag and status
        displayMgr.showClockScreen(timeStr, dateStr, tempF, humidity, heartRate,
                                   state.isScreenTransition,
                                   batteryPct, bleConnected, wifiConnected, gpsLock);

        state.needsRedraw = false;
        state.isScreenTransition = false;  // Clear transition flag after rendering
    }

    // Auto-refresh every 10 seconds for time updates
    static unsigned long lastClockUpdate = 0;
    if (millis() - lastClockUpdate > 10000) {
        state.needsRedraw = true;
        state.isScreenTransition = false;  // Auto-refresh is not a transition
        lastClockUpdate = millis();
    }
}

void UIFramework::handleSensorScreen() {
    if (state.needsRedraw) {
        // Get real environmental data from BME280 (or use defaults if unavailable)
        float tempC = sensorMgr.getTemperature();
        float tempF = (tempC * 9.0 / 5.0) + 32.0;  // Convert C to F
        float humidity = sensorMgr.getHumidity();
        float pressure = sensorMgr.getPressure();
        float altitudeM = sensorMgr.getAltitude();
        float altitudeFt = altitudeM * 3.28084;  // Convert meters to feet

        // Get real step count from IMU
        int steps = sensorMgr.getStepCount();

        // Get status data for status bar
        int batteryPct = powerMgr.getBatteryPercentage();
        bool bleConnected = bluetooth.isConnected();
        bool wifiConnected = (WiFi.status() == WL_CONNECTED);
        bool gpsLock = sensorMgr.hasGPSFix();

        // Pass all data including transition flag and status
        displayMgr.showSensorScreen(tempF, humidity, pressure, altitudeFt, steps,
                                    state.isScreenTransition,
                                    batteryPct, bleConnected, wifiConnected, gpsLock);

        state.needsRedraw = false;
        state.isScreenTransition = false;  // Clear transition flag after rendering
    }

    // Auto-refresh every 10 seconds to update environmental data
    static unsigned long lastSensorUpdate = 0;
    if (millis() - lastSensorUpdate > 10000) {
        state.needsRedraw = true;
        state.isScreenTransition = false;  // Auto-refresh is not a transition
        lastSensorUpdate = millis();
    }
}

void UIFramework::handleGPSScreen() {
    if (state.needsRedraw) {
        // Get real GPS data from sensor manager
        double latitude = sensorMgr.getLatitude();
        double longitude = sensorMgr.getLongitude();
        float speed = sensorMgr.getSpeed();
        int satellites = sensorMgr.getSatellites();

        // Get status data for status bar
        int batteryPct = powerMgr.getBatteryPercentage();
        bool bleConnected = bluetooth.isConnected();
        bool wifiConnected = (WiFi.status() == WL_CONNECTED);
        bool gpsLock = sensorMgr.hasGPSFix();

        // Pass all data including transition flag and status
        displayMgr.showGPSScreen(latitude, longitude, speed, satellites,
                                 state.isScreenTransition,
                                 batteryPct, bleConnected, wifiConnected, gpsLock);

        state.needsRedraw = false;
        state.isScreenTransition = false;  // Clear transition flag after rendering
    }

    // Auto-refresh every 5 seconds to update GPS data
    static unsigned long lastGPSUpdate = 0;
    if (millis() - lastGPSUpdate > 5000) {
        state.needsRedraw = true;
        state.isScreenTransition = false;  // Auto-refresh is not a transition
        lastGPSUpdate = millis();
    }
}

void UIFramework::handleNotificationScreen() {
    if (state.needsRedraw) {
        // TODO: Get notification count from notification manager
        int notifCount = 0;

        displayMgr.showNotificationScreen(notifCount);
        state.needsRedraw = false;
    }
}

void UIFramework::handleMenuScreen() {
    if (state.needsRedraw) {
        const char* menuItems[] = {
            "Clock",
            "Sensors",
            "GPS",
            "Notifications",
            "Smart Home",
            "Settings"
        };

        displayMgr.showMenuScreen(menuItems, 6, state.selectedIndex);
        state.needsRedraw = false;
    }
}

void UIFramework::handleSmartHomeScreen() {
    if (state.needsRedraw) {
        const char* devices[] = {
            "Living Room",
            "Bedroom",
            "Kitchen",
            "Office"
        };

        bool states[] = {true, false, true, false};

        displayMgr.showDeviceControl(devices, states, 4, state.selectedIndex);
        state.needsRedraw = false;
    }
}

void UIFramework::handleSettingsScreen() {
    if (state.needsRedraw) {
        // Update settings state
        settingsState.wifiEnabled = (WiFi.getMode() != WIFI_OFF);
        settingsState.bleEnabled = bluetooth.isConnected();
        settingsState.brightness = displayMgr.getBrightness();

        // Get status data
        int batteryPct = powerMgr.getBatteryPercentage();
        bool bleConnected = bluetooth.isConnected();
        bool wifiConnected = (WiFi.status() == WL_CONNECTED);
        bool gpsLock = sensorMgr.hasGPSFix();

        displayMgr.showSettingsScreen(settingsState, batteryPct, bleConnected,
                                     wifiConnected, gpsLock);
        state.needsRedraw = false;
    }
}

bool UIFramework::isTouchInZone(int zoneX, int zoneY, int zoneW, int zoneH) {
    TouchPoint point = touchMgr.getPoint();
    return (point.x >= zoneX && point.x <= zoneX + zoneW &&
            point.y >= zoneY && point.y <= zoneY + zoneH);
}

ScreenID UIFramework::getCurrentScreen() {
    return state.currentScreen;
}

void UIFramework::setNeedsRedraw() {
    state.needsRedraw = true;
}

bool UIFramework::needsRedraw() {
    return state.needsRedraw;
}

void UIFramework::resetIdleTimer() {
    state.lastInteraction = millis();
}

bool UIFramework::isIdle(unsigned long timeoutMs) {
    return (millis() - state.lastInteraction) > timeoutMs;
}

void UIFramework::setMenuSelection(int index) {
    state.selectedIndex = index;
    state.needsRedraw = true;
}

int UIFramework::getMenuSelection() {
    return state.selectedIndex;
}

void UIFramework::performWiFiScan() {
    settingsState.isScanning = true;
    state.needsRedraw = true;

    int n = WiFi.scanNetworks();
    settingsState.networkCount = min(n, 10);

    for (int i = 0; i < settingsState.networkCount; i++) {
        strncpy(settingsState.networks[i].ssid, WiFi.SSID(i).c_str(), 31);
        settingsState.networks[i].ssid[31] = '\0';
        settingsState.networks[i].rssi = WiFi.RSSI(i);
        settingsState.networks[i].encrypted = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }

    WiFi.scanDelete();
    settingsState.isScanning = false;
    state.needsRedraw = true;
}

void UIFramework::handleNetworkSelection(int index) {
    settingsState.selectedNetworkIndex = index;

    // If SSID matches configured network, attempt connection
    if (strcmp(settingsState.networks[index].ssid, WIFI_SSID) == 0) {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        Serial.printf("Connecting to %s...\n", WIFI_SSID);
    } else {
        Serial.printf("Network %s requires password entry (not yet implemented)\n",
                     settingsState.networks[index].ssid);
    }
    state.needsRedraw = true;
}

void UIFramework::handleHealthScreen() {
    if (state.needsRedraw) {
        // Get health data from sensor manager
        int heartRate = sensorMgr.getHeartRate();
        int spo2 = sensorMgr.getSpO2();
        bool fingerDetected = sensorMgr.getFingerDetected();
        uint8_t signalQuality = sensorMgr.getSignalQuality();

        // Get status data for status bar
        int batteryPct = powerMgr.getBatteryPercentage();
        bool bleConnected = bluetooth.isConnected();
        bool wifiConnected = (WiFi.status() == WL_CONNECTED);
        bool gpsLock = sensorMgr.hasGPSFix();

        // Pass all data including transition flag and status
        displayMgr.showHealthScreen(heartRate, spo2, fingerDetected, signalQuality,
                                   state.isScreenTransition,
                                   batteryPct, bleConnected, wifiConnected, gpsLock);

        state.needsRedraw = false;
        state.isScreenTransition = false;  // Clear transition flag after rendering
    }

    // Auto-refresh every 500ms for live updates
    static unsigned long lastHealthUpdate = 0;
    if (millis() - lastHealthUpdate > 500) {
        state.needsRedraw = true;
        state.isScreenTransition = false;  // Auto-refresh is not a transition
        lastHealthUpdate = millis();
    }
}
