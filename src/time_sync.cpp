/**
 * Time Sync Manager - Automatic Time Synchronization
 * Syncs RTC from BLE (primary), WiFi NTP (fallback), or GPS
 */

#include "time_sync.h"
#include "sensor_manager.h"
#include "config.h"

TimeSync timeSyncMgr;

TimeSync::TimeSync()
    : status(SYNC_NEVER)
    , lastSource(SYNC_SOURCE_NONE)
    , lastSyncTime(DateTime(2000, 1, 1))
    , lastSyncMillis(0)
    , gmtOffsetSec(0)
    , dstOffsetSec(0) {
}

bool TimeSync::begin() {
    Serial.println("Initializing Time Sync Manager...");

    // Initialize preferences for timezone storage
    prefs.begin("timesync", false);

    // Load saved timezone configuration
    loadTimezone();

    // Apply timezone to system
    applyTimezone();

    // Check if RTC needs initial sync
    extern SensorManager sensorMgr;
    if (sensorMgr.getStatus().rtcAvailable) {
        // Check if we have a valid sync time stored
        uint32_t lastSync = prefs.getUInt("lastSync", 0);
        if (lastSync == 0) {
            status = SYNC_NEVER;
            Serial.println("  No previous sync - time sync required");
        } else {
            lastSyncMillis = millis();
            status = SYNC_SUCCESS;

            // Check if sync is stale (>7 days)
            unsigned long daysSinceSync = (millis() / 1000 - lastSync) / 86400;
            if (daysSinceSync > 7) {
                status = SYNC_STALE;
                Serial.println("  Last sync >7 days ago - time sync recommended");
            } else {
                Serial.printf("  Last sync: %lu days ago\n", daysSinceSync);
            }
        }
    } else {
        Serial.println("  RTC not available - time sync disabled");
        return false;
    }

    Serial.println("Time Sync Manager initialized");
    return true;
}

void TimeSync::update() {
    // Check if auto-sync is needed
    if (shouldAutoSync()) {
        Serial.println("Auto-sync triggered");

        // Try NTP sync (BLE sync is event-driven from phone)
        // Don't attempt WiFi NTP every loop - only when scheduled
        static unsigned long lastNTPAttempt = 0;
        if (millis() - lastNTPAttempt > 60000) {  // Rate limit to once per minute
            if (syncFromNTP()) {
                Serial.println("✓ Auto-sync completed via NTP");
            }
            lastNTPAttempt = millis();
        }
    }
}

bool TimeSync::syncFromBLE(uint32_t unixTimestamp, const char* timezone, int tzOffsetSec) {
    Serial.printf("Syncing time from BLE: %lu (%s, %d)\n", unixTimestamp, timezone, tzOffsetSec);

    // Validate timestamp (year 2025-2050)
    if (unixTimestamp < 1735689600 || unixTimestamp > 2524608000) {  // 2025-01-01 to 2050-01-01
        Serial.println("  ERROR: Invalid timestamp (year out of range)");
        status = SYNC_FAILED;
        return false;
    }

    // Convert Unix timestamp to DateTime
    DateTime dt(unixTimestamp);

    // Save timezone to preferences
    saveTimezone(timezone, tzOffsetSec, 0);  // DST offset handled by phone

    // Set RTC time
    extern SensorManager sensorMgr;
    sensorMgr.setRTCTime(dt);

    // Update state
    status = SYNC_SUCCESS;
    lastSource = SYNC_SOURCE_BLE;
    lastSyncTime = dt;
    lastSyncMillis = millis();

    // Save last sync timestamp to preferences
    prefs.putUInt("lastSync", unixTimestamp);

    Serial.printf("✓ RTC synced to: %04d-%02d-%02d %02d:%02d:%02d\n",
                 dt.year(), dt.month(), dt.day(),
                 dt.hour(), dt.minute(), dt.second());

    return true;
}

bool TimeSync::syncFromNTP() {
    Serial.println("Attempting WiFi NTP sync...");
    status = SYNC_IN_PROGRESS;

    // Connect to WiFi
    if (!connectWiFi(WIFI_CONNECT_TIMEOUT_MS)) {
        Serial.println("  WiFi connection failed");
        status = SYNC_FAILED;
        return false;
    }

    // Get NTP time
    DateTime ntpTime(2000, 1, 1);
    if (!getNTPTime(ntpTime)) {
        Serial.println("  NTP sync failed");
        disconnectWiFi();
        status = SYNC_FAILED;
        return false;
    }

    // Set RTC time
    extern SensorManager sensorMgr;
    sensorMgr.setRTCTime(ntpTime);

    // Update state
    status = SYNC_SUCCESS;
    lastSource = SYNC_SOURCE_NTP;
    lastSyncTime = ntpTime;
    lastSyncMillis = millis();

    // Save last sync timestamp to preferences
    prefs.putUInt("lastSync", ntpTime.unixtime());

    Serial.printf("✓ RTC synced to: %04d-%02d-%02d %02d:%02d:%02d (NTP)\n",
                 ntpTime.year(), ntpTime.month(), ntpTime.day(),
                 ntpTime.hour(), ntpTime.minute(), ntpTime.second());

    // Disconnect WiFi to save power
    disconnectWiFi();

    return true;
}

