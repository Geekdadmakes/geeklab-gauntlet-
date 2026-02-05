# ESP32-S3 N16R8 Wiring Guide - CORRECTED FOR YOUR BOARD

## ✅ Your Board's Available Pins

**LEFT SIDE (top to bottom):**
3.3V, GND, **4, 5, 6, 7, 15, 16, 17, 18, 8, 3**, 9, 10, 11, 12, 13, 14, GND, 3.3V

**RIGHT SIDE (top to bottom):**
3.3V, 5V, GND, **1, 2, 42, 41, 40, 39, 38**, 48, 47, **21, 20, 19**, GND, GND, 5V, 3.3V

---

## 🎨 Updated Pin Assignments (ALL VERIFIED AVAILABLE!)

### 📺 Display (2.8" ILI9341 TFT) - SPI

```
Display Pin     →  ESP32-S3 Pin   Location    Notes
─────────────────────────────────────────────────────────
VCC             →  3.3V           Power       ⚠️ NOT 5V!
GND             →  GND            Ground
MOSI (SDI)      →  GPIO 11        Left side   ✓ CORRECTED
MISO (SDO)      →  GPIO 19        Right side
SCK (CLK)       →  GPIO 18        Left side
CS              →  GPIO 15        Left side
DC (RS)         →  GPIO 2         Right side
RST (RESET)     →  GPIO 4         Left side
LED (Backlight) →  GPIO 21        Right side  Most important!
```

### 👆 Touch Screen (XPT2046) - Shares SPI Bus

```
Touch Pin       →  ESP32-S3 Pin   Location    Notes
─────────────────────────────────────────────────────────
T_CLK           →  GPIO 10        Left side   ✓ CORRECTED
T_MOSI          →  GPIO 9         Left side   ✓ CORRECTED
T_MISO          →  GPIO 39        Right side
T_CS            →  GPIO 8         Left side   ✓ CORRECTED
T_IRQ           →  Not connected  Optional
```

---

### 🌡️ I2C Bus 1 (Environmental Sensors)

All three sensors share these two pins:

```
I2C Signal      →  ESP32-S3 Pin   Location    Notes
─────────────────────────────────────────────────────────
SDA (Data)      →  GPIO 6         Left side   ✓ CORRECTED
SCL (Clock)     →  GPIO 7         Left side   ✓ CORRECTED
```

#### BME280 (Temperature/Humidity/Pressure)
```
BME280 Pin      →  Connection
──────────────────────────────
VCC             →  3.3V
GND             →  GND
SDA             →  GPIO 6
SCL             →  GPIO 7
```
**I2C Address:** 0x76 (or 0x77)

#### MPU6050 (Accelerometer/Gyroscope)
```
MPU6050 Pin     →  Connection      Notes
─────────────────────────────────────────────
VCC             →  3.3V
GND             →  GND
SDA             →  GPIO 6          Shared with BME280
SCL             →  GPIO 7          Shared with BME280
AD0             →  3.3V            ⚠️ CRITICAL! Sets address to 0x69
INT             →  Not connected
```
**I2C Address:** 0x69 (with AD0 connected to 3.3V)

#### PCF8574 (I/O Expander)
```
PCF8574 Pin     →  Connection      Notes
─────────────────────────────────────────────
VCC             →  3.3V
GND             →  GND
SDA             →  GPIO 6          Shared
SCL             →  GPIO 7          Shared
A0, A1, A2      →  GND             All address pins to GND
```
**I2C Address:** 0x20

---

### ⏰ I2C Bus 2 (RTC - Separate Bus!)

```
DS3231 Pin      →  ESP32-S3 Pin   Location    Notes
─────────────────────────────────────────────────────────
VCC             →  3.3V (or 5V)
GND             →  GND
SDA             →  GPIO 17        Left side
SCL             →  GPIO 16        Left side
32K, SQW        →  Not connected
```
**I2C Address:** 0x68
**Why separate bus?** Avoids address conflict with MPU6050

---

