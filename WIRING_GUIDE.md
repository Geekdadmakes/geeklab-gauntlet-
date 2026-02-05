# ESP32-S3 Gauntlet - Complete Wiring Guide

## ⚠️ CRITICAL SAFETY WARNINGS

1. **NEVER connect 5V to ESP32 GPIO pins!** - Use only 3.3V
2. **Power off before wiring** - Disconnect USB while making connections
3. **Double-check polarity** - Wrong polarity can damage components
4. **Use common ground** - All components must share the same GND

---

## 🔧 Tools & Materials Needed

- Breadboard (830 tie-points recommended)
- Jumper wires (male-to-male, male-to-female)
- ESP32-S3 DevKit
- USB cable for power and programming
- Multimeter (optional but helpful)
- Wire strippers (if using solid core wire)

---

## 📋 Component Checklist

✅ You have these:
- ESP32-S3 Development Board
- 2.8" ILI9341 TFT LCD (320x240, SPI, with touch)
- BME280 Sensor (temp/humidity/pressure)
- GPS NEO6M Module
- DS3231 RTC Module
- MPU6050 IMU (accel/gyro)
- PCF8574 I/O Expander
- INMP441 MEMS Microphone
- TP4056 Type-C Charging Module
- Vibration Motor (3V)
- Passive Buzzer (5V)
- 3x Tactile Buttons (6x6mm)

❌ Need to add:
- LiPo Battery (recommendation: 1500-2000mAh with JST connector)
- 2x 10kΩ resistors (for battery voltage divider)
- 3x 10kΩ resistors (for button pull-ups, if needed)

---

## 🎨 Color Coding Convention

For easy identification, use these wire colors:
- **RED** = 3.3V power
- **BLACK** = Ground (GND)
- **YELLOW** = SPI CLK (clock signals)
- **GREEN** = SPI MOSI (data out)
- **BLUE** = SPI MISO (data in)
- **ORANGE** = I2C SDA (data)
- **PURPLE** = I2C SCL (clock)
- **WHITE** = UART/GPIO/Other signals

---

## 📍 ESP32-S3 Pinout Reference

```
ESP32-S3 DevKitC-1 (38-pin typical layout)

Left Side (top to bottom):        Right Side (top to bottom):
3V3  ●                          ● GND
3V3  ●                          ● GPIO 43 (TX)
RST  ●                          ● GPIO 44 (RX)
GPIO 4  ●                       ● GPIO 1
GPIO 5  ●                       ● GPIO 2
GPIO 6  ●                       ● GPIO 42
GPIO 7  ●                       ● GPIO 41
GPIO 15 ●                       ● GPIO 40
GPIO 16 ●                       ● GPIO 39 (MTCK)
GPIO 17 ●                       ● GPIO 38 (MTDO)
GPIO 18 ●                       ● GPIO 37 (MTDI)
GPIO 8  ●                       ● GPIO 36 (MTMS)
GPIO 3  ●                       ● GPIO 35
GPIO 46 ●                       ● GPIO 0
GPIO 9  ●                       ● GPIO 45
GPIO 10 ●                       ● GPIO 48
GPIO 11 ●                       ● GPIO 47
GPIO 12 ●                       ● GPIO 21
GPIO 13 ●                       ● GPIO 20
GPIO 14 ●                       ● GPIO 19
GND  ●                          ● GND
```

---

## 🚀 Step-by-Step Wiring Guide

### Phase 1: Display & Touch (PRIORITY - Visual Feedback)

This is the most important component - you'll see immediate feedback!

#### 2.8" ILI9341 TFT Display (SPI)

**Power Connections:**
```
Display         ESP32-S3
────────────────────────
VCC      →      3.3V (NOT 5V!)
GND      →      GND
```

**SPI Display Pins:**
```
Display         ESP32-S3        Notes
───────────────────────────────────────────
MOSI/SDI →      GPIO 23        (Data to display)
MISO/SDO →      GPIO 19        (Data from display)
SCK/CLK  →      GPIO 18        (SPI clock)
CS       →      GPIO 15        (Chip select)
DC/RS    →      GPIO 2         (Data/Command)
RST      →      GPIO 4         (Reset)
LED/BL   →      GPIO 21        (Backlight PWM)
```

**Touch Screen Pins (XPT2046):**
```
Touch           ESP32-S3        Notes
───────────────────────────────────────────
T_CLK    →      GPIO 25        (Touch clock)
T_MOSI   →      GPIO 32        (Touch data out)
T_MISO   →      GPIO 39        (Touch data in)
T_CS     →      GPIO 33        (Touch chip select)
T_IRQ    →      Not connected  (Optional, not used)
```

