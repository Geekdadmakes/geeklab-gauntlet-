# BLE Time Sync Protocol

## Overview

The Gauntlet wearable device supports automatic time synchronization via Bluetooth Low Energy (BLE) from a connected phone. This document describes the JSON protocol used for time sync commands.

## BLE Service Information

**Service UUID:** `4fafc201-1fb5-459e-8fcc-c5c9c331914b`

**Command Characteristic UUID:** `c6b2f38c-23ab-46d8-a6ab-a3a870bbd5b6`
- Properties: Read, Write, Notify
- Purpose: Bidirectional command/response channel

## Command: SET_TIME (Phone → Device)

### Request Format

```json
{
  "cmd": "SET_TIME",
  "timestamp": 1738279200,
  "timezone": "America/New_York",
  "tzOffset": -18000
}
```

### Request Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `cmd` | string | ✅ Yes | Must be "SET_TIME" |
| `timestamp` | integer | ✅ Yes | Unix timestamp (seconds since Jan 1, 1970 UTC) |
| `timezone` | string | ⚠️ Optional | IANA timezone name (e.g., "America/New_York") |
| `tzOffset` | integer | ⚠️ Optional | Timezone offset in seconds (e.g., -18000 for EST) |

### Response Format (Device → Phone)

**Success:**
```json
{
  "cmd": "TIME_SYNC_ACK",
  "success": true,
  "timestamp": 1738279200
}
```

**Failure:**
```json
{
  "cmd": "TIME_SYNC_ACK",
  "success": false
}
```

### Response Fields

| Field | Type | Description |
|-------|------|-------------|
| `cmd` | string | Always "TIME_SYNC_ACK" |
| `success` | boolean | `true` if sync succeeded, `false` if failed |
| `timestamp` | integer | Echoes the timestamp that was set (only if success=true) |

### Example Interaction

**1. Phone sends SET_TIME:**
```json
{"cmd":"SET_TIME","timestamp":1738279200,"timezone":"America/New_York","tzOffset":-18000}
```

**2. Device processes and responds:**
```json
{"cmd":"TIME_SYNC_ACK","success":true,"timestamp":1738279200}
```

**3. Device provides haptic feedback:**
- Vibration motor activates for 100ms to confirm sync

## Command: REQUEST_TIME (Device → Phone)

### Request Format

```json
{
  "cmd": "REQUEST_TIME"
}
```

### When Triggered

The device automatically sends REQUEST_TIME when:
1. RTC lost power (battery removed)
2. RTC time is invalid (year < 2025)
3. BLE connection established and time sync needed
4. User manually requests sync (future feature)

### Expected Phone Response

Phone should respond with SET_TIME command containing current time:
```json
{"cmd":"SET_TIME","timestamp":1738279200,"timezone":"America/New_York","tzOffset":-18000}
```

## Implementation Examples

### JavaScript (Web Bluetooth API)

```javascript
// Connect to device
const device = await navigator.bluetooth.requestDevice({
  filters: [{ name: 'Gauntlet-TechPad' }],
  optionalServices: ['4fafc201-1fb5-459e-8fcc-c5c9c331914b']
});

const server = await device.gatt.connect();
const service = await server.getPrimaryService('4fafc201-1fb5-459e-8fcc-c5c9c331914b');
const commandChar = await service.getCharacteristic('c6b2f38c-23ab-46d8-a6ab-a3a870bbd5b6');

// Send time sync command
async function syncTime() {
  const now = Math.floor(Date.now() / 1000); // Unix timestamp
  const timezone = Intl.DateTimeFormat().resolvedOptions().timeZone; // e.g., "America/New_York"
  const tzOffset = -new Date().getTimezoneOffset() * 60; // Convert minutes to seconds

  const command = {
    cmd: "SET_TIME",
    timestamp: now,
    timezone: timezone,
    tzOffset: tzOffset
  };

  const encoder = new TextEncoder();
  await commandChar.writeValue(encoder.encode(JSON.stringify(command)));
  console.log('Time sync command sent:', command);
}

// Listen for REQUEST_TIME from device
await commandChar.startNotifications();
commandChar.addEventListener('characteristicvaluechanged', async (event) => {
  const decoder = new TextDecoder();
  const message = decoder.decode(event.target.value);
  const data = JSON.parse(message);

  if (data.cmd === 'REQUEST_TIME') {
    console.log('Device requested time sync');
    await syncTime(); // Auto-respond with current time
  } else if (data.cmd === 'TIME_SYNC_ACK') {
    console.log('Time sync acknowledged:', data.success);
  }
});

// Initial sync on connect
await syncTime();
```

