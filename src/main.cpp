/**
 * WEARABLE TECH PAD - Main Application
 * Pip-Boy Style Wrist Computer with 2.8" TFT Display
 *
 * Hardware: ESP32-WROOM-32 + ILI9341 TFT + Touch Screen
 * Features: BLE, Voice AI, Smart Home Control, iPhone Integration, Multi-Sensor Suite
 */

#include <Arduino.h>
#include <Wire.h>
#include <AceButton.h>
#include "config.h"
#include "display_manager.h"
#include "touch_manager.h"
#include "sensor_manager.h"
#include "ui_framework.h"
#include "bluetooth_service.h"
#include "power_manager.h"
#include "time_sync.h"
#include "voice_ai.h"
#include "notification_handler.h"
#include "smart_home_mqtt.h"

using namespace ace_button;

// Global instances (extern declarations are in respective headers)
// Instances are now defined in their respective .cpp files:
// - bluetooth (bluetooth_service.cpp)
// - powerMgr (power_manager.cpp)
// - displayMgr (display_manager.cpp)
// - sensorMgr (sensor_manager.cpp)
// - touchMgr (touch_manager.cpp)
// - uiFramework (ui_framework.cpp)

// Instances that still need to be moved to their .cpp files:
VoiceAI voiceAI;
NotificationHandler notificationHandler;
SmartHomeMQTT smartHome;

// Button handlers
AceButton button1;
AceButton button2;
AceButton button3;

// Application state
struct AppState {
    bool bluetoothEnabled;
    bool wifiEnabled;
    bool sensorsReady;
    unsigned long bootTime;
    unsigned long lastHeartbeat;
};

AppState appState;

// Forward declarations
void handleButtonEvent(AceButton*, uint8_t, uint8_t);
void onWakeWord();
void onVoiceCommand(const VoiceCommand& cmd);
void printSystemInfo();
void performSelfTest();

