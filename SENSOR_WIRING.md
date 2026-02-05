# SENSOR WIRING DIAGRAM
## ESP32-S3 N16R8 Wearable Computer

---

## Power Distribution

**CRITICAL:** All sensors use **3.3V**, NOT 5V!

```
ESP32-S3 3.3V Pin  ──┬── BME280 VCC
                     ├── MPU6050 VCC
                     ├── DS3231 VCC
                     ├── PCF8574 VCC
                     └── GPS NEO6M VCC

ESP32-S3 GND Pin   ──┬── BME280 GND
                     ├── MPU6050 GND
                     ├── DS3231 GND
                     ├── PCF8574 GND
                     └── GPS NEO6M GND
```

---

## I2C Bus 1 (Primary Sensors)
**GPIO 6 = SDA, GPIO 7 = SCL**

### Connections:
```
ESP32-S3 GPIO 6 (SDA) ──┬── BME280 SDA
                        ├── MPU6050 SDA
                        └── PCF8574 SDA

ESP32-S3 GPIO 7 (SCL) ──┬── BME280 SCL
                        ├── MPU6050 SCL
                        └── PCF8574 SCL
```

### Individual Sensor Details:

**BME280 Environmental Sensor (Temperature, Humidity, Pressure)**
```
┌─────────────┐
│   BME280    │
├─────────────┤
│ VCC → 3.3V  │
│ GND → GND   │
│ SDA → GPIO 6│
│ SCL → GPIO 7│
│ SDO → GND   │ (Sets I2C address to 0x76)
│ CSB → 3.3V  │ (Enables I2C mode)
└─────────────┘
I2C Address: 0x76
```

**MPU6050 IMU (Accelerometer, Gyroscope)**
```
┌─────────────┐
│   MPU6050   │
├─────────────┤
│ VCC → 3.3V  │
│ GND → GND   │
│ SDA → GPIO 6│
│ SCL → GPIO 7│
│ AD0 → 3.3V  │ ⚠️ CRITICAL: Connect to 3.3V (changes address to 0x69)
│ XDA → (NC)  │ (Not connected)
│ XCL → (NC)  │ (Not connected)
│ INT → (NC)  │ (Optional, not used)
└─────────────┘
I2C Address: 0x69 (with AD0=HIGH)

⚠️ WARNING: AD0 MUST be connected to 3.3V, otherwise address conflicts with DS3231!
```

**PCF8574 I/O Expander (8-bit GPIO)**
```
┌─────────────┐
│   PCF8574   │
├─────────────┤
│ VCC → 3.3V  │
│ GND → GND   │
│ SDA → GPIO 6│
│ SCL → GPIO 7│
│ A0  → GND   │ (Address select)
│ A1  → GND   │ (Address select)
│ A2  → GND   │ (Address select)
│ INT → (NC)  │ (Optional, not used)
│ P0-P7 → ... │ (Expansion GPIO pins)
└─────────────┘
I2C Address: 0x20
```

---

## I2C Bus 2 (RTC Only)
**GPIO 17 = SDA, GPIO 16 = SCL**

**DS3231 Real-Time Clock**
```
┌─────────────┐
│   DS3231    │
├─────────────┤
│ VCC → 3.3V  │
│ GND → GND   │
│ SDA → GPIO17│
│ SCL → GPIO16│
│ SQW → (NC)  │ (Not used)
│ 32K → (NC)  │ (Not used)
└─────────────┘
I2C Address: 0x68

⚠️ SEPARATE BUS: Uses GPIO 16/17 to avoid address conflict with MPU6050
```

---

## UART (GPS Module)

**NEO-6M GPS Module**
```
┌─────────────┐
│  NEO-6M GPS │
├─────────────┤
│ VCC → 3.3V  │
│ GND → GND   │
│ TX  → GPIO 3│ (GPS TX → ESP32 RX)
│ RX  → GPIO 1│ (GPS RX → ESP32 TX)
│ PPS → (NC)  │ (Not used)
└─────────────┘
Baud Rate: 9600
Serial Port: Serial2

📍 GPS requires outdoor placement or window view for satellite lock
⏱️  Allow 1-5 minutes for initial fix (cold start)
```

---

## I2C Address Summary

| Device    | Bus | Address | GPIO        | Notes                          |
|-----------|-----|---------|-------------|--------------------------------|
| BME280    | 1   | 0x76    | SDA=6,SCL=7 | SDO to GND                     |
| MPU6050   | 1   | 0x69    | SDA=6,SCL=7 | AD0 to 3.3V (REQUIRED!)        |
| PCF8574   | 1   | 0x20    | SDA=6,SCL=7 | A0/A1/A2 to GND                |
| DS3231    | 2   | 0x68    | SDA=17,SCL=16 | Separate bus (conflict avoidance) |

---

## Pull-up Resistors

**I2C buses require pull-up resistors (typically 4.7kΩ to 10kΩ):**