### 🛰️ GPS (NEO6M) - UART

```
GPS Pin         →  ESP32-S3 Pin   Location    Notes
─────────────────────────────────────────────────────────
VCC             →  3.3V or 5V
GND             →  GND
TX              →  GPIO 3 (RX)    Left side   GPS sends to ESP32
RX              →  GPIO 1 (TX)    Right side  ESP32 sends to GPS
PPS             →  Not connected
```
**Baud Rate:** 9600

---

### 🎤 Microphone (INMP441) - I2S

```
INMP441 Pin     →  ESP32-S3 Pin   Location    Notes
─────────────────────────────────────────────────────────
VDD             →  3.3V
GND             →  GND
L/R             →  GND                         Left channel
WS              →  GPIO 47        Right side  ✓ CORRECTED
SD              →  GPIO 14        Left side
SCK             →  GPIO 12        Left side
```

---

### 🎮 User Interface

#### Buttons (3x Tactile)

```
Button          →  ESP32-S3 Pin   Location    Wiring
─────────────────────────────────────────────────────────
Button 1        →  GPIO 40        Right side  ✓ CORRECTED
Button 2        →  GPIO 41        Right side  ✓ CORRECTED
Button 3        →  GPIO 42        Right side  ✓ CORRECTED
```

**Wiring (Simple - Using Internal Pull-ups):**
```
Each button:
  One leg → GPIO pin
  Other leg → GND
```

**Wiring (with External Pull-ups if needed):**
```
GPIO pin → 10kΩ resistor → 3.3V
GPIO pin → Button → GND
```

#### Vibration Motor
```
Motor Pin       →  Connection
──────────────────────────────
+               →  GPIO 5 (Left side)
-               →  GND
```

#### Buzzer
```
Buzzer Pin      →  Connection
──────────────────────────────
+               →  GPIO 13 (Left side)
-               →  GND
```

---

### 🔋 Battery Monitoring

**Voltage Divider Circuit:**
```
Battery + → 10kΩ resistor → GPIO 20 (Right side) ✓ CORRECTED
                             ↓
                          10kΩ resistor
                             ↓
                            GND
```

This divides battery voltage by 2, making 4.2V safe for the 3.3V ADC.

---

## 📋 Complete Pin Usage Summary

```
GPIO    Function            Location    Interface
──────────────────────────────────────────────────────
1       GPS TX              Right       UART
2       TFT DC              Right       SPI
3       GPS RX              Left        UART
4       TFT RST             Left        SPI
5       Vibration Motor     Left        GPIO Output
6       I2C1 SDA            Left        I2C (Sensors)
7       I2C1 SCL            Left        I2C (Sensors)
8       Touch CS            Left        SPI-Touch
9       Touch MOSI          Left        SPI-Touch
10      Touch CLK           Left        SPI-Touch
11      TFT MOSI            Left        SPI
12      I2S SCK             Left        I2S
13      Buzzer              Left        GPIO Output
14      I2S SD              Left        I2S
15      TFT CS              Left        SPI
16      I2C2 SCL            Left        I2C (RTC)
17      I2C2 SDA            Left        I2C (RTC)
18      TFT SCLK            Left        SPI
19      TFT MISO            Right       SPI
20      Battery ADC         Right       ADC
21      TFT Backlight       Right       PWM
39      Touch MISO          Right       SPI-Touch
40      Button 1            Right       Input
41      Button 2            Right       Input
42      Button 3            Right       Input
47      I2S WS              Right       I2S

UNUSED: 38, 48 (available for future expansion)
```

---

## 🚀 Step-by-Step Wiring Process

### Step 1: Power Distribution
1. Power OFF - unplug USB
2. On breadboard, connect ESP32-S3's 3.3V to (+) power rail
3. Connect ESP32-S3's GND to (-) ground rail
4. All component VCC → (+) rail
5. All component GND → (-) rail

### Step 2: Display (HIGHEST PRIORITY)
Wire the display first - gives immediate visual feedback!