### Swift (iOS CoreBluetooth)

```swift
import CoreBluetooth

class TimeSync: NSObject, CBPeripheralDelegate {
    var peripheral: CBPeripheral?
    var commandCharacteristic: CBCharacteristic?

    let serviceUUID = CBUUID(string: "4fafc201-1fb5-459e-8fcc-c5c9c331914b")
    let commandUUID = CBUUID(string: "c6b2f38c-23ab-46d8-a6ab-a3a870bbd5b6")

    func syncTime() {
        guard let char = commandCharacteristic else { return }

        let timestamp = Int(Date().timeIntervalSince1970)
        let timezone = TimeZone.current.identifier // e.g., "America/New_York"
        let tzOffset = TimeZone.current.secondsFromGMT()

        let command: [String: Any] = [
            "cmd": "SET_TIME",
            "timestamp": timestamp,
            "timezone": timezone,
            "tzOffset": tzOffset
        ]

        if let jsonData = try? JSONSerialization.data(withJSONObject: command),
           let jsonString = String(data: jsonData, encoding: .utf8) {
            let data = jsonString.data(using: .utf8)!
            peripheral?.writeValue(data, for: char, type: .withResponse)
            print("Time sync command sent: \(jsonString)")
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        guard let data = characteristic.value,
              let message = String(data: data, encoding: .utf8),
              let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
              let cmd = json["cmd"] as? String else { return }

        if cmd == "REQUEST_TIME" {
            print("Device requested time sync")
            syncTime() // Auto-respond
        } else if cmd == "TIME_SYNC_ACK" {
            let success = json["success"] as? Bool ?? false
            print("Time sync acknowledged: \(success)")
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }

        for char in characteristics where char.uuid == commandUUID {
            commandCharacteristic = char
            peripheral.setNotifyValue(true, for: char)
            syncTime() // Initial sync on connect
        }
    }
}
```

### Python (using bleak library)

```python
import asyncio
import json
import time
from datetime import datetime, timezone
from bleak import BleakClient

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
COMMAND_UUID = "c6b2f38c-23ab-46d8-a6ab-a3a870bbd5b6"

async def sync_time(client):
    """Send time sync command to device"""
    timestamp = int(time.time())
    tz_name = datetime.now(timezone.utc).astimezone().tzname()
    tz_offset = -time.timezone  # Seconds from UTC

    command = {
        "cmd": "SET_TIME",
        "timestamp": timestamp,
        "timezone": tz_name,
        "tzOffset": tz_offset
    }

    json_str = json.dumps(command)
    await client.write_gatt_char(COMMAND_UUID, json_str.encode())
    print(f"Time sync command sent: {json_str}")

async def notification_handler(sender, data):
    """Handle notifications from device"""
    message = data.decode('utf-8')
    response = json.loads(message)

    if response.get('cmd') == 'REQUEST_TIME':
        print("Device requested time sync")
        # Auto-respond (would need client reference)

    elif response.get('cmd') == 'TIME_SYNC_ACK':
        success = response.get('success', False)
        print(f"Time sync acknowledged: {success}")

async def main():
    device_address = "YOUR_DEVICE_MAC_ADDRESS"  # e.g., "A4:C1:38:12:34:56"

    async with BleakClient(device_address) as client:
        print(f"Connected to {client.address}")

        # Enable notifications
        await client.start_notify(COMMAND_UUID, notification_handler)

        # Initial time sync
        await sync_time(client)

        # Keep connection alive
        await asyncio.sleep(60)

asyncio.run(main())
```

## Testing with nRF Connect

### Step-by-Step Instructions