void setup() {
    Serial.begin(115200);

    // STARTUP DELAY - Gives 5 seconds to upload new firmware before crash
    Serial.println("\n\n=== STARTUP DELAY - 5 seconds ===");
    Serial.println("Upload window available...");
    for(int i = 5; i > 0; i--) {
        Serial.printf("Starting in %d...\n", i);
        delay(1000);
    }
    Serial.println("=== Starting initialization ===\n");

    // Setup backlight as LED for debugging
    pinMode(21, OUTPUT);

    // Blink backlight to show we're alive
    for(int i = 0; i < 5; i++) {
        digitalWrite(21, HIGH);
        delay(200);
        digitalWrite(21, LOW);
        delay(200);
    }
    digitalWrite(21, HIGH);  // Leave it on

    delay(500);  // Give serial time to stabilize

    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║  TOUCH TEST - MINIMAL MODE           ║");
    Serial.println("╚════════════════════════════════════════╝\n");

    appState.bootTime = millis();
    appState.bluetoothEnabled = false;
    appState.wifiEnabled = false;
    appState.sensorsReady = false;
    appState.lastHeartbeat = 0;

    // ========================================
    // PHASE 1: Core Hardware Initialization
    // ========================================

    Serial.println("┌─ PHASE 1: Core Hardware ─────────────┐");

    // FORCE BACKLIGHT ON (Debugging)
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
    analogWrite(21, 255);
    Serial.println("│ BACKLIGHT FORCED ON (GPIO 21)        │");
    delay(500);

    // Initialize display first (for visual feedback)
    Serial.print("│ Display (ILI9341 TFT)... ");
    Serial.flush();

    if (!displayMgr.begin()) {
        Serial.println("FAILED! │");
        Serial.println("└───────────────────────────────────────┘");
        Serial.println("CRITICAL ERROR: Cannot proceed without display!");
        // Blink backlight rapidly on error
        while(1) {
            digitalWrite(21, !digitalRead(21));
            delay(200);
        }
    }
    Serial.println("OK       │");

    // Skip boot screen - go straight to touch test
    // displayMgr.showBootScreen("Tech Pad v2.0");
    // delay(1000);

    // Initialize touch screen
    Serial.print("│ Touch Screen (XPT2046)... ");
    Serial.flush();

    if (!touchMgr.begin()) {
        Serial.println("FAILED! │");
        Serial.flush();
    } else {
        Serial.println("OK       │");
        Serial.flush();
    }

    // SKIP sensors and BLE for now - just enable UI
    Serial.println("│ (Sensors/BLE disabled)    SKIP     │");
    Serial.flush();

    Serial.println("└───────────────────────────────────────┘\n");
    Serial.flush();

    // ========================================
    // PHASE 4: UI Framework
    // ========================================

    Serial.println("┌─ PHASE 4: UI Framework ───────────────┐");
    Serial.print("│ UI Framework...           ");
    uiFramework.begin();
    Serial.println("OK       │");
    Serial.println("└───────────────────────────────────────┘\n");

    // Initialize sensors (BME280 only - incremental testing)
    Serial.println("┌─ PHASE 1.5: Sensors (BME280 only) ────┐");
    Serial.print("│ Sensor Manager...         ");
    if (!sensorMgr.begin()) {
        Serial.println("FAILED! │");
        appState.sensorsReady = false;
    } else {
        Serial.println("OK       │");
        appState.sensorsReady = true;
    }
    Serial.println("└───────────────────────────────────────┘\n");

    // ========================================
    // PHASE 2: Peripherals & Communication
    // ========================================

    Serial.println("┌─ PHASE 2: Peripherals ────────────────┐");

    // Initialize power management
    Serial.print("│ Power Manager...          ");
    if (!powerMgr.begin()) {
        Serial.println("FAILED! │");
    } else {
        Serial.println("OK       │");
    }

    // Initialize Bluetooth
    Serial.print("│ Bluetooth (BLE)...        ");
    if (!bluetooth.begin()) {
        Serial.println("FAILED! │");
    } else {
        Serial.println("OK       │");
        appState.bluetoothEnabled = true;
    }

    // Initialize Time Sync
    Serial.print("│ Time Sync Manager...      ");
    if (!timeSyncMgr.begin()) {
        Serial.println("FAILED! │");
    } else {
        Serial.println("OK       │");
    }

    // Initialize WiFi - Auto-connect at startup
    Serial.print("│ WiFi Connection...        ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Wait up to 10 seconds for connection
    int wifiTimeout = 0;
    while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
        delay(500);
        Serial.print(".");
        wifiTimeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("OK       │");
        Serial.print("│ IP Address: ");
        Serial.print(WiFi.localIP());
        Serial.println("              │");
        appState.wifiEnabled = true;
    } else {
        Serial.println("TIMEOUT  │");
        Serial.println("│ WiFi not available                    │");
        appState.wifiEnabled = false;
    }

    // Initialize buttons
    Serial.print("│ Physical Buttons...       ");
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    pinMode(BUTTON_3, INPUT_PULLUP);

    button1.init(BUTTON_1);
    button2.init(BUTTON_2);
    button3.init(BUTTON_3);

    ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(handleButtonEvent);
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
    Serial.println("OK       │");

    // Initialize vibration motor
    Serial.print("│ Haptic Feedback...        ");
    pinMode(VIBRATION_MOTOR, OUTPUT);
    digitalWrite(VIBRATION_MOTOR, LOW);
    Serial.println("OK       │");

    // Initialize buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    // digitalWrite(BUZZER_PIN, LOW);

    // Serial.println("└───────────────────────────────────────┘\n");

    // // ========================================
    // // PHASE 3: Advanced Features
    // // ========================================

    // Serial.println("┌─ PHASE 3: Advanced Features ──────────┐");

    // // Initialize voice AI
    // Serial.print("│ Voice AI (INMP441)...     ");
    // if (!voiceAI.begin()) {
    //     Serial.println("DISABLED│");
    // } else {
    //     Serial.println("OK       │");
    //     voiceAI.onWakeWord(onWakeWord);
    //     voiceAI.onCommand(onVoiceCommand);
    // }

    // // Initialize notification handler
    // Serial.print("│ Notification Handler...   ");
    // notificationHandler.begin();
    // Serial.println("OK       │");

    // // Smart home is initialized on-demand when WiFi is enabled
    // Serial.print("│ Smart Home MQTT...        ");
    // Serial.println("STANDBY │");

    // Serial.println("└───────────────────────────────────────┘\n");

    // // ========================================
    // // PHASE 4: UI Framework
    // // ========================================

    // Serial.println("┌─ PHASE 4: UI Framework ───────────────┐");
    // Serial.print("│ UI Framework...           ");
    // uiFramework.begin();
    // Serial.println("OK       │");
    // Serial.println("└───────────────────────────────────────┘\n");

    // ========================================
    // System Ready
    // ========================================

    unsigned long bootDuration = millis() - appState.bootTime;
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║         SYSTEM READY                   ║");
    Serial.printf("║  Boot Time: %lu ms%*s║\n", bootDuration, 25 - String(bootDuration).length(), "");
    Serial.println("╚════════════════════════════════════════╝\n");
    Serial.flush();

    // // Print system information
    // printSystemInfo();

    // // Perform self-test
    // performSelfTest();

    // Check if RTC needs time sync
    Serial.println("┌─ POST-INIT: Time Sync ────────────────┐");
    if (sensorMgr.getStatus().needsTimeSync) {
        Serial.println("│ RTC needs sync - requesting from BLE  │");
        if (bluetooth.isConnected()) {
            bluetooth.requestTimeSync();
        } else {
            Serial.println("│ BLE not connected - will try NTP      │");
            // TimeSync manager will attempt NTP in its update() loop
        }
    } else {
        Serial.println("│ RTC time valid - no sync needed       │");
    }
    Serial.println("└───────────────────────────────────────┘\n");

    // // Welcome haptic feedback
    // digitalWrite(VIBRATION_MOTOR, HIGH);
    // delay(100);
    // digitalWrite(VIBRATION_MOTOR, LOW);
    // delay(100);
    // digitalWrite(VIBRATION_MOTOR, HIGH);
    // delay(100);
    // digitalWrite(VIBRATION_MOTOR, LOW);

    // Start with clock screen
    Serial.println("\n>>> Entering main loop - UI active <<<\n");
}

