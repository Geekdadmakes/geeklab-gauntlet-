# Hardware Assembly Guide

This guide walks you through assembling the Wearable Tech Pad hardware.

## Required Components

### Core Components
- [ ] ESP32-S3 DevKit ($15-20)
- [ ] 1.3" OLED Display 128x64 I2C ($8-15)
- [ ] INMP441 MEMS Microphone ($3-5)
- [ ] MAX98357A I2S Amplifier + Speaker ($8-12)
- [ ] 500-1000mAh LiPo Battery ($10-15)
- [ ] TP4056 Charging Module ($2-3)
- [ ] Vibration Motor ($2)
- [ ] Tactile Buttons x3 ($3)

### Supporting Components
- [ ] Jumper wires (male-to-female and male-to-male)
- [ ] Breadboard for prototyping
- [ ] USB-C cable for programming
- [ ] Soldering iron and solder
- [ ] Heat shrink tubing
- [ ] Double-sided tape or hot glue

### Tools
- [ ] Soldering iron (recommended: adjustable temperature)
- [ ] Wire cutters/strippers
- [ ] Multimeter
- [ ] Small screwdrivers
- [ ] Helping hands/PCB holder

## Pin Connections

### ESP32-S3 Pinout

```
ESP32-S3 Pin    →   Component
─────────────────────────────────
GPIO 21 (SDA)   →   OLED SDA
GPIO 22 (SCL)   →   OLED SCL
3.3V            →   OLED VCC
GND             →   OLED GND

GPIO 25 (WS)    →   INMP441 WS (Word Select)
GPIO 33 (SD)    →   INMP441 SD (Serial Data)
GPIO 32 (SCK)   →   INMP441 SCK (Bit Clock)
3.3V            →   INMP441 VDD
GND             →   INMP441 GND

GPIO 26 (DOUT)  →   MAX98357A DIN
GPIO 32 (SCK)   →   MAX98357A BCLK
GPIO 25 (WS)    →   MAX98357A LRC
5V              →   MAX98357A VIN
GND             →   MAX98357A GND

GPIO 19         →   Button 1
GPIO 18         →   Button 2
GPIO 5          →   Button 3
GND             →   All button grounds

GPIO 4          →   Vibration Motor (+)
GND             →   Vibration Motor (-)

GPIO 35         →   Battery Voltage Monitor (via voltage divider)
```

## Assembly Steps

### Phase 1: Breadboard Prototype

#### 1. Test ESP32-S3

1. Connect ESP32-S3 to computer via USB-C
2. Open PlatformIO and upload a simple blink sketch
3. Verify the board is working

```cpp
void setup() {
    pinMode(2, OUTPUT);
}

void loop() {
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);
}
```

#### 2. Connect OLED Display

1. Connect OLED to ESP32-S3:
   - VCC → 3.3V
   - GND → GND
   - SCL → GPIO 22
   - SDA → GPIO 21

2. Test with display code:
   - Upload firmware/src/main.cpp
   - Should see boot screen

3. Verify I2C address (usually 0x3C):
   ```cpp
   Wire.beginTransmission(0x3C);
   ```

#### 3. Add Buttons

1. Connect each button between GPIO and GND
2. Enable internal pull-up resistors in code
3. Test button presses with Serial output

#### 4. Add Vibration Motor

1. Connect vibration motor:
   - Positive → GPIO 4 (through transistor for higher current)
   - Negative → GND

2. Add transistor if motor draws >20mA:
   - Use 2N2222 or similar NPN transistor
   - Base → GPIO 4 (through 1kΩ resistor)
   - Collector → Motor positive
   - Emitter → GND

#### 5. Add Microphone

1. Connect INMP441:
   - VDD → 3.3V
   - GND → GND
   - WS → GPIO 25
   - SD → GPIO 33
   - SCK → GPIO 32
   - L/R → GND (left channel)

2. Test audio input with Serial plotter

#### 6. Add Speaker Amplifier

1. Connect MAX98357A:
   - VIN → 5V (USB power)
   - GND → GND
   - DIN → GPIO 26
   - BCLK → GPIO 32
   - LRC → GPIO 25

2. Connect speaker to MAX98357A output terminals