1. **Install nRF Connect app** (iOS/Android)
2. **Scan for devices** and select "Gauntlet-TechPad"
3. **Connect** to the device
4. **Find the service** with UUID `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
5. **Tap the Command characteristic** (`c6b2f38c-23ab-46d8-a6ab-a3a870bbd5b6`)
6. **Enable notifications** (tap the 3-arrow icon)
7. **Write value** using the following steps:

   - **Select "Text" format**
   - **Enter this JSON** (replace timestamp with current time from https://www.unixtimestamp.com/):
     ```json
     {"cmd":"SET_TIME","timestamp":1738279200,"timezone":"America/New_York","tzOffset":-18000}
     ```
   - **Send**

8. **Watch for response** in the notification log:
   ```json
   {"cmd":"TIME_SYNC_ACK","success":true,"timestamp":1738279200}
   ```

9. **Check device serial monitor** for confirmation:
   ```
   Parsing time sync command...
   ✓ Time synced from BLE
   RTC synced to: 2026-01-30 14:30:00
   ```

## Timezone Reference

### Common Timezone Offsets

| Timezone | IANA Name | Offset (seconds) | Offset (hours) |
|----------|-----------|------------------|----------------|
| EST (Eastern Standard Time) | America/New_York | -18000 | UTC-5 |
| CST (Central Standard Time) | America/Chicago | -21600 | UTC-6 |
| MST (Mountain Standard Time) | America/Denver | -25200 | UTC-7 |
| PST (Pacific Standard Time) | America/Los_Angeles | -28800 | UTC-8 |
| GMT (Greenwich Mean Time) | Europe/London | 0 | UTC+0 |
| CET (Central European Time) | Europe/Paris | 3600 | UTC+1 |
| JST (Japan Standard Time) | Asia/Tokyo | 32400 | UTC+9 |

### Calculating Timezone Offset

**Formula:**
```
tzOffset (seconds) = (UTC hour - local hour) * 3600
```

**Example:**
- Local time: 2:00 PM (14:00)
- UTC time: 7:00 PM (19:00)
- Offset: (19 - 14) * 3600 = 18000 seconds = UTC-5 (EST)

**Note:** Offset is negative for time zones west of UTC, positive for east.

## Error Handling

### Invalid Timestamp (Year Out of Range)

**Input:**
```json
{"cmd":"SET_TIME","timestamp":9999999999}
```

**Device Response:**
```json
{"cmd":"TIME_SYNC_ACK","success":false}
```

**Serial Output:**
```
ERROR: Invalid timestamp (year out of range)
Time sync rejected
```

**Valid Range:** 2025-01-01 to 2050-01-01
- Min: 1735689600 (2025-01-01 00:00:00 UTC)
- Max: 2524608000 (2050-01-01 00:00:00 UTC)

### Missing Required Fields

**Input:**
```json
{"cmd":"SET_TIME"}
```

**Device Response:**
```json
{"cmd":"TIME_SYNC_ACK","success":false}
```

**Serial Output:**
```
ERROR: No timestamp field found
```

### BLE Not Connected

If device attempts `requestTimeSync()` when BLE is disconnected:

**Serial Output:**
```
Cannot request time sync - BLE not connected
```

## Auto-Sync Schedule

The device automatically triggers time sync under these conditions:

1. **On boot:** If RTC lost power or time invalid
2. **Daily at 3:00-3:05 AM:** Daily accuracy maintenance
3. **Every 24 hours:** If daily window missed
4. **After failed sync:** Retry every 6 hours

## Integration Checklist

### Phone App Requirements

- [ ] Implement BLE Central role (connect to peripheral)
- [ ] Connect to service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- [ ] Subscribe to Command characteristic notifications
- [ ] Send SET_TIME on connection established
- [ ] Auto-respond to REQUEST_TIME with current time
- [ ] Handle TIME_SYNC_ACK responses
- [ ] Get timezone from system settings
- [ ] Calculate timezone offset correctly (including DST)
- [ ] Use Unix timestamp (not milliseconds)

### Device Firmware (✅ Already Implemented)

- [x] Parse SET_TIME JSON command
- [x] Validate timestamp range (2025-2050)
- [x] Apply timezone offset to RTC
- [x] Send TIME_SYNC_ACK response
- [x] Send REQUEST_TIME when needed
- [x] Provide haptic feedback on success
- [x] Store timezone to NVS (persistent)
- [x] Daily auto-sync at 3 AM

## Security Considerations

1. **No authentication:** BLE connection is unauthenticated. Any connected device can set time.
2. **Future enhancement:** Add BLE pairing requirement
3. **Validation:** Device validates timestamp range to prevent invalid dates
4. **Timezone trust:** Device trusts phone's timezone without verification

## Troubleshooting

### Time Sync Not Working

**Check:**
1. BLE connected (status bar shows cyan BLE icon)
2. JSON format is valid (use online JSON validator)
3. Timestamp is in seconds, not milliseconds
4. Characteristic UUID is correct: `c6b2f38c-23ab-46d8-a6ab-a3a870bbd5b6`
5. Notifications are enabled on Command characteristic

### Time Incorrect After Sync

**Possible causes:**
1. Timezone offset wrong sign (negative vs positive)
2. DST not accounted for
3. Timestamp in milliseconds instead of seconds (divide by 1000)

### No Response from Device

**Check:**
1. Command characteristic has Write property enabled
2. JSON is valid (no syntax errors)
3. "cmd" field is exactly "SET_TIME" (case-sensitive)
4. Device serial monitor for error messages

---

**Protocol Version:** 1.0
**Last Updated:** January 30, 2026
**Device Firmware:** v2.0+
