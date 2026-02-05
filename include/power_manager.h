#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include "config.h"

// Power states
enum PowerState {
    POWER_NORMAL,           // Battery >20%, display active
    POWER_LOW_BATTERY,      // Battery 5-20%
    POWER_CRITICAL,         // Battery <5%
    POWER_DISPLAY_SLEEP,    // Display sleeping, system active
    POWER_DEEP_SLEEP        // Deep sleep mode
};

class PowerManager {
public:
    PowerManager();
    bool begin();
    void update();

    // Battery monitoring
    int getBatteryPercentage();
    float getBatteryVoltage();
    bool isCharging();
    bool isLowBattery();

    // Power state management
    void enterLightSleep();
    void enterDeepSleep(uint64_t sleepTimeSeconds = 0);
    void wake();
    PowerState getCurrentState();

    // Display sleep management
    bool isDisplaySleeping();

    // Activity tracking
    void resetIdleTimer();
    unsigned long getIdleTime();
    unsigned long getUptimeSeconds();

    // Power optimization
    void setScreenBrightness(uint8_t level);
    void enableWiFi(bool enable);
    void enableBluetooth(bool enable);

    // Wake sources
    void configureWakeButton(uint8_t pin);
    void configureWakeBLE();

private:
    PowerState currentState;
    unsigned long lastActivityTime;
    int batteryPercentage;
    float batteryVoltage;
    float smoothedVoltage;  // For exponential moving average
    bool charging;
    bool displaySleeping;
    bool lowBatteryWarningShown;

    void readBattery();
    void updatePowerState();
    void checkBatteryWarnings();
    float mapVoltageToPercentage(float voltage);
};

extern PowerManager powerMgr;

#endif // POWER_MANAGER_H