**Left side pins needed:** 4, 8, 9, 10, 11, 15, 18
**Right side pins needed:** 2, 19, 21

1. VCC → 3.3V rail (NOT 5V!)
2. GND → GND rail
3. MOSI → GPIO 11 (left)
4. MISO → GPIO 19 (right)
5. SCK → GPIO 18 (left)
6. CS → GPIO 15 (left)
7. DC → GPIO 2 (right)
8. RST → GPIO 4 (left)
9. LED → GPIO 21 (right) ⚠️ IMPORTANT!
10. T_CLK → GPIO 10 (left)
11. T_MOSI → GPIO 9 (left)
12. T_MISO → GPIO 39 (right)
13. T_CS → GPIO 8 (left)

**Power ON and test!**

### Step 3: I2C Bus 1 (Sensors)
**Pins needed:** GPIO 6 (SDA), GPIO 7 (SCL) - both on left

1. Wire all three sensors in parallel:
   - All SDA pins → GPIO 6
   - All SCL pins → GPIO 7
2. BME280: VCC, GND, SDA, SCL
3. MPU6050: VCC, GND, SDA, SCL, **AD0 → 3.3V**
4. PCF8574: VCC, GND, SDA, SCL, A0/A1/A2 → GND

### Step 4: RTC (I2C Bus 2)
**Pins needed:** GPIO 16 (SCL), GPIO 17 (SDA) - both on left

1. DS3231: VCC, GND, SDA → GPIO 17, SCL → GPIO 16

### Step 5: GPS
**Pins needed:** GPIO 1 (TX-right), GPIO 3 (RX-left)

1. GPS TX → GPIO 3
2. GPS RX → GPIO 1

### Step 6: Buttons & Peripherals
**Pins needed:** GPIO 5, 13 (left), GPIO 40, 41, 42 (right)

1. Three buttons → GPIO 40, 41, 42 (other leg to GND)
2. Vibration motor → GPIO 5
3. Buzzer → GPIO 13

### Step 7: Microphone
**Pins needed:** GPIO 12, 14 (left), GPIO 47 (right)

1. INMP441: WS → 47, SD → 14, SCK → 12, L/R → GND

### Step 8: Battery (LAST!)
**Pin needed:** GPIO 20 (right)

1. Build voltage divider carefully
2. Connect to GPIO 20

---

## ⚠️ Critical Safety Notes

1. **3.3V ONLY for display** - 5V will destroy it!
2. **MPU6050 AD0 to 3.3V** - Required for correct I2C address
3. **Power off while wiring** - Prevents shorts
4. **Double-check before power on** - Wrong pins can damage components

---

## 🧪 Testing After Wiring

After wiring display:
```
1. Upload firmware (rebuild first with new pins)
2. Open serial monitor (115200 baud)
3. Should see: "Display (ILI9341 TFT)... OK"
4. Display shows: Boot screen → Clock screen
5. Touch screen should respond
```

---

## 🎯 What Changed from Original Config?

```
OLD PIN → NEW PIN   Reason
────────────────────────────────────────
GPIO 23 → GPIO 11   GPIO 23 not available on N16R8
GPIO 27 → GPIO 6    GPIO 27 used by PSRAM
GPIO 22 → GPIO 7    GPIO 22 not available on N16R8
GPIO 25 → GPIO 10   GPIO 25 not available on N16R8
GPIO 32 → GPIO 9    GPIO 32 used by PSRAM
GPIO 33 → GPIO 8    GPIO 33 used by PSRAM
GPIO 26 → GPIO 47   GPIO 26 used by PSRAM
GPIO 34 → GPIO 40   GPIO 34 not available on N16R8
GPIO 35 → GPIO 41   GPIO 35 conflicts with Octal PSRAM
GPIO 36 → GPIO 42   GPIO 36 conflicts with Octal PSRAM
GPIO 0  → GPIO 20   GPIO 0 is BOOT button
```

---

Ready to start wiring? **Begin with the display!** 🚀
