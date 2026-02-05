# Firmware Setup Guide

This guide explains how to set up the development environment and flash firmware to your Wearable Tech Pad.

## Prerequisites

### Required Software
- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE Extension](https://platformio.org/install/ide?install=vscode)
- [Git](https://git-scm.com/)
- USB drivers for ESP32-S3

### System Requirements
- Windows 10/11, macOS 10.14+, or Linux
- 4GB RAM minimum
- USB-C cable for programming

## Development Environment Setup

### 1. Install Visual Studio Code

Download and install VS Code from https://code.visualstudio.com/

### 2. Install PlatformIO Extension

1. Open VS Code
2. Click Extensions icon (or press Ctrl+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install
5. Restart VS Code after installation

### 3. Clone or Open Project

```bash
cd "C:\Users\skygu\Desktop\Geekdad gauntlet"
code .
```

Or open the folder in VS Code: File → Open Folder → Select project directory

## Project Structure

```
firmware/
├── platformio.ini          # Project configuration
├── src/
│   ├── main.cpp            # Main application
│   ├── display_manager.cpp # Display control
│   ├── bluetooth_service.cpp # BLE communication
│   ├── power_manager.cpp   # Power management
│   ├── voice_ai.cpp        # Voice processing
│   ├── notification_handler.cpp # Notifications
│   └── smart_home_mqtt.cpp # MQTT/Smart home
├── include/
│   ├── config.h            # Configuration constants
│   └── [module headers]    # Header files
└── lib/                    # External libraries
```

## Configuration

### 1. Update WiFi Credentials (Optional)

Edit `firmware/include/config.h`:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

**Note:** WiFi is optional. The device can work entirely through iPhone BLE relay.

### 2. Update MQTT Settings (Optional)

For smart home integration, edit `firmware/include/config.h`:

```cpp
#define MQTT_SERVER "homeassistant.local"
#define MQTT_PORT 1883
#define MQTT_USER "gauntlet"
#define MQTT_PASSWORD "YOUR_MQTT_PASSWORD"
```

### 3. Adjust Pin Assignments

If your hardware uses different pins, update in `firmware/include/config.h`:

```cpp
#define I2C_SDA 21
#define I2C_SCL 22
#define BUTTON_1 19
// etc.
```

## Building the Firmware

### 1. Open PlatformIO Terminal

In VS Code:
- Click PlatformIO icon in sidebar
- Click "Open" under PIO Home
- Or use shortcut: Ctrl+Alt+T

### 2. Build Project

```bash
pio run
```

This will:
- Download required libraries
- Compile all source files
- Link the final firmware binary

### 3. Check for Errors

If build fails:
- Read error messages carefully
- Check for missing libraries in `platformio.ini`
- Verify syntax in source files
- Ensure all header files are present

## Uploading to ESP32-S3

### 1. Connect Hardware

1. Connect ESP32-S3 to computer via USB-C cable
2. Put device in boot mode if needed:
   - Hold BOOT button
   - Press RESET button
   - Release RESET
   - Release BOOT

### 2. Select Port

PlatformIO should auto-detect the port. If not:

Windows:
```bash
pio device list
```

Look for `COM3`, `COM4`, etc.

macOS/Linux:
```bash
pio device list
```

Look for `/dev/ttyUSB0` or `/dev/cu.usbserial-*`

### 3. Upload Firmware

```bash
pio run --target upload
```

Or use VS Code:
- Click PlatformIO icon
- Expand `esp32-s3-devkitc-1`
- Click "Upload"

### 4. Monitor Serial Output

```bash
pio device monitor
```

Or in VS Code:
- Click "Monitor" in PlatformIO tasks

You should see:
```
=== WEARABLE TECH PAD STARTING ===
Initializing display...
Display initialized
Initializing power management...
Power Manager initialized
Initializing Bluetooth...
BLE advertising started
=== SYSTEM READY ===
```

## Troubleshooting

### Upload Failed

**Error: "Failed to connect to ESP32"**

Solution:
1. Check USB cable (must support data, not just charging)
2. Try different USB port
3. Install/update USB drivers
4. Put device in boot mode manually

**Error: "Serial port not found"**

Solution:
1. Check Device Manager (Windows) or `ls /dev/tty*` (Mac/Linux)
2. Install CH340/CP2102 drivers if needed
3. Try different USB cable

### Compilation Errors

**Error: "Library not found"**

Solution:
```bash
pio lib install
```

Or add to `platformio.ini`:
```ini
lib_deps =
    adafruit/Adafruit GFX Library@^1.11.9
    # etc.
```

**Error: "Undefined reference"**

Solution:
- Check that all .cpp files are in `src/` directory
- Verify all headers are in `include/`
- Ensure function declarations match implementations

### Runtime Errors

**Display not showing anything**

1. Check I2C connections (SDA, SCL)
2. Verify I2C address in Serial monitor
3. Run I2C scanner:
   ```cpp
   Wire.begin();
   Wire.beginTransmission(0x3C);
   byte error = Wire.endTransmission();
   ```

**BLE not working**

1. Check Serial output for BLE initialization
2. Verify UUIDs match iOS app
3. Restart device
4. Clear iPhone Bluetooth cache (Settings → Bluetooth → Forget Device)

## Advanced Configuration

### Enable Debug Output

In `firmware/include/config.h`:

```cpp
#define DEBUG_MODE true
#define SERIAL_DEBUG true
#define CORE_DEBUG_LEVEL 3
```

### Optimize Power Consumption

1. Reduce display refresh rate:
   ```cpp
   #define SCREEN_REFRESH_RATE 15  // Lower = less power
   ```

2. Increase sleep timeout:
   ```cpp
   #define SLEEP_TIMEOUT_MS 15000  // 15 seconds
   ```

3. Reduce CPU frequency in `platformio.ini`:
   ```ini
   board_build.f_cpu = 160000000L  # 160MHz instead of 240MHz
   ```

### Add Custom Voice Commands

Edit `firmware/src/voice_ai.cpp`:

```cpp
CommandType VoiceAI::parseCommand(const char* text) {
    String textStr = String(text);
    textStr.toLowerCase();

    // Add your custom command
    if (textStr.indexOf("your keyword") >= 0) {
        return CMD_CUSTOM_ACTION;
    }

    // ... existing code
}
```

## Over-the-Air (OTA) Updates

### Enable OTA in platformio.ini

```ini
upload_protocol = espota
upload_port = 192.168.1.XXX  ; ESP32 IP address
```

### Upload via OTA

```bash
pio run --target upload
```

This requires:
- ESP32 connected to WiFi
- OTA library included
- OTA handlers in code

## Testing

### Unit Tests

Create test files in `test/` directory:

```cpp
#include <unity.h>

void test_battery_percentage() {
    TEST_ASSERT_EQUAL(100, powerMgr.getBatteryPercentage());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_battery_percentage);
    UNITY_END();
}

void loop() {}
```

Run tests:
```bash
pio test
```

### Integration Tests

1. Test display output
2. Test button inputs
3. Test BLE connectivity
4. Test power consumption
5. Test voice input

## Performance Monitoring

### Memory Usage

Check during compilation:
```
RAM:   [====      ]  42.3% (used 138476 bytes from 327680 bytes)
Flash: [======    ]  58.7% (used 1218844 bytes from 2097152 bytes)
```

Optimize if RAM > 80%:
- Reduce buffer sizes
- Use PROGMEM for constants
- Optimize data structures

### CPU Usage

Monitor in Serial output:
```cpp
unsigned long loopTime = millis() - lastLoop;
Serial.println(loopTime);  // Should be <50ms for responsive UI
```

## Next Steps

After successful firmware upload:

1. Test all hardware components
2. Pair with iOS companion app
3. Test voice commands
4. Configure smart home integration
5. Perform battery life testing

## Resources

- PlatformIO Documentation: https://docs.platformio.org/
- ESP32-S3 Reference: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/
- Arduino ESP32 Core: https://github.com/espressif/arduino-esp32
- NimBLE Arduino: https://github.com/h2zero/NimBLE-Arduino