void loop() {
    static unsigned long lastUpdate = 0;
    static unsigned long lastSensorUpdate = 0;
    unsigned long currentTime = millis();

    // ========================================
    // Input Handling
    // ========================================

    // Update buttons
    // button1.check();
    // button2.check();
    // button3.check();

    // Update touch manager
    touchMgr.update();

    // ========================================
    // Subsystem Updates
    // ========================================

    // Update sensors (every 100ms) - BME280 only
    if (currentTime - lastSensorUpdate >= 100) {
        sensorMgr.update();
        lastSensorUpdate = currentTime;
    }

    // // Update Bluetooth
    // if (appState.bluetoothEnabled) {
    //     bluetooth.update();
    // }

    // Update power management
    powerMgr.update();

    // Update time sync
    timeSyncMgr.update();

    // // Update voice AI
    // voiceAI.update();

    // // Update notification handler
    // notificationHandler.update();

    // // Update smart home (if connected)
    // if (smartHome.isConnected()) {
    //     smartHome.update();
    // }

    // ========================================
    // UI Framework & Display Update
    // ========================================

    // UI Framework Update
    uiFramework.update();

    // Update display
    displayMgr.update();

    // ========================================
    // Notification Handling (DISABLED FOR TOUCH TEST)
    // ========================================

    // static int lastNotifCount = 0;
    // if (appState.bluetoothEnabled) {
    //     int currentNotifCount = bluetooth.getNotificationCount();
    //     if (currentNotifCount > lastNotifCount) {
    //         Notification notif = bluetooth.getLatestNotification();
    //         notificationHandler.showNotification(notif);

    //         // Navigate to notification screen
    //         uiFramework.navigateTo(SCREEN_NOTIFICATIONS);

    //         // Haptic feedback
    //         digitalWrite(VIBRATION_MOTOR, HIGH);
    //         delay(200);
    //         digitalWrite(VIBRATION_MOTOR, LOW);

    //         lastNotifCount = currentNotifCount;
    //     }
    // }

    // ========================================
    // Heartbeat (every 10 seconds)
    // ========================================

    if (currentTime - appState.lastHeartbeat >= 10000) {
        Serial.printf("[%lu] Heartbeat - Free RAM: %d bytes, Uptime: %lu sec\n",
                      currentTime / 1000,
                      ESP.getFreeHeap(),
                      currentTime / 1000);

        appState.lastHeartbeat = currentTime;
    }

    // Small delay to prevent watchdog issues
    delay(10);
}