3. Test audio output

### Phase 2: Power System

#### 1. Battery Voltage Monitor

1. Create voltage divider:
   - Battery + → 10kΩ resistor → GPIO 35
   - GPIO 35 → 10kΩ resistor → GND

2. This divides battery voltage (0-4.2V) by 2 for ADC

#### 2. Charging Circuit

1. Connect TP4056:
   - IN+ → USB-C VBUS
   - IN- → USB-C GND
   - OUT+ → Battery +
   - OUT- → Battery -

2. Add protection:
   - Use TP4056 module with built-in protection
   - Or add separate battery protection circuit

#### 3. Power Distribution

1. Battery powers ESP32-S3 via 3.3V regulator
2. Use MT3608 boost converter if needed:
   - Input: Battery (3.3-4.2V)
   - Output: 5V for MAX98357A
   - Adjust output with potentiometer

### Phase 3: Testing

#### Checklist

- [ ] ESP32-S3 boots and runs code
- [ ] OLED displays boot screen and UI
- [ ] All 3 buttons register presses
- [ ] Vibration motor activates
- [ ] Microphone captures audio
- [ ] Speaker plays audio
- [ ] Battery charges via USB-C
- [ ] Battery level reads correctly
- [ ] BLE connects to iPhone

#### Power Consumption Test

1. Measure current draw in different states:
   - Deep sleep: <50μA
   - Idle (display on): 50-100mA
   - Active (all features): 150-250mA
   - Voice listening: 200-300mA

2. Calculate battery life:
   - 1000mAh battery ÷ 100mA average = 10 hours
   - Target: 24+ hours with sleep modes

### Phase 4: Permanent Assembly

#### 1. Solder Connections

1. Plan wire routing for compact assembly
2. Use appropriate wire gauge:
   - Power: 22-24 AWG
   - Signals: 26-28 AWG

3. Solder components:
   - Start with headers on ESP32-S3
   - Solder display wires
   - Add buttons with wire leads
   - Connect all power and ground

4. Add heat shrink to all solder joints

#### 2. Component Mounting

1. Use double-sided foam tape for:
   - ESP32-S3
   - Display
   - Battery
   - TP4056 module

2. Use hot glue for:
   - Buttons
   - Vibration motor
   - Microphone

3. Keep components organized and accessible

## Troubleshooting

### Display Not Working
- Check I2C address (scan with I2C scanner sketch)
- Verify SDA/SCL connections
- Ensure 3.3V power supply is stable
- Try different I2C pull-up resistors

### Buttons Not Responding
- Verify internal pull-ups are enabled
- Check for short circuits
- Test with multimeter continuity mode

### Microphone No Audio
- Verify I2S configuration
- Check L/R pin (GND = left, VDD = right)
- Test with I2S example code
- Ensure SCK and WS are correct

### Battery Not Charging
- Check TP4056 LED indicators
- Verify USB-C power (5V)
- Test battery voltage with multimeter
- Ensure correct polarity

### BLE Not Connecting
- Verify correct UUIDs in code
- Check iPhone Bluetooth settings
- Restart both devices
- Check for BLE initialization errors in Serial

## Safety Notes

⚠️ **Important Safety Guidelines:**

1. **Battery Safety:**
   - Never short circuit LiPo batteries
   - Use batteries with protection circuits
   - Don't charge above 4.2V
   - Don't discharge below 3.0V
   - Store at 3.7V if not using

2. **Soldering Safety:**
   - Use proper ventilation
   - Don't touch hot components
   - Use ESD protection for sensitive components

3. **Power Safety:**
   - Double-check polarity before powering on
   - Use fuses for battery connections
   - Monitor temperature during operation

## Next Steps

Once hardware is assembled and tested:

1. Upload full firmware (firmware/src/main.cpp)
2. Install iOS companion app
3. Pair with iPhone via BLE
4. Test all features end-to-end
5. Design and print enclosure (see enclosure/README.md)

## Resources

- ESP32-S3 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
- SSD1306 OLED Datasheet: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
- INMP441 Datasheet: https://invensense.tdk.com/products/digital/inmp441/
- MAX98357A Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/MAX98357A-MAX98357B.pdf