bool TimeSync::syncFromGPS(DateTime gpsTime) {
    Serial.println("Syncing time from GPS...");

    // Validate GPS time (year > 2020)
    if (gpsTime.year() < 2020) {
        Serial.println("  ERROR: Invalid GPS time");
        status = SYNC_FAILED;
        return false;
    }

    // Set RTC time
    extern SensorManager sensorMgr;
    sensorMgr.setRTCTime(gpsTime);

    // Update state
    status = SYNC_SUCCESS;
    lastSource = SYNC_SOURCE_GPS;
    lastSyncTime = gpsTime;
    lastSyncMillis = millis();

    // Save last sync timestamp to preferences
    prefs.putUInt("lastSync", gpsTime.unixtime());

    Serial.printf("✓ RTC synced to: %04d-%02d-%02d %02d:%02d:%02d (GPS)\n",
                 gpsTime.year(), gpsTime.month(), gpsTime.day(),
                 gpsTime.hour(), gpsTime.minute(), gpsTime.second());

    return true;
}

bool TimeSync::needsSync() const {
    return (status == SYNC_NEVER || status == SYNC_STALE || status == SYNC_FAILED);
}

bool TimeSync::connectWiFi(uint32_t timeoutMs) {
    Serial.print("Connecting to WiFi");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > timeoutMs) {
            Serial.println(" timeout!");
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            return false;
        }
        Serial.print(".");
        delay(500);
    }

    Serial.printf(" connected (%s)\n", WiFi.localIP().toString().c_str());
    return true;
}

void TimeSync::disconnectWiFi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi disconnected");
}

bool TimeSync::getNTPTime(DateTime& outTime) {
    Serial.println("Fetching NTP time...");

    // Configure NTP with timezone
    configTime(gmtOffsetSec, dstOffsetSec, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    // Wait for time to be set
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, NTP_TIMEOUT_MS / 1000)) {
        Serial.println("  Failed to get time from NTP");
        return false;
    }

    // Convert to DateTime
    outTime = DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                       timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    Serial.printf("  NTP time: %04d-%02d-%02d %02d:%02d:%02d\n",
                 outTime.year(), outTime.month(), outTime.day(),
                 outTime.hour(), outTime.minute(), outTime.second());

    return true;
}

void TimeSync::loadTimezone() {
    // Load timezone settings from NVS
    timezoneString = prefs.getString("tz", DEFAULT_TIMEZONE);
    gmtOffsetSec = prefs.getInt("gmtOffset", 0);
    dstOffsetSec = prefs.getInt("dstOffset", 0);

    Serial.printf("  Loaded timezone: %s (GMT%+d)\n",
                 timezoneString.c_str(), gmtOffsetSec / 3600);
}

void TimeSync::saveTimezone(const char* tz, int gmtOffset, int dstOffset) {
    timezoneString = String(tz);
    gmtOffsetSec = gmtOffset;
    dstOffsetSec = dstOffset;

    prefs.putString("tz", timezoneString);
    prefs.putInt("gmtOffset", gmtOffsetSec);
    prefs.putInt("dstOffset", dstOffsetSec);

    Serial.printf("  Timezone saved: %s (GMT%+d)\n", tz, gmtOffset / 3600);

    // Apply immediately
    applyTimezone();
}

void TimeSync::applyTimezone() {
    // Set timezone for system time functions
    configTime(gmtOffsetSec, dstOffsetSec, "pool.ntp.org");
    Serial.printf("  Timezone applied: GMT%+d (DST%+d)\n",
                 gmtOffsetSec / 3600, dstOffsetSec / 3600);
}

bool TimeSync::shouldAutoSync() {
    // Don't auto-sync if never synced successfully
    if (status == SYNC_NEVER) {
        return true;  // Always try to get initial sync
    }

    // Check if 24 hours elapsed since last sync
    if (millis() - lastSyncMillis > SYNC_INTERVAL_MS) {
        return true;
    }

    // Check if current time is within daily sync window (3:00-3:05 AM)
    extern SensorManager sensorMgr;
    DateTime now = sensorMgr.getCurrentTime();

    if (now.hour() == SYNC_HOUR && now.minute() < 5) {
        // Check if we already synced today
        static int lastSyncDay = -1;
        if (now.day() != lastSyncDay) {
            lastSyncDay = now.day();
            return true;
        }
    }

    // If last sync failed, retry after 6 hours
    if (status == SYNC_FAILED && millis() - lastSyncMillis > SYNC_RETRY_MS) {
        return true;
    }

    return false;
}