**Wiring Tips:**
- Most 2.8" displays have all pins on one side
- Some displays have SD card slot - ignore those pins
- If your display is labeled "SDI" instead of "MOSI", they're the same
- Double-check 3.3V - using 5V will damage the display!

**Testing:** After wiring, upload and monitor serial - should see "Display (ILI9341 TFT)... OK"

---

### Phase 2: I2C Bus 1 (Sensors)

#### BME280 Environmental Sensor

```
BME280          ESP32-S3        Notes
───────────────────────────────────────────
VCC/VIN  →      3.3V
GND      →      GND
SDA      →      GPIO 27        (I2C Bus 1 Data)
SCL      →      GPIO 22        (I2C Bus 1 Clock)
```

**I2C Address:** 0x76 (or 0x77 depending on module)

---

#### MPU6050 IMU (Accelerometer + Gyroscope)

```
MPU6050         ESP32-S3        Notes
───────────────────────────────────────────
VCC      →      3.3V
GND      →      GND
SDA      →      GPIO 27        (I2C Bus 1 Data - shared)
SCL      →      GPIO 22        (I2C Bus 1 Clock - shared)
AD0      →      3.3V           ⚠️ IMPORTANT! Changes address to 0x69
INT      →      Not connected  (Optional)
```

**⚠️ CRITICAL:** Connect AD0 pin to 3.3V! This changes the I2C address from 0x68 to 0x69, avoiding conflict with DS3231 RTC.

**I2C Address:** 0x69 (with AD0 HIGH)

---

#### PCF8574 I/O Expander

```
PCF8574         ESP32-S3        Notes
───────────────────────────────────────────
VCC      →      3.3V
GND      →      GND
SDA      →      GPIO 27        (I2C Bus 1 Data - shared)
SCL      →      GPIO 22        (I2C Bus 1 Clock - shared)
A0       →      GND            (Address pin)
A1       →      GND            (Address pin)
A2       →      GND            (Address pin)
```

**I2C Address:** 0x20 (with all address pins to GND)

**I2C Bus 1 Summary:**
```
All three sensors share the same SDA (GPIO 27) and SCL (GPIO 22):
- BME280: 0x76
- MPU6050: 0x69 (AD0 to 3.3V)
- PCF8574: 0x20
```

---

### Phase 3: I2C Bus 2 (RTC)

#### DS3231 RTC Module

```
DS3231          ESP32-S3        Notes
───────────────────────────────────────────
VCC      →      3.3V or 5V     (Most modules accept either)
GND      →      GND
SDA      →      GPIO 17        (I2C Bus 2 Data - SEPARATE BUS!)
SCL      →      GPIO 16        (I2C Bus 2 Clock - SEPARATE BUS!)
32K      →      Not connected  (Optional square wave output)
SQW      →      Not connected  (Optional)
```

**I2C Address:** 0x68

**Why Separate I2C Bus?** The DS3231 uses address 0x68, which conflicts with MPU6050's default address. Using two I2C buses solves this.

---

### Phase 4: GPS Module (UART)

#### NEO6M GPS Module

```
GPS NEO6M       ESP32-S3        Notes
───────────────────────────────────────────
VCC      →      3.3V or 5V     (Check your module - most accept 3.3-5V)
GND      →      GND
TX       →      GPIO 3 (RX)    (GPS transmits to ESP32 RX)
RX       →      GPIO 1 (TX)    (GPS receives from ESP32 TX)
PPS      →      Not connected  (Optional pulse-per-second)
```

**Baud Rate:** 9600 (configured in software)

**Testing:** GPS needs outdoor location or window view. Red LED on module blinks while searching, solid when locked.

---

### Phase 5: Microphone (I2S)

#### INMP441 MEMS Microphone

```
INMP441         ESP32-S3        Notes
───────────────────────────────────────────
VDD      →      3.3V
GND      →      GND
L/R      →      GND            (Left channel)
WS       →      GPIO 26        (Word Select - I2S)
SD       →      GPIO 14        (Serial Data - I2S)
SCK      →      GPIO 12        (Bit Clock - I2S)
```

**Notes:**
- L/R pin to GND = left channel
- L/R pin to 3.3V = right channel
- We only need mono, so use GND

---

### Phase 6: User Interface (Buttons & Feedback)

#### 3x Tactile Buttons

**⚠️ IMPORTANT:** GPIOs 34, 35, 36 are INPUT ONLY on ESP32 (not ESP32-S3). On ESP32-S3, any GPIO can be used, but we'll keep the same pin numbers for consistency.

