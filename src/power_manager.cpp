/**
 * Power Manager - Battery and Sleep Management
 * Optimizes power consumption for all-day battery life
 */

#include "power_manager.h"
#include "display_manager.h"
#include <WiFi.h>

PowerManager powerMgr;

PowerManager::PowerManager()
    : currentState(POWER_NORMAL)
    , lastActivityTime(0)
    , batteryPercentage(100)
    , batteryVoltage(4.2)
    , smoothedVoltage(0)
    , charging(false)
    , displaySleeping(false)
    , lowBatteryWarningShown(false) {
}

bool PowerManager::begin() {
    Serial.println("Initializing Power Manager...");

    // Configure ADC for battery monitoring
    pinMode(BATTERY_ADC, INPUT);
    analogSetAttenuation(ADC_11db); // For 0-3.3V range

    // Configure wake sources
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_1, LOW); // Wake on button press

    // Initial battery read
    readBattery();

    // Configure power management for ESP32-S3
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);

    lastActivityTime = millis();

    Serial.println("Power Manager initialized");
    return true;
}

void PowerManager::update() {
    static unsigned long lastBatteryRead = 0;
    unsigned long now = millis();

    // Read battery every 10 seconds
    if (now - lastBatteryRead > 10000) {
        readBattery();
        checkBatteryWarnings();
        lastBatteryRead = now;
    }

    // Update power state based on idle time and battery level
    updatePowerState();

    // Auto-sleep display if idle too long
    if (!displaySleeping && (now - lastActivityTime > SLEEP_TIMEOUT_MS)) {
        displayMgr.sleep();
        displaySleeping = true;
        Serial.println("Display sleep (idle timeout)");
    }
}

int PowerManager::getBatteryPercentage() {
    return batteryPercentage;
}

float PowerManager::getBatteryVoltage() {
    return batteryVoltage;
}

bool PowerManager::isCharging() {
    return charging;
}

bool PowerManager::isLowBattery() {
    return batteryPercentage < LOW_BATTERY_THRESHOLD;
}

void PowerManager::enterLightSleep() {
    Serial.println("Entering light sleep mode");

    // Configure wake sources
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_1, LOW);
    esp_sleep_enable_timer_wakeup(5000000); // Wake every 5 seconds to check BLE

    // Enter light sleep (CPU sleep, RAM preserved)
    esp_light_sleep_start();

    // Woken up - restore state
    lastActivityTime = millis();
    updatePowerState();
    Serial.println("Woke from light sleep");
}

void PowerManager::enterDeepSleep(uint64_t sleepTimeSeconds) {
    currentState = POWER_DEEP_SLEEP;
    Serial.printf("Entering deep sleep mode");
    if (sleepTimeSeconds > 0) {
        Serial.printf(" for %llu seconds\n", sleepTimeSeconds);
    } else {
        Serial.println();
    }

    // Configure wake sources
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_1, LOW);  // Wake on Button 1 press (LOW)

    // Configure timer wake if sleep time specified
    if (sleepTimeSeconds > 0) {
        esp_sleep_enable_timer_wakeup(sleepTimeSeconds * 1000000ULL);  // Convert to microseconds
    }

    // Deep sleep (will reset on wake)
    esp_deep_sleep_start();
}

void PowerManager::wake() {
    if (displaySleeping) {
        displayMgr.wake();
        displaySleeping = false;
        Serial.println("Display wake (manual)");
    }
    lastActivityTime = millis();
    updatePowerState();
}

PowerState PowerManager::getCurrentState() {
    return currentState;
}

bool PowerManager::isDisplaySleeping() {
    return displaySleeping;
}

unsigned long PowerManager::getUptimeSeconds() {
    return millis() / 1000;
}

void PowerManager::resetIdleTimer() {
    lastActivityTime = millis();

    // Wake display if sleeping
    if (displaySleeping) {
        displayMgr.wake();
        displaySleeping = false;
        Serial.println("Display wake (user interaction)");
    }
}

unsigned long PowerManager::getIdleTime() {
    return millis() - lastActivityTime;
}

void PowerManager::setScreenBrightness(uint8_t level) {
    // Control TFT backlight brightness
    displayMgr.setBrightness(level);
    Serial.printf("Screen brightness set to: %d\n", level);
}

void PowerManager::enableWiFi(bool enable) {
    if (enable) {
        WiFi.mode(WIFI_STA);
        Serial.println("WiFi enabled");
    } else {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        Serial.println("WiFi disabled");
    }
}