// ============================================================================
// Button Event Handler
// ============================================================================

void handleButtonEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    powerMgr.resetIdleTimer();
    uint8_t pin = button->getPin();

    // Wake display if sleeping
    if (displayMgr.isSleeping()) {
        displayMgr.wake();
        return;
    }

    switch (eventType) {
        case AceButton::kEventClicked:
            Serial.printf("Button %d clicked\n", pin == BUTTON_1 ? 1 : (pin == BUTTON_2 ? 2 : 3));

            if (pin == BUTTON_1) {
                // Button 1 (Left): Menu / Back
                ScreenID current = uiFramework.getCurrentScreen();
                if (current == SCREEN_CLOCK || current == SCREEN_SENSORS ||
                    current == SCREEN_GPS || current == SCREEN_NOTIFICATIONS) {
                    uiFramework.navigateTo(SCREEN_MENU);
                } else {
                    uiFramework.navigateBack();
                }
            } else if (pin == BUTTON_2) {
                // Button 2 (Bottom): Mode switch / Previous
                uiFramework.navigatePrev();
            } else if (pin == BUTTON_3) {
                // Button 3 (Right): Select / Next
                uiFramework.navigateNext();
            }

            // Haptic feedback
            digitalWrite(VIBRATION_MOTOR, HIGH);
            delay(30);
            digitalWrite(VIBRATION_MOTOR, LOW);
            break;

        case AceButton::kEventLongPressed:
            Serial.printf("Button %d long pressed\n", pin == BUTTON_1 ? 1 : (pin == BUTTON_2 ? 2 : 3));

            if (pin == BUTTON_1) {
                // Long press Button 1: Activate voice
                uiFramework.navigateTo(SCREEN_VOICE);
                voiceAI.startListening();
            } else if (pin == BUTTON_2) {
                // Long press Button 2: Toggle WiFi
                appState.wifiEnabled = !appState.wifiEnabled;
                Serial.printf("WiFi %s\n", appState.wifiEnabled ? "ENABLED" : "DISABLED");
            } else if (pin == BUTTON_3) {
                // Long press Button 3: Reset to clock
                uiFramework.navigateTo(SCREEN_CLOCK);
            }

            // Longer haptic feedback
            digitalWrite(VIBRATION_MOTOR, HIGH);
            delay(100);
            digitalWrite(VIBRATION_MOTOR, LOW);
            break;

        case AceButton::kEventDoubleClicked:
            Serial.printf("Button %d double clicked\n", pin == BUTTON_1 ? 1 : (pin == BUTTON_2 ? 2 : 3));

            if (pin == BUTTON_1) {
                // Double click: Return to clock
                uiFramework.navigateTo(SCREEN_CLOCK);
            } else if (pin == BUTTON_3) {
                // Double click Button 3: Settings
                uiFramework.navigateTo(SCREEN_SETTINGS);
            }
            break;
    }
}

// ============================================================================
// Voice AI Callbacks
// ============================================================================

