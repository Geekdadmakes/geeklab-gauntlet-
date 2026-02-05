# Wearable Tech Pad (Gauntlet v2.0)

A Pip-Boy inspired wrist-mounted computer with 2.8" color touchscreen, multi-sensor suite, AI voice interaction, iPhone integration, and smart home control capabilities.

![Project Status](https://img.shields.io/badge/status-in_development-yellow)
![Platform](https://img.shields.io/badge/platform-ESP32--WROOM--32-blue)
![Display](https://img.shields.io/badge/display-2.8%22_ILI9341-green)
![License](https://img.shields.io/badge/license-MIT-green)

## Features

### Display & Interface
- 🖥️ **2.8" Color TFT Display** - 320x240 ILI9341 with 65K colors
- 👆 **Resistive Touch Screen** - Full touch navigation with gesture support
- 🎨 **Multi-Screen UI** - Clock, sensors, GPS, notifications, settings, and more
- ⚡ **Smooth Graphics** - 20+ FPS rendering with custom UI framework

### Sensors & Tracking
- 🌡️ **Environmental Sensing** - BME280 (temperature, humidity, pressure, altitude)
- 📍 **GPS Navigation** - NEO6M with speed, coordinates, and satellite tracking
- 🏃 **Motion Tracking** - MPU6050 IMU with step counter and orientation
- 🕐 **Real-Time Clock** - DS3231 with battery backup for accurate timekeeping
- 🔌 **I/O Expansion** - PCF8574 for future sensor additions

### Communication & Control
- 🎙️ **Voice AI Assistant** - INMP441 microphone with wake word detection
- 📱 **iPhone Integration** - Receive notifications and send text replies via BLE
- 🏠 **Smart Home Control** - Control devices via MQTT (Home Assistant compatible)
- 🔋 **Massive Battery Life** - 10,000mAh UPS pack = 40-60 hour runtime!
- ⌚ **Wearable Design** - Pip-Boy style forearm mount with tactile buttons

## Project Structure

```
Geekdad gauntlet/
├── firmware/              # ESP32-S3 firmware
│   ├── src/              # Source code
│   ├── include/          # Header files
│   └── platformio.ini    # Build configuration
├── ios-app/              # iOS companion app
│   ├── *.swift           # Swift source files
│   └── README.md         # iOS setup guide
├── enclosure/            # 3D printable case files
├── docs/                 # Documentation
│   ├── hardware-assembly.md
│   ├── firmware-setup.md
│   ├── ios-app-setup.md
│   └── user-manual.md
└── README.md             # This file
```

## Hardware

### Core Components (All Owned! ✓)

| Component | Purpose | Notes |
|-----------|---------|-------|
| **ESP32-WROOM-32** | Main controller | 38-pin dev board with WiFi/BLE |
| **2.8" ILI9341 TFT** | Color display (320x240) | SPI interface with touch |
| **XPT2046 Touch** | Touch screen controller | Resistive touch (integrated with display) |
| **INMP441 Microphone** | Voice input (I2S digital) | High-quality MEMS mic |
| **TP4056 Charger** | USB-C charging circuit | For battery management |
| **Vibration Motor** | Haptic feedback | 3V motor |
| **Passive Buzzer** | Audio alerts | 5V buzzer |
| **Tactile Buttons (x3)** | Physical input | 6x6mm buttons |

### Sensor Suite (All Owned! ✓)

| Sensor | Measurements | Interface |
|--------|-------------|-----------|
| **BME280** | Temperature, Humidity, Pressure, Altitude | I2C (0x76) |
| **GPS NEO6M** | Location, Speed, Satellites | UART (9600 baud) |
| **DS3231 RTC** | Date/Time with battery backup | I2C (0x68) |
| **MPU6050 IMU** | 6-axis motion (accel + gyro) | I2C (0x69) |
| **PCF8574** | 8 additional GPIO pins | I2C (0x20) |

### Power System

| Component | Capacity | Runtime |
|-----------|----------|---------|
| **MakerFocus 10,000mAh UPS** | 10,000mAh @ 5V | 40-60 hours typical! |
| Alternative: LiPo battery | 1500-2000mAh | 10-15 hours typical |

**Total Component Cost:** ~$0 additional (all owned!) + ~$15 for breadboard/wires/connectors

See [docs/hardware-assembly.md](docs/hardware-assembly.md) for complete pin assignments and assembly instructions.

## Quick Start

### 1. Hardware Assembly

Follow the [Hardware Assembly Guide](docs/hardware-assembly.md) to build your device.

**Summary:**
1. Gather components
2. Assemble breadboard prototype
3. Test all components
4. Solder permanent connections
5. 3D print enclosure (optional)

### 2. Firmware Setup

Follow the [Firmware Setup Guide](docs/firmware-setup.md) to flash the software.

**Summary:**
```bash
# Install PlatformIO
# Clone/download project
cd "C:\Users\skygu\Desktop\Geekdad gauntlet\firmware"

# Build and upload
pio run --target upload

# Monitor serial output
pio device monitor
```

### 3. iOS App Setup

Follow the [iOS App Setup Guide](docs/ios-app-setup.md) to install the companion app.

**Summary:**
1. Create Xcode project
2. Add Swift source files
3. Configure capabilities
4. Build and run on iPhone
5. Grant permissions

### 4. Pairing

1. Power on Gauntlet
2. Open iOS app
3. Tap "Scan for Gauntlet"
4. Select device to connect
5. Vibration confirms pairing

## Usage

### Basic Operation

**Buttons:**
- Button 1 (Left): Menu/Select, Long-press for Voice
- Button 2 (Bottom): Previous/Up
- Button 3 (Right): Next/Down

**Modes:**
- Clock: Default display
- Notifications: View iPhone notifications
- Voice: Speak commands
- Smart Home: Control devices
- Menu: Navigate features

### Voice Commands

Activate with wake word "Gauntlet" or long-press Button 1:

- "What time is it?"
- "Check battery"
- "Show notifications"
- "Turn on living room lights"
- "What's the weather?"
- Ask any question (relayed to OpenAI)

See [User Manual](docs/user-manual.md) for complete usage guide.

## Configuration

### WiFi (Optional)

Edit `firmware/include/config.h`:
```cpp
#define WIFI_SSID "YourNetwork"
#define WIFI_PASSWORD "YourPassword"
```

### MQTT/Smart Home

Edit `firmware/include/config.h`:
```cpp
#define MQTT_SERVER "homeassistant.local"
#define MQTT_USER "gauntlet"
#define MQTT_PASSWORD "YourPassword"
```

### OpenAI Integration

Edit `ios-app/VoiceCommandRelay.swift`:
```swift
private let openAIKey = "sk-YourAPIKey"
```

## Architecture

### System Overview

```
┌─────────────────┐
│  Wearable       │
│  (ESP32-S3)     │
│  - Display      │
│  - Buttons      │
│  - Microphone   │
│  - Speaker      │
└────────┬────────┘
         │ BLE 5.0
         │
┌────────▼────────┐
│  iPhone App     │
│  - Notification │
│    Relay        │
│  - Text Reply   │
│  - Voice→Cloud  │
└────────┬────────┘
         │
    ┌────┴────┐
    │         │
    ▼         ▼
┌────────┐ ┌──────────┐
│ OpenAI │ │ Home     │
│ API    │ │ Assistant│
└────────┘ └──────────┘
```

### Firmware Architecture

**Main Components:**
- `main.cpp`: Application entry point with phased initialization and event loop
- `display_manager`: ILI9341 TFT UI rendering (320x240 RGB565 color)
- `touch_manager`: XPT2046 touch input with gesture detection
- `sensor_manager`: Unified interface for all sensors (BME280, GPS, RTC, IMU, PCF8574)
- `ui_framework`: Multi-screen navigation and state management
- `bluetooth_service`: BLE communication with iPhone
- `power_manager`: Battery monitoring and sleep modes
- `voice_ai`: Wake word detection and command parsing
- `notification_handler`: Notification display and management
- `smart_home_mqtt`: MQTT client for home automation

**UI Screens:**
- Clock Screen: Large time display with weather (temp/humidity)
- Sensor Dashboard: All sensor readings in real-time
- GPS Screen: Location, speed, satellites, coordinates
- Notifications: iPhone notification list with touch navigation
- Smart Home: Device control with status indicators
- Settings: Brightness, WiFi/BLE toggles, system info
- Menu: Touch-friendly navigation between screens

### Communication Protocol

**BLE Services:**
- Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Notification Characteristic: iPhone → Gauntlet
- Message Characteristic: Gauntlet → iPhone (text replies)
- Command Characteristic: Bidirectional (voice, queries)

**Message Format:**
```
Notification: "TYPE|TITLE|MESSAGE"
Text Reply: "RECIPIENT|MESSAGE"
Voice Query: "QUERY_TEXT"
```

## Customization

### Adding Voice Commands

Edit `firmware/src/voice_ai.cpp`:

```cpp
CommandType VoiceAI::parseCommand(const char* text) {
    if (strstr(text, "your keyword")) {
        return CMD_CUSTOM;
    }
    // ...
}
```

### Modifying UI

Edit `firmware/src/display_manager.cpp`:

```cpp
void DisplayManager::showClock() {
    // Customize clock display
}
```

### Quick Reply Templates

Edit `ios-app/MessageHandler.swift`:

```swift
enum QuickReplyTemplate {
    case custom = "Your custom reply"
    // ...
}
```

## Power Consumption

**Measured Current Draw:**
- Deep Sleep: ~20μA
- Idle (display on): 80-120mA
- Active use: 150-250mA
- Voice listening: 200-300mA

**Battery Life (1000mAh):**
- Light use: 36-48 hours
- Moderate use: 24-36 hours
- Heavy use: 12-24 hours

## Troubleshooting

### Common Issues

**Display not working:**
- Check I2C connections (SDA, SCL)
- Verify I2C address (0x3C)
- Run I2C scanner

**BLE won't connect:**
- Verify UUIDs match in firmware and iOS app
- Restart both devices
- Check BLE is advertising

**Voice commands not working:**
- Check microphone connections
- Verify I2S configuration
- Test with audio visualization

See [User Manual - Troubleshooting](docs/user-manual.md#troubleshooting) for more solutions.

## Development

### Prerequisites

**Firmware:**
- Visual Studio Code
- PlatformIO IDE extension
- USB-C cable

**iOS App:**
- macOS with Xcode 14+
- Apple Developer account
- iPhone with iOS 15+

### Building from Source

**Firmware:**
```bash
cd firmware
pio run
pio run --target upload
```

**iOS App:**
1. Create Xcode project
2. Add source files
3. Build and run

### Testing

**Unit Tests:**
```bash
cd firmware
pio test
```

**Integration Tests:**
- Test each hardware component
- Test BLE connectivity
- Test end-to-end scenarios

## Roadmap

### Phase 1 ✅ (Complete)
- [x] Hardware prototype
- [x] Basic firmware
- [x] Display rendering
- [x] BLE communication

### Phase 2 🚧 (In Progress)
- [x] iOS companion app
- [ ] Notification relay
- [ ] Voice AI integration
- [ ] Smart home control

### Phase 3 📋 (Planned)
- [ ] 3D printed enclosure
- [ ] Battery optimization
- [ ] OTA firmware updates
- [ ] Advanced voice features

### Future Enhancements
- GPS module
- Heart rate monitoring
- Solar charging
- Weather widget
- Calendar integration
- Fitness tracking

## Contributing

This is a personal project, but suggestions are welcome!

1. Fork the repository
2. Create feature branch
3. Commit changes
4. Submit pull request

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

**Attribution:**
- ESP32 Arduino Core: Apache 2.0
- Adafruit Libraries: BSD License
- NimBLE: Apache 2.0

## Acknowledgments

- **Espressif** for ESP32-S3 platform
- **Adafruit** for excellent libraries and tutorials
- **PlatformIO** for development environment
- **Community** for inspiration and support

Special thanks to the maker community for sharing knowledge and projects that made this possible.

## Resources

### Documentation
- [Hardware Assembly Guide](docs/hardware-assembly.md)
- [Firmware Setup Guide](docs/firmware-setup.md)
- [iOS App Setup Guide](docs/ios-app-setup.md)
- [User Manual](docs/user-manual.md)

### External Resources
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [PlatformIO Docs](https://docs.platformio.org/)
- [Apple Core Bluetooth Guide](https://developer.apple.com/bluetooth/)
- [Home Assistant MQTT](https://www.home-assistant.io/integrations/mqtt/)

### Similar Projects
- [Pip-Boy 3000 Mk IV](https://www.thingiverse.com/thing:1069781)
- [Open-SmartWatch](https://open-smartwatch.github.io/)
- [Watchy by SQFMI](https://watchy.sqfmi.com/)

## Contact

For questions, issues, or suggestions:
- Open an issue on GitHub
- Email: [your-email@example.com]
- Twitter: [@yourhandle]

---

**Built with ❤️ by a geek dad for geek dads everywhere**

*"I don't want to set the world on fire..." 🎵*
