#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>
#include <RTClib.h>
#include <WiFi.h>
#include <Preferences.h>
#include "time.h"

enum TimeSyncSource {
    SYNC_SOURCE_NONE,
    SYNC_SOURCE_BLE,
    SYNC_SOURCE_NTP,
    SYNC_SOURCE_GPS,
    SYNC_SOURCE_MANUAL
};

enum TimeSyncStatus {
    SYNC_NEVER,           // Never synced since boot
    SYNC_SUCCESS,         // Last sync successful
    SYNC_FAILED,          // Last sync failed
    SYNC_IN_PROGRESS,     // Currently syncing
    SYNC_STALE            // Synced >7 days ago
};

class TimeSync {
public:
    TimeSync();

    bool begin();
    void update();

    // BLE time sync (primary)
    bool syncFromBLE(uint32_t unixTimestamp, const char* timezone, int tzOffsetSec);

    // WiFi NTP sync (fallback)
    bool syncFromNTP();

    // GPS time sync (alternative fallback)
    bool syncFromGPS(DateTime gpsTime);

    // Status
    TimeSyncStatus getStatus() const { return status; }
    TimeSyncSource getLastSource() const { return lastSource; }
    DateTime getLastSyncTime() const { return lastSyncTime; }
    bool needsSync() const;

private:
    // WiFi management
    bool connectWiFi(uint32_t timeoutMs = 15000);
    void disconnectWiFi();

    // NTP client
    bool getNTPTime(DateTime& outTime);

    // Timezone management
    void loadTimezone();
    void saveTimezone(const char* tz, int gmtOffset, int dstOffset);
    void applyTimezone();

    // Sync scheduling
    bool shouldAutoSync();

    // State
    TimeSyncStatus status;
    TimeSyncSource lastSource;
    DateTime lastSyncTime;
    unsigned long lastSyncMillis;

    // Timezone
    Preferences prefs;
    String timezoneString;
    int gmtOffsetSec;
    int dstOffsetSec;

    // Constants
    static const unsigned long SYNC_INTERVAL_MS = 86400000;  // 24 hours
    static const unsigned long SYNC_RETRY_MS = 21600000;     // 6 hours
    static const int SYNC_HOUR = 3;  // 3 AM daily sync
};

extern TimeSync timeSyncMgr;

#endif