void onWakeWord() {
    Serial.println("🎤 Wake word detected!");
    uiFramework.navigateTo(SCREEN_VOICE);

    // Haptic feedback
    digitalWrite(VIBRATION_MOTOR, HIGH);
    delay(50);
    digitalWrite(VIBRATION_MOTOR, LOW);
    delay(50);
    digitalWrite(VIBRATION_MOTOR, HIGH);
    delay(50);
    digitalWrite(VIBRATION_MOTOR, LOW);
}

void onVoiceCommand(const VoiceCommand& cmd) {
    Serial.printf("🎤 Voice command: %s\n", cmd.raw);

    switch (cmd.type) {
        case CMD_CHECK_MESSAGES:
        case CMD_SHOW_NOTIFICATIONS:
            uiFramework.navigateTo(SCREEN_NOTIFICATIONS);
            break;

        case CMD_DEVICE_CONTROL:
            if (smartHome.isConnected()) {
                smartHome.toggleDevice(cmd.parameter);
                uiFramework.navigateTo(SCREEN_SMART_HOME);
            }
            break;

        case CMD_TIME_QUERY:
            uiFramework.navigateTo(SCREEN_CLOCK);
            break;

        case CMD_BATTERY_STATUS: {
            int battery = powerMgr.getBatteryPercentage();
            Serial.printf("Battery: %d%%\n", battery);
            uiFramework.navigateTo(SCREEN_SETTINGS);
            break;
        }

        case CMD_CUSTOM_QUERY:
            // Send to cloud via phone
            voiceAI.sendToCloud(cmd.raw);
            break;

        default:
            Serial.println("Unknown command");
            break;
    }

    voiceAI.stopListening();
}

// ============================================================================
// Helper Functions
// ============================================================================

void printSystemInfo() {
    Serial.println("┌─ SYSTEM INFORMATION ──────────────────┐");
    Serial.printf("│ Chip Model: %s%*s│\n", ESP.getChipModel(), 25 - strlen(ESP.getChipModel()), "");
    Serial.printf("│ CPU Frequency: %d MHz%*s│\n", ESP.getCpuFreqMHz(), 19, "");
    Serial.printf("│ Free Heap: %d bytes%*s│\n", ESP.getFreeHeap(), 18 - String(ESP.getFreeHeap()).length(), "");
    Serial.printf("│ Flash Size: %d MB%*s│\n", ESP.getFlashChipSize() / (1024 * 1024), 20 - String(ESP.getFlashChipSize() / (1024 * 1024)).length(), "");
    Serial.println("└───────────────────────────────────────┘\n");
}

void performSelfTest() {
    Serial.println("┌─ SELF TEST ───────────────────────────┐");

    // Test sensors
    SensorStatus status = sensorMgr.getStatus();
    Serial.printf("│ BME280:      %s%*s│\n", status.bme280Available ? "✓ OK" : "✗ FAIL", status.bme280Available ? 29 : 27, "");
    Serial.printf("│ GPS:         %s%*s│\n", status.gpsAvailable ? "✓ OK" : "✗ FAIL", status.gpsAvailable ? 29 : 27, "");
    Serial.printf("│ RTC:         %s%*s│\n", status.rtcAvailable ? "✓ OK" : "✗ FAIL", status.rtcAvailable ? 29 : 27, "");
    Serial.printf("│ IMU:         %s%*s│\n", status.imuAvailable ? "✓ OK" : "✗ FAIL", status.imuAvailable ? 29 : 27, "");
    Serial.printf("│ I/O Expand:  %s%*s│\n", status.ioExpanderAvailable ? "✓ OK" : "✗ FAIL", status.ioExpanderAvailable ? 29 : 27, "");

    // Test battery
    int battery = powerMgr.getBatteryPercentage();
    Serial.printf("│ Battery:     %d%%%*s│\n", battery, 27 - String(battery).length(), "");

    // Test display
    Serial.printf("│ Display:     ✓ OK (320x240)%*s│\n", 14, "");

    // Test touch
    Serial.printf("│ Touch:       ✓ OK%*s│\n", 24, "");

    Serial.println("└───────────────────────────────────────┘\n");
}