**Button Wiring (with internal pull-ups):**
```
Button 1:
  One leg → GPIO 34
  Other leg → GND

Button 2:
  One leg → GPIO 35
  Other leg → GND

Button 3:
  One leg → GPIO 36
  Other leg → GND
```

**Alternative Wiring (if buttons are noisy, add external pull-ups):**
```
Button 1:
  GPIO 34 → 10kΩ resistor → 3.3V
  GPIO 34 → Button → GND

Button 2:
  GPIO 35 → 10kΩ resistor → 3.3V
  GPIO 35 → Button → GND

Button 3:
  GPIO 36 → 10kΩ resistor → 3.3V
  GPIO 36 → Button → GND
```

**Button Functions:**
- Button 1 (GPIO 34): Wake/Sleep, Menu, Back
- Button 2 (GPIO 35): Mode switch, Previous
- Button 3 (GPIO 36): Select, Next

---

#### Vibration Motor (Haptic Feedback)

```
Vibration Motor ESP32-S3        Notes
───────────────────────────────────────────
+        →      GPIO 5         (Control pin)
-        →      GND
```

**⚠️ IMPORTANT:** If motor draws >40mA, add a transistor:
```
GPIO 5 → 1kΩ resistor → NPN transistor base (2N2222)
Transistor emitter → GND
Transistor collector → Motor negative
Motor positive → 3.3V
```

---

#### Passive Buzzer (Audio Feedback)

```
Buzzer          ESP32-S3        Notes
───────────────────────────────────────────
+        →      GPIO 13        (Control pin)
-        →      GND
```

**Note:** If buzzer is 5V, you may need a transistor driver for louder sound. For 3.3V buzzer, direct connection works.

---

### Phase 7: Battery & Power Management

#### TP4056 Charging Module

```
TP4056          Connection      Notes
───────────────────────────────────────────
IN+ / 5V →      USB-C 5V       (Charging input)
IN- / GND →     USB-C GND

OUT+ / B+ →     Battery +      (To LiPo battery)
OUT- / B- →     Battery -

OUT+ / BAT+ →   ESP32 5V       (To power ESP32)
OUT- / BAT- →   ESP32 GND
```

**⚠️ Battery Safety:**
- Only use LiPo batteries with built-in protection circuits
- 1500-2000mAh recommended
- Never short circuit battery terminals!
- Charge in fire-safe location

---

#### Battery Voltage Monitor (Voltage Divider)

To monitor battery level, we need to divide the battery voltage (4.2V max) to fit ESP32 ADC (3.3V max):

```
Battery + ───┬─── 10kΩ resistor ───┬─── GPIO 0 (ADC)
             │                      │
             │                  10kΩ resistor
             │                      │
             └──────────────────────┴─── GND
```

**Voltage Divider Circuit:**
1. Connect battery positive to first 10kΩ resistor
2. Connect other end of first resistor to GPIO 0 AND to second 10kΩ resistor
3. Connect other end of second resistor to GND

**Result:** Battery voltage is divided by 2:
- 4.2V battery → 2.1V at GPIO 0 ✓ Safe!
- 3.3V battery → 1.65V at GPIO 0 ✓ Safe!

---

## 📊 Complete Wiring Summary Table