```
I2C Bus 1:
  3.3V ──[4.7kΩ]── GPIO 6 (SDA)
  3.3V ──[4.7kΩ]── GPIO 7 (SCL)

I2C Bus 2:
  3.3V ──[4.7kΩ]── GPIO 17 (SDA)
  3.3V ──[4.7kΩ]── GPIO 16 (SCL)
```

**NOTE:** Most breakout boards have pull-ups built-in. Check your modules before adding external resistors.

---

## Breadboard Layout Example

```
                    ESP32-S3
                  ┌──────────┐
                  │          │
       3.3V ──────┤ 3.3V     │
       GND  ──────┤ GND      │
                  │          │
  I2C Bus 1:      │          │
       SDA ───────┤ GPIO 6   │ ─┬─ BME280 SDA
                  │          │  ├─ MPU6050 SDA
       SCL ───────┤ GPIO 7   │ ─┼─ PCF8574 SDA
                  │          │  │
  I2C Bus 2:      │          │  │
       SDA ───────┤ GPIO 17  │ ─┤ (DS3231 only)
       SCL ───────┤ GPIO 16  │ ─┤
                  │          │  │
  GPS UART:       │          │  │
       GPS TX ────┤ GPIO 3   │  │
       GPS RX ────┤ GPIO 1   │  │
                  │          │  │
  Special:        │          │  │
       MPU AD0 ───┤ 3.3V     │  │ ⚠️ CRITICAL CONNECTION
                  └──────────┘  │
                                │
                    Power Bus   │
                  ┌──────────┐  │
       3.3V ──────┤ + Rail  ─┼──┴─ All VCC pins
       GND  ──────┤ - Rail  ─┴──── All GND pins
                  └──────────┘
```

---

## Troubleshooting

### I2C Scanner
If sensors are not detected, run I2C scanner in main.cpp (uncomment scanner code).

**Expected Output:**
```
I2C Bus 1 (GPIO 6/7):
  - 0x20: PCF8574
  - 0x69: MPU6050
  - 0x76: BME280

I2C Bus 2 (GPIO 16/17):
  - 0x68: DS3231
```

### Common Issues

**MPU6050 shows as 0x68 instead of 0x69:**
- AD0 pin is not connected to 3.3V
- This causes address conflict with DS3231
- **Solution:** Connect MPU6050 AD0 to 3.3V

**BME280 not detected:**
- Check SDO pin is connected to GND (sets address to 0x76)
- Some modules have 0x77 as default - check datasheet

**GPS no data:**
- Must be outdoors or near window
- Allow 1-5 minutes for satellite lock
- Check TX/RX are not swapped

**No devices detected:**
- Check pull-up resistors (4.7kΩ on SDA/SCL)
- Verify all GND connections are common
- Check 3.3V power is stable

---

## Testing Sequence

1. **Power Check:** Measure 3.3V at each sensor VCC pin
2. **I2C Scan:** Run I2C scanner to detect all devices
3. **BME280:** Check temperature reads room temperature (~20-25°C)
4. **MPU6050:** Move device, check accelerometer values change
5. **DS3231:** Set time, verify it keeps running
6. **GPS:** Take outdoors, wait for fix (LED stops blinking)
7. **PCF8574:** Toggle output pins, verify with multimeter

---

## Current Implementation Status

✅ **BME280** - Fully integrated, displaying live data on Clock and Sensor screens
🔄 **MPU6050** - Code ready, needs testing with real hardware
🔄 **DS3231** - Code ready, needs time setting
🔄 **GPS** - Code ready, needs outdoor testing
🔄 **PCF8574** - Code ready, GPIO expansion available

---

## Pin Summary Table

| Function      | ESP32 Pin | Device        | Device Pin    | Notes                    |
|---------------|-----------|---------------|---------------|--------------------------|
| I2C1 SDA      | GPIO 6    | BME280        | SDA           | Bus 1 (3 devices)        |
|               |           | MPU6050       | SDA           |                          |
|               |           | PCF8574       | SDA           |                          |
| I2C1 SCL      | GPIO 7    | BME280        | SCL           | Bus 1 (3 devices)        |
|               |           | MPU6050       | SCL           |                          |
|               |           | PCF8574       | SCL           |                          |
| I2C2 SDA      | GPIO 17   | DS3231        | SDA           | Bus 2 (1 device)         |
| I2C2 SCL      | GPIO 16   | DS3231        | SCL           | Bus 2 (1 device)         |
| UART RX       | GPIO 3    | GPS NEO6M     | TX            | 9600 baud                |
| UART TX       | GPIO 1    | GPS NEO6M     | RX            | 9600 baud                |
| Power         | 3.3V      | All sensors   | VCC           | ~200mA total             |
| Ground        | GND       | All sensors   | GND           | Common ground            |
| MPU AD0       | 3.3V      | MPU6050       | AD0           | ⚠️ Sets address to 0x69  |
| BME SDO       | GND       | BME280        | SDO           | Sets address to 0x76     |

---

**Last Updated:** 2026-01-29
**Firmware Version:** v2.0 - Sensor Integration Phase