void PowerManager::enableBluetooth(bool enable) {
    // BLE enable/disable would be handled by bluetooth service
    Serial.print("Bluetooth ");
    Serial.println(enable ? "enabled" : "disabled");
}

void PowerManager::configureWakeButton(uint8_t pin) {
    esp_sleep_enable_ext0_wakeup((gpio_num_t)pin, LOW);
}

void PowerManager::configureWakeBLE() {
    // Configure BLE as wake source
    // This requires more complex setup with BLE library
}

void PowerManager::readBattery() {
    // Read ADC value
    int raw = analogRead(BATTERY_ADC);

    // Convert to voltage (assuming voltage divider)
    // ADC resolution: 12-bit (0-4095)
    // Voltage divider: 2:1 ratio (10kΩ to battery, 10kΩ to GND)
    float voltage = (raw / 4095.0) * 3.3 * 2.0;

    // Debug: Print raw ADC reading (first few times only)
    static int debugCount = 0;
    if (debugCount < 5) {
        Serial.printf("[Battery Debug] Raw ADC: %d, Voltage: %.2fV\n", raw, voltage);
        debugCount++;
    }

    // Apply exponential moving average smoothing (90% old, 10% new)
    if (smoothedVoltage == 0) {
        smoothedVoltage = voltage;  // Initialize on first read
    } else {
        smoothedVoltage = (smoothedVoltage * 0.9) + (voltage * 0.1);
    }

    batteryVoltage = smoothedVoltage;

    // Convert voltage to percentage using accurate LiPo discharge curve
    float percentage = mapVoltageToPercentage(smoothedVoltage);
    batteryPercentage = constrain((int)percentage, 0, 100);

    // Detect charging (voltage rises above battery max when charging)
    charging = (batteryVoltage > BATTERY_MAX_VOLTAGE + 0.1);
}

void PowerManager::updatePowerState() {
    // Update power state based on battery level and display state
    if (batteryPercentage <= 5) {
        currentState = POWER_CRITICAL;
    } else if (batteryPercentage <= LOW_BATTERY_THRESHOLD) {
        currentState = POWER_LOW_BATTERY;
    } else if (displaySleeping) {
        currentState = POWER_DISPLAY_SLEEP;
    } else {
        currentState = POWER_NORMAL;
    }
}

void PowerManager::checkBatteryWarnings() {
    // Low battery warning at 20%
    if (batteryPercentage <= LOW_BATTERY_THRESHOLD && batteryPercentage > 5 && !lowBatteryWarningShown) {
        Serial.printf("⚠️  Low battery: %d%% (%.2fV)\n", batteryPercentage, batteryVoltage);
        // TODO: Show warning on display (could integrate with notification system)
        lowBatteryWarningShown = true;
    }

    // Reset warning flag when battery is above threshold
    if (batteryPercentage > LOW_BATTERY_THRESHOLD) {
        lowBatteryWarningShown = false;
    }

    // Critical battery shutdown at 5% - TEMPORARILY DISABLED FOR DEBUGGING
    if (batteryPercentage <= 5) {
        Serial.printf("⚠️ Low battery reading: %d%% (%.2fV) - NOT shutting down (debug mode)\n",
                     batteryPercentage, batteryVoltage);
        // Shutdown disabled - battery ADC may need calibration
        // TODO: Re-enable after verifying battery monitoring works correctly
    }
}

float PowerManager::mapVoltageToPercentage(float voltage) {
    // LiPo discharge curve (piecewise linear approximation)
    // Based on typical single-cell LiPo battery characteristics
    // 4.20V = 100%, 4.00V = 85%, 3.80V = 60%, 3.70V = 40%, 3.50V = 15%, 3.30V = 0%

    if (voltage >= 4.20) return 100.0;
    if (voltage >= 4.00) return 85.0 + (voltage - 4.00) * 75.0;   // 4.00-4.20V = 85-100% (15% range)
    if (voltage >= 3.80) return 60.0 + (voltage - 3.80) * 125.0;  // 3.80-4.00V = 60-85% (25% range)
    if (voltage >= 3.70) return 40.0 + (voltage - 3.70) * 200.0;  // 3.70-3.80V = 40-60% (20% range)
    if (voltage >= 3.50) return 15.0 + (voltage - 3.50) * 125.0;  // 3.50-3.70V = 15-40% (25% range)
    if (voltage >= 3.30) return (voltage - 3.30) * 75.0;          // 3.30-3.50V = 0-15% (15% range)

    return 0.0;  // Below 3.30V = dead battery
}
