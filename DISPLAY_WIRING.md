# 3.5" ILI9488 Display Wiring Guide
## Your Display → ESP32-S3 N16R8 Pin Mapping

---

## ✅ Complete Pin Mapping (Top to Bottom on Display)

```
Display Pin      →  ESP32-S3 Pin    Location    Function
──────────────────────────────────────────────────────────────
T_IRQ            →  Not connected   -           Optional touch interrupt
T_DO             →  GPIO 39         Right       Touch data out (MISO)
T_DIN            →  GPIO 9          Left        Touch data in (MOSI)
T_CS             →  GPIO 8          Left        Touch chip select
T_CLK            →  GPIO 10         Left        Touch clock

SDO (MISO)       →  GPIO 19         Right       Display data out
LED              →  GPIO 21         Right       Backlight (PWM)
SCK              →  GPIO 18         Left        SPI clock
SDI (MOSI)       →  GPIO 11         Left        Display data in
DC               →  GPIO 2          Right       Data/Command
RESET            →  GPIO 4          Left        Reset
CS               →  GPIO 15         Left        Chip select

GND              →  GND             Any         Ground
VCC              →  3.3V            Any         Power ⚠️ NOT 5V!
──────────────────────────────────────────────────────────────
```

---

## 🔌 Step-by-Step Wiring Instructions

### Preparation:
1. **Power OFF** - Disconnect USB from ESP32-S3
2. Get breadboard and jumper wires ready
3. Have display and ESP32-S3 in front of you

### Power Connections (Do These First):
```
Display VCC  →  ESP32-S3 "3.3V" pin  ⚠️ CRITICAL: Use 3.3V NOT 5V!
Display GND  →  ESP32-S3 "GND" pin
```

### SPI Display Connections (Main Display):
```
Display CS       →  ESP32 pin labeled "15" (LEFT side)
Display RESET    →  ESP32 pin labeled "4"  (LEFT side)
Display DC       →  ESP32 pin labeled "2"  (RIGHT side)
Display SDI      →  ESP32 pin labeled "11" (LEFT side)
Display SCK      →  ESP32 pin labeled "18" (LEFT side)
Display LED      →  ESP32 pin labeled "21" (RIGHT side) ⚠️ IMPORTANT!
Display SDO      →  ESP32 pin labeled "19" (RIGHT side)
```

### Touch Screen Connections:
```
Display T_CLK    →  ESP32 pin labeled "10" (LEFT side)
Display T_CS     →  ESP32 pin labeled "8"  (LEFT side)
Display T_DIN    →  ESP32 pin labeled "9"  (LEFT side)
Display T_DO     →  ESP32 pin labeled "39" (RIGHT side)
Display T_IRQ    →  Leave unconnected (not needed)
```

---

## 📍 Quick Visual Reference