```
Component       Pin Name    ESP32-S3 GPIO   Bus/Interface   Notes
─────────────────────────────────────────────────────────────────────
POWER
3.3V Rail       -           3V3             Power           Multiple 3.3V pins
Ground          -           GND             Ground          Multiple GND pins

DISPLAY (ILI9341)
MOSI            SDI         GPIO 23         SPI             Data to display
MISO            SDO         GPIO 19         SPI             Data from display
SCK             CLK         GPIO 18         SPI             Clock
CS              CS          GPIO 15         SPI             Chip Select
DC              RS          GPIO 2          GPIO            Data/Command
RST             RESET       GPIO 4          GPIO            Reset
Backlight       LED         GPIO 21         PWM             Brightness control
Power           VCC         3.3V            Power           NOT 5V!
Ground          GND         GND             Ground

TOUCH (XPT2046)
T_CLK           -           GPIO 25         SPI-Touch       Touch clock
T_MOSI          -           GPIO 32         SPI-Touch       Touch data out
T_MISO          -           GPIO 39         SPI-Touch       Touch data in
T_CS            -           GPIO 33         SPI-Touch       Touch chip select

I2C BUS 1 (Sensors)
SDA             -           GPIO 27         I2C             Shared by 3 devices
SCL             -           GPIO 22         I2C             Shared by 3 devices
  BME280        -           0x76            I2C Address
  MPU6050       AD0=HIGH    0x69            I2C Address     AD0 to 3.3V!
  PCF8574       -           0x20            I2C Address

I2C BUS 2 (RTC)
SDA             -           GPIO 17         I2C             Separate bus
SCL             -           GPIO 16         I2C             Separate bus
  DS3231        -           0x68            I2C Address

GPS (UART)
TX              -           GPIO 3 (RX)     UART            GPS → ESP32
RX              -           GPIO 1 (TX)     UART            ESP32 → GPS
Power           VCC         3.3V            Power

MICROPHONE (I2S)
WS              -           GPIO 26         I2S             Word Select
SD              -           GPIO 14         I2S             Serial Data
SCK             -           GPIO 12         I2S             Bit Clock
L/R             -           GND             Config          Left channel

BUTTONS
Button 1        -           GPIO 34         Input           Pull-up enabled
Button 2        -           GPIO 35         Input           Pull-up enabled
Button 3        -           GPIO 36         Input           Pull-up enabled

PERIPHERALS
Vibration       -           GPIO 5          Output          Haptic motor
Buzzer          -           GPIO 13         Output          Audio feedback

BATTERY
ADC Monitor     -           GPIO 0          ADC             Voltage divider
```

---

## 🔌 Breadboard Layout Suggestion

```
Power Rails:
  Top    (+) = 3.3V from ESP32
  Top    (-) = GND from ESP32
  Bottom (+) = Not used (or 5V if needed)
  Bottom (-) = GND (connect to top GND)

Section 1: ESP32-S3 (center of breadboard)
Section 2: Display (right side - short wires)
Section 3: I2C sensors (left side - grouped together)
Section 4: GPS, Microphone (bottom left)
Section 5: Buttons, Motor, Buzzer (bottom right)
```

---

## ✅ Testing Procedure

### Test 1: Power On (Nothing Connected)
1. Connect ESP32-S3 via USB
2. Open Serial Monitor (115200 baud)
3. Should see boot sequence (many "FAILED" messages - normal!)

### Test 2: Display Only
1. Power off, wire display + touch
2. Power on
3. Serial should show: "Display (ILI9341 TFT)... OK"
4. Display should show boot screen!

### Test 3: Add I2C Bus 1
1. Power off, wire BME280, MPU6050, PCF8574
2. Power on
3. Serial should show: "Sensor Manager... OK"
4. Display shows sensor data

### Test 4: Add RTC (I2C Bus 2)
1. Power off, wire DS3231
2. Power on
3. Serial shows RTC detected
4. Clock screen shows correct time (needs setting first)

### Test 5: Add GPS
1. Power off, wire GPS
2. Power on
3. Take device near window or outdoors
4. Wait 2-5 minutes for satellite fix
5. GPS screen shows coordinates

### Test 6: Add Buttons & Peripherals
1. Power off, wire buttons, vibration motor, buzzer
2. Power on
3. Press buttons - should navigate screens
4. BLE connect/disconnect - should vibrate

### Test 7: Battery (LAST!)
1. Wire voltage divider carefully
2. Connect battery to TP4056
3. Power on
4. Status bar shows battery percentage

---

## 🛠️ Troubleshooting

### Display stays black:
- Check backlight (GPIO 21) - most common issue
- Verify 3.3V power (measure with multimeter)
- Check all SPI connections
- Try different display orientation in code

### Touch not responding:
- Check T_CS connection (GPIO 33)
- Run touch calibration routine
- Check touch pressure threshold

### I2C sensor not found:
- Run I2C scanner sketch
- Check SDA/SCL connections
- Verify correct I2C bus (Bus 1 vs Bus 2)
- MPU6050: Ensure AD0 is connected to 3.3V!

### GPS no satellites:
- Must be near window or outdoors
- Red LED should blink (searching)
- Wait 5+ minutes for first fix
- Won't work in basements or interior rooms

### Buttons not working:
- Check GPIO 34, 35, 36 connections
- Try external 10kΩ pull-up resistors if noisy
- Verify button is normally-open (NO) type

---

## 📸 Next Steps

After wiring:
1. Take a photo of your setup (for debugging if needed)
2. Upload firmware if not already done
3. Open serial monitor
4. Watch boot sequence
5. Report what works and what doesn't!

---

## 🆘 Need Help?

Post in discussions with:
- Photo of your wiring
- Serial monitor output
- Which components work/don't work

Good luck! 🚀
