# ESP32 Flashing Guide - Gauntlet Firmware

## Prerequisites
✅ ESP32-WROOM-32 board
✅ USB cable (Micro-USB or USB-C depending on your board)
✅ VS Code with PlatformIO extension installed
✅ Windows drivers (usually auto-installed)

## Your ESP32 Connection
**Detected Port:** COM10 (Silicon Labs CP210x)

---

## Method 1: VS Code PlatformIO GUI (EASIEST)

### Step 1: Open the Project
1. Open VS Code
2. Go to: `File` → `Open Folder`
3. Navigate to: `C:\Users\skygu\Desktop\Geekdad gauntlet\firmware`
4. Click `Select Folder`

### Step 2: Look for PlatformIO Icons
At the **bottom of VS Code window**, you'll see a blue status bar with icons:
```
🏠 (Home) | ✓ (Build) | → (Upload) | 🗑️ (Clean) | 🔌 (Serial Monitor)
```

### Step 3: Build the Firmware (First Time)
1. Click the **✓ (checkmark)** icon in the bottom bar
2. PlatformIO will:
   - Download required libraries (first time only, ~2-5 minutes)
   - Compile the firmware
3. Watch the **Terminal** at bottom for progress
4. Look for: `SUCCESS` or `[SUCCESS]` message

**Expected Output:**
```
Processing esp32dev (platform: espressif32; board: esp32dev; framework: arduino)
...
Compiling .pio/build/esp32dev/src/main.cpp.o
Linking .pio/build/esp32dev/firmware.elf
Building .pio/build/esp32dev/firmware.bin
RAM:   [==        ]  23.4% (used 76540 bytes)
Flash: [=====     ]  54.2% (used 711234 bytes)
======================== [SUCCESS] Took 45.23 seconds ========================
```

### Step 4: Upload to ESP32
1. Make sure ESP32 is connected to **COM10**
2. Click the **→ (right arrow)** icon in bottom bar
3. PlatformIO will:
   - Build firmware (if needed)
   - Upload to ESP32
4. Watch for: `SUCCESS` message

**Expected Output:**
```
Uploading .pio/build/esp32dev/firmware.bin
esptool.py v4.5.1
Serial port COM10
Connecting....
Chip is ESP32-D0WDQ6 (revision v1.0)
...
Writing at 0x00010000... (100%)
Wrote 711234 bytes (458901 compressed) at 0x00010000 in 6.8 seconds
Leaving...
Hard resetting via RTS pin...
======================== [SUCCESS] Took 12.45 seconds ========================
```

### Step 5: Monitor Serial Output
1. Click the **🔌 (plug)** icon in bottom bar
2. Serial monitor opens showing ESP32 output
3. Press **RESET button** on ESP32 to see boot sequence

**Expected Boot Output:**
```
╔════════════════════════════════════════╗
║  WEARABLE TECH PAD - GAUNTLET v2.0   ║
╚════════════════════════════════════════╝

┌─ PHASE 1: Core Hardware ─────────────┐
│ Display (ILI9341 TFT)...      OK     │
│ Touch Screen (XPT2046)...     OK     │
│ Sensor Manager...             OK     │
└───────────────────────────────────────┘

┌─ PHASE 2: Peripherals ────────────────┐
│ Power Manager...              OK     │
│ Bluetooth (BLE)...            OK     │
└───────────────────────────────────────┘

Device Name: Gauntlet-TechPad
Service UUID: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
BLE advertising started

System ready!
```

---

## Method 2: Command Line (Alternative)

### If VS Code method doesn't work, try this:

1. **Open Command Prompt** (cmd.exe)
2. **Navigate to firmware folder:**
   ```bash
   cd "C:\Users\skygu\Desktop\Geekdad gauntlet\firmware"
   ```

3. **Build firmware:**
   ```bash
   platformio run
   ```

4. **Upload to ESP32:**
   ```bash
   platformio run --target upload
   ```

5. **Monitor serial output:**
   ```bash
   platformio device monitor
   ```

---

## Troubleshooting

### Problem: "A serial exception error occurred"
**Solution:**
- Close any Serial Monitor windows
- Unplug ESP32, wait 5 seconds, plug back in
- Try upload again

### Problem: "Failed to connect to ESP32"
**Solution:**
- Press and HOLD the **BOOT button** on ESP32
- Click Upload in VS Code
- Release BOOT button when you see "Connecting...."

### Problem: "Port COM10 not found"
**Solution:**
- Unplug and replug USB cable
- Check Device Manager → Ports (COM & LPT)
- Install CP210x drivers from Silicon Labs website

### Problem: Compilation errors
**Solution:**
- Check Terminal output for specific error
- Common issues:
  - Missing library: PlatformIO will auto-download
  - Syntax error: Check recent code changes
  - Out of memory: Rarely happens with ESP32

### Problem: ESP32 boots but display is black
**Solution:**
- Check TFT display connections:
  - TFT_MOSI → GPIO 23
  - TFT_SCLK → GPIO 18
  - TFT_CS → GPIO 15
  - TFT_DC → GPIO 2
  - TFT_RST → GPIO 4
  - TFT_BL → GPIO 21 (backlight - most important!)
- Verify 3.3V power (NOT 5V!)

### Problem: Upload works but serial output is garbage
**Solution:**
- Check baud rate: Must be **115200**
- In VS Code serial monitor: Look for baud rate selector
- Change to 115200 if different

---

## Quick Reference

### VS Code PlatformIO Shortcuts
- **Build:** `Ctrl + Alt + B`
- **Upload:** `Ctrl + Alt + U`
- **Clean:** `Ctrl + Alt + C`
- **Serial Monitor:** `Ctrl + Alt + S`

### PlatformIO Commands
```bash
pio run                    # Build
pio run -t upload          # Upload
pio device monitor         # Serial monitor
pio run -t clean           # Clean build files
pio device list            # List connected devices
```

### Serial Monitor Controls
- **Exit:** `Ctrl + C`
- **Clear screen:** `Ctrl + L`
- **Reset ESP32:** Press physical RESET button

---

## Next Steps After Successful Upload

1. **Verify boot sequence** - Should show all phases
2. **Check display** - Should show boot screen then clock
3. **Test touch** - Tap screen to wake if sleeping
4. **Check sensors** - View sensor data on display
5. **Test buttons** - Press physical buttons (GPIO 34, 35, 36)
6. **Check BLE** - Use nRF Connect app on phone to see "Gauntlet-TechPad"

---

## Hardware Assembly Checklist

Before flashing, ensure you have:
- ☐ ESP32-WROOM-32 board
- ☐ USB cable connected to computer
- ☐ (Optional) 2.8" ILI9341 display connected
- ☐ (Optional) Battery voltage divider on GPIO 0
- ☐ (Optional) Buttons on GPIO 34, 35, 36
- ☐ (Optional) Vibration motor on GPIO 5

**Note:** The firmware will boot and work even without peripherals connected. You'll see error messages for missing hardware in serial output, but core system will still run.

---

## Support

If you encounter issues:
1. Check the serial monitor output for error messages
2. Verify hardware connections match config.h
3. Post issue to: https://github.com/anthropics/claude-code/issues
4. Include: Error message, serial output, hardware configuration

---

**Last Updated:** 2026-01-22
**Firmware Version:** v2.0
**Board:** ESP32-WROOM-32
**Upload Port:** COM10