**LEFT SIDE of ESP32-S3 (pins you'll use):**
```
Position    Label   Display Pin
────────────────────────────────
...
Pin 6       "4"  →  RESET
Pin 7       "5"  →  (not used)
...
Pin 9       "15" →  CS
...
Pin 11      "18" →  SCK
Pin 12      "8"  →  T_CS
...
Pin 14      "9"  →  T_DIN
Pin 15      "10" →  T_CLK
Pin 16      "11" →  SDI (MOSI)
...
```

**RIGHT SIDE of ESP32-S3 (pins you'll use):**
```
Position    Label   Display Pin
────────────────────────────────
...
Pin 6       "2"  →  DC
...
Pin 11      "39" →  T_DO
...
Pin 14      "21" →  LED (Backlight!)
Pin 15      "20" →  (not used)
Pin 16      "19" →  SDO (MISO)
...
```

---

## ⚠️ CRITICAL Safety Checks

Before powering on, verify:

1. ✅ **VCC connected to 3.3V** (NOT 5V!)
2. ✅ **GND connected to GND**
3. ✅ **LED (backlight) connected to GPIO 21**
4. ✅ **No loose wires touching each other**
5. ✅ **All connections secure**

**Double-check the 3.3V connection!** Using 5V will permanently damage the display!

---

## 🧪 Testing Procedure

### Step 1: Visual Inspection
- Check all wires are firmly connected
- Verify no shorts between adjacent pins
- Confirm 3.3V and GND are correct

### Step 2: Power On
- Connect USB to ESP32-S3
- Display backlight should turn on (if firmware already uploaded)
- No smoke, no burning smell (if you smell anything, DISCONNECT IMMEDIATELY!)

### Step 3: Upload Firmware
If you haven't uploaded the updated firmware yet:
1. In VS Code, click ✓ to rebuild (with new ILI9488 settings)
2. Click → to upload
3. Wait for SUCCESS message

### Step 4: Serial Monitor Check
1. Click 🔌 to open serial monitor
2. Look for: `Display (ILI9341 TFT)... OK` (will say ILI9341 but driver works for ILI9488)
3. Should see: `TFT Display initialized: 480x320`

### Step 5: Visual Check
- Display should show boot screen with "GAUNTLET" text
- Colors should be clear and vibrant
- Text should be readable
- After 2-3 seconds, transitions to clock screen

---

## 🐛 Troubleshooting

### Display stays completely black:
1. **Check backlight** - LED pin to GPIO 21 (most common issue!)
2. **Check power** - Measure 3.3V between VCC and GND with multimeter
3. **Check RESET** - GPIO 4 connection
4. **Try different orientation** - Change TFT_ROTATION in config.h

### Display backlight on but no image:
1. **Check SPI connections** - SDI (MOSI), SCK, CS
2. **Check DC pin** - GPIO 2
3. **Verify driver** - Should say ILI9488_DRIVER in platformio.ini
4. **Check resolution** - 480x320 in config.h

### Display shows garbage/random colors:
1. **Check MISO** - SDO to GPIO 19
2. **Reduce SPI speed** - Change SPI_FREQUENCY to 20000000 in platformio.ini
3. **Check wiring** - Loose connection on SPI pins

### Touch not responding:
1. **Check T_CS** - GPIO 8
2. **Check T_CLK** - GPIO 10
3. **Check T_DIN** - GPIO 9
4. **Check T_DO** - GPIO 39
5. **T_IRQ can stay disconnected** - It's optional

### Display works but touch doesn't:
- Run touch calibration routine
- Check touch pressure threshold
- Verify all 4 touch pins are connected (T_CS, T_CLK, T_DIN, T_DO)

---

## 📐 Pin Summary Table

| Display Pin | Type | ESP32 GPIO | Side | Wire Color Suggestion |
|-------------|------|------------|------|----------------------|
| VCC | Power | 3.3V | Any | Red |
| GND | Ground | GND | Any | Black |
| CS | SPI | 15 | Left | Yellow |
| RESET | GPIO | 4 | Left | White |
| DC | GPIO | 2 | Right | Orange |
| SDI (MOSI) | SPI | 11 | Left | Green |
| SCK | SPI | 18 | Left | Yellow |
| LED | PWM | 21 | Right | Red/White |
| SDO (MISO) | SPI | 19 | Right | Blue |
| T_CLK | Touch | 10 | Left | Purple |
| T_CS | Touch | 8 | Left | Brown |
| T_DIN | Touch | 9 | Left | Gray |
| T_DO | Touch | 39 | Right | Pink |
| T_IRQ | Touch | NC | - | None |

---

## ✅ Expected Results After Wiring

**Serial Monitor Output:**
```
╔════════════════════════════════════════╗
║  WEARABLE TECH PAD - GAUNTLET v2.0   ║
╚════════════════════════════════════════╝

┌─ PHASE 1: Core Hardware ─────────────┐
│ Display (ILI9341 TFT)...      OK ✓   │
│ Touch Screen (XPT2046)...     OK ✓   │
└───────────────────────────────────────┘

TFT Display initialized: 480x320
```

**Display Output:**
- Large cyan "GAUNTLET" text
- Version "v2.0"
- "Initializing..." message
- Transitions to clock screen showing time/date
- Touch should be responsive

---

## 🎯 Configuration Summary

**Firmware Settings:**
- Driver: ILI9488
- Resolution: 480x320 pixels
- SPI Speed: 27 MHz
- Touch: XPT2046 compatible

**Your Display Specs:**
- Size: 3.5 inches
- Resolution: 480x320
- Driver IC: ILI9488
- Interface: SPI
- Touch: Resistive (XPT2046)
- Power: 3.3V

---

Ready to wire! Start with power (VCC/GND), then the main display pins, then touch. 🚀
