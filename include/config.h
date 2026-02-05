#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// WEARABLE TECH PAD - CONFIGURATION
// ESP32-WROOM-32 + 2.8" ILI9341 TFT Display
// ============================================================================

// Display Configuration (ILI9341 TFT - 2.8" 240x320)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define TFT_ROTATION 1  // Landscape mode (rotated 90 degrees)

// Display Pins (ILI9341 via SPI)
#define TFT_MOSI 11   // Changed from 23 (not available on N16R8)
#define TFT_MISO 19   // OK
#define TFT_SCLK 18   // OK
#define TFT_CS   15   // OK
#define TFT_DC   2    // OK
#define TFT_RST  4    // OK
#define TFT_BL   21   // OK - Backlight PWM control

// Touch Screen Pins (XPT2046) - UPDATED to avoid flash conflicts
#define TOUCH_CLK  12   // Was GPIO 10 (conflicted with flash)
#define TOUCH_MOSI 13   // Was GPIO 9 (conflicted with flash)
#define TOUCH_MISO 39   // Unchanged (input-only, OK for MISO)
#define TOUCH_CS   14   // Was GPIO 8
#define TOUCH_IRQ  -1   // Optional, not used

// Touch Calibration (calibrated values - swapped for correct mapping)
#define TOUCH_CAL_X_MIN 3525  // Swapped: was 429
#define TOUCH_CAL_X_MAX 429   // Swapped: was 3525
#define TOUCH_CAL_Y_MIN 3591  // Swapped: was 63
#define TOUCH_CAL_Y_MAX 63    // Swapped: was 3591

// I2C Bus 1 (Primary - Sensors)
#define I2C1_SDA 6    // Changed from 27 (PSRAM conflict on N16R8)
#define I2C1_SCL 7    // Changed from 22 (not available on N16R8)
// Devices: BME280 (0x76), MPU6050 (0x69), PCF8574 (0x20)

// I2C Bus 2 (Secondary - RTC)
#define I2C2_SDA 17
#define I2C2_SCL 16
// Devices: DS3231 RTC (0x68)

// I2S Microphone (INMP441) - MOVED to make room for touch
#define I2S_WS  47  // Word Select
#define I2S_SD  48  // Serial Data - MOVED from GPIO 14 (now used by touch)
#define I2S_SCK 45  // Bit Clock - MOVED from GPIO 12 (now used by touch)
#define I2S_PORT I2S_NUM_0

// UART for GPS (NEO6M)
#define GPS_RX 3   // ESP32 RX from GPS TX
#define GPS_TX 1   // ESP32 TX to GPS RX
#define GPS_BAUD 9600

// User Interface
#define BUTTON_1 40  // Changed from 34 (not available on N16R8)
#define BUTTON_2 41  // Changed from 35 (PSRAM conflict on N16R8)
#define BUTTON_3 42  // Changed from 36 (PSRAM conflict on N16R8)
#define VIBRATION_MOTOR 5   // OK
#define BUZZER_PIN 46       // MOVED from GPIO 13 (now used by touch)

// Battery Monitoring
#define BATTERY_ADC 20  // Changed from 0 (GPIO 0 is BOOT button)
#define BATTERY_ADC_CHANNEL ADC1_CHANNEL_9  // ADC1 Channel 9 for GPIO 20

// BLE Configuration
#define DEVICE_NAME "Gauntlet-TechPad"
#define BLE_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define NOTIFICATION_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define MESSAGE_CHAR_UUID "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"
#define COMMAND_CHAR_UUID "c6b2f38c-23ab-46d8-a6ab-a3a870bbd5b6"

// WiFi Configuration (optional - can use phone relay instead)
#define WIFI_SSID "house_of_rock"
#define WIFI_PASSWORD "Dr1nkWat3r"

// NTP Time Sync Configuration
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.google.com"
#define NTP_SERVER_3 "time.cloudflare.com"
#define NTP_TIMEOUT_MS 10000
#define WIFI_CONNECT_TIMEOUT_MS 15000
#define DEFAULT_TIMEZONE "UTC0"
#define SYNC_INTERVAL_HOURS 24
#define SYNC_RETRY_HOURS 6
#define SYNC_DAILY_HOUR 3  // 3 AM

// MQTT Configuration
#define MQTT_SERVER "homeassistant.local"
#define MQTT_PORT 1883
#define MQTT_USER "gauntlet"
#define MQTT_PASSWORD "YOUR_MQTT_PASSWORD"
#define MQTT_CLIENT_ID "wearable-gauntlet"

// Power Management
#define BATTERY_MIN_VOLTAGE 3.3
#define BATTERY_MAX_VOLTAGE 4.2
#define SLEEP_TIMEOUT_MS 30000  // 30 seconds
#define LOW_BATTERY_THRESHOLD 20

// Voice AI Configuration
#define WAKE_WORD "Gauntlet"
#define SAMPLE_RATE 16000
#define RECORDING_DURATION 5000  // 5 seconds max
#define COMMAND_TIMEOUT 5000

// UI Configuration
#define SCREEN_REFRESH_RATE 30  // Hz
#define ANIMATION_FRAME_TIME 50 // ms
#define NOTIFICATION_DISPLAY_TIME 5000  // 5 seconds
#define MAX_NOTIFICATIONS 10

// Debug Settings
#define DEBUG_MODE true
#define SERIAL_DEBUG true

#endif // CONFIG_H
