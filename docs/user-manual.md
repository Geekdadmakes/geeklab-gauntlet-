# Wearable Tech Pad - User Manual

Welcome to your Pip-Boy style Wearable Tech Pad! This manual will help you get started and make the most of your device.

## Quick Start Guide

### First Time Setup

1. **Charge the Device**
   - Connect USB-C cable to charging port
   - Red LED indicates charging
   - Green LED indicates fully charged
   - First charge: 2-3 hours

2. **Power On**
   - Press and hold Button 1 for 2 seconds
   - Boot screen will appear
   - Device initializes (takes 5-10 seconds)
   - Clock screen appears when ready

3. **Pair with iPhone**
   - Open Gauntlet Companion app on iPhone
   - Tap "Scan for Gauntlet"
   - Select your device from list
   - Connection confirmed with vibration
   - BLE icon appears on status bar

## Device Overview

### Display Layout

```
┌─────────────────────────┐
│ 75%  BLE  WiFi         │ ← Status Bar
├─────────────────────────┤
│                         │
│     Main Content        │ ← Content Area
│                         │
│                         │
└─────────────────────────┘
```

**Status Bar Shows:**
- Battery percentage
- BLE connection status
- WiFi connection status (if enabled)

### Button Layout

```
         [Wrist Strap]
              │
    ┌─────────────────┐
    │                 │
[1] │     Display     │ [3]
    │                 │
    └─────────────────┘
              [2]
```

**Button Functions:**

**Button 1 (Left):**
- Click: Open menu / Select
- Double-click: Return to clock
- Long press: Activate voice

**Button 2 (Bottom):**
- Click: Previous / Up
- Long press: Dismiss notification

**Button 3 (Right):**
- Click: Next / Down

## Main Features

### 1. Clock Mode

**Default screen showing:**
- Current time (large)
- Current date
- Status bar

**Actions:**
- Click Button 1: Open main menu
- Long press Button 1: Activate voice
- Double-click Button 1: (already on clock)

### 2. Notifications

**When notification arrives:**
- Vibration alert
- Screen shows notification
- Title and message displayed
- Auto-dismiss after 5 seconds or manual action

**Navigate notifications:**
- Button 2: Previous notification
- Button 3: Next notification
- Long press Button 2: Dismiss current
- Button 1: Open action menu

**Reply to messages:**
1. View notification
2. Long press Button 1
3. Speak your reply
4. Or select quick reply template

**Quick Replies:**
- "OK"
- "On my way"
- "Call you later"
- "Busy now"
- "Thanks"

### 3. Voice Commands

**Activate voice:**
- Say wake word: "Gauntlet"
- Or long press Button 1

**Available commands:**

**Device Control:**
- "What time is it?"
- "Check battery"
- "Show notifications"
- "Check messages"

**Smart Home:**
- "Turn on [device name]"
- "Turn off [device name]"
- "Set [device] to [value]"

**General Queries:**
- "What's the weather?"
- Ask any question (sent to cloud AI)

**Voice Feedback:**
- Listening indicator appears
- Speak clearly within 5 seconds
- Response displayed on screen
- Or relayed through iPhone speaker

### 4. Smart Home Control

**Prerequisites:**
- WiFi enabled and connected
- MQTT broker configured
- Devices added to Home Assistant

**Control devices:**
1. Button 1 → Menu
2. Select "Smart Home"
3. Browse devices (Button 2/3)
4. Button 1 to toggle/control

**Voice control:**
- "Turn on living room lights"
- "Set thermostat to 72"
- "Turn off bedroom fan"

### 5. Menu System

**Main Menu Items:**
- Notifications
- Voice AI
- Smart Home
- Back to Clock

**Navigate:**
- Button 2: Up
- Button 3: Down
- Button 1: Select

## Battery Management

### Battery Life

**Expected runtime:**
- Light use: 36-48 hours
- Moderate use: 24-36 hours
- Heavy use: 12-24 hours

**Usage modes:**
- Clock only: 48+ hours
- With notifications: 24-36 hours
- Voice active: 12-18 hours
- Continuous use: 8-12 hours

### Charging

**Charging process:**
1. Connect USB-C cable
2. Can use while charging
3. Full charge: 1-2 hours
4. Red LED: Charging
5. Green LED: Complete

**Tips for longer battery:**
- Reduce screen brightness
- Disable WiFi if not needed
- Minimize voice usage
- Let device sleep when idle

### Low Battery

**Indicators:**
- Battery icon shows <20%
- Screen dims slightly
- Non-essential features disabled
- Warning vibration at 10%

**Power saving mode:**
- Activates at 15%
- Reduces screen refresh rate
- Disables voice AI
- Extends runtime by 50%

## Wearing the Device

### Proper Fit

1. Place on left or right wrist
2. Adjust strap for snug fit
3. Should not slide around
4. Not too tight (allows air flow)
5. Display should face you

### Best Position

- **For microphone:** Speak across wrist, not directly at it
- **For display:** Angle for easy viewing
- **For buttons:** Easy thumb access

### Comfort Tips

- Remove during sleep
- Clean wrist strap weekly
- Take breaks on long wear days
- Rotate wrists if wearing >12 hours

## Maintenance

### Cleaning

**Display:**
- Use microfiber cloth
- Gentle pressure only
- No harsh chemicals
- Avoid water

**Enclosure:**
- Wipe with damp cloth
- Mild soap if needed
- Dry thoroughly
- No submersion

**Wrist Strap:**
- Remove from device
- Hand wash with soap
- Air dry completely
- Replace if worn

### Storage

**Short-term (< 1 week):**
- Normal storage
- Any charge level OK

**Long-term (> 1 week):**
- Charge to 60-80%
- Power off
- Store in cool, dry place
- Recharge every 3 months

### Software Updates

**Firmware updates:**
1. Connect to computer via USB
2. Open PlatformIO
3. Upload new firmware
4. Device restarts automatically

**iOS app updates:**
- Automatic via App Store
- Or manual update in App Store

## Troubleshooting

### Device Won't Power On

**Try:**
1. Charge for 30 minutes
2. Press and hold Button 1 for 10 seconds
3. Connect to USB while pressing Button 1
4. Check battery connection

### Display Issues

**Blank screen:**
- Charge device
- Press any button to wake
- Restart device

**Dim display:**
- Check power saving mode
- Charge battery
- Adjust brightness in code

### Bluetooth Connection Lost

**Reconnect:**
1. Open iPhone app
2. Tap "Disconnect" then "Scan"
3. Or restart both devices
4. Re-pair if needed

**Prevent disconnection:**
- Keep iPhone within 30 feet
- Avoid metal barriers
- Keep app running in background

### Voice Not Working

**Check:**
1. Microphone not blocked
2. Speak clearly and close
3. Minimal background noise
4. iPhone app connected
5. Voice feature enabled

### Notifications Not Appearing

**Verify:**
1. BLE connected (icon showing)
2. iPhone app running
3. Notification permissions granted
4. App notification relay enabled

### Smart Home Not Responding

**Check:**
1. WiFi connected
2. MQTT broker accessible
3. Devices configured
4. Home Assistant running

## Safety Information

### Important Warnings

⚠️ **Do NOT:**
- Submerge in water
- Expose to extreme heat (>45°C/113°F)
- Open enclosure (voids warranty)
- Use damaged battery
- Charge with damaged cable
- Wear if skin irritation occurs

✅ **Do:**
- Keep dry
- Charge with recommended cable
- Remove if uncomfortable
- Store properly
- Follow all safety guidelines

### Battery Safety

- LiPo battery inside
- Do not puncture or damage
- Dispose properly at e-waste facility
- If swollen, stop use immediately
- Keep away from children

### Regulatory

- FCC compliant (Part 15)
- CE marked (if in EU)
- For personal use only
- Not a medical device

## Specifications

**Processor:** ESP32-S3 Dual Core 240MHz
**Memory:** 512KB RAM, 8MB Flash
**Display:** 1.3" OLED 128x64
**Battery:** 1000mAh LiPo (3.7V)
**Bluetooth:** BLE 5.0
**WiFi:** 802.11 b/g/n (optional)
**Buttons:** 3 tactile
**Haptics:** Vibration motor
**Audio:** MEMS microphone, I2S speaker
**Charging:** USB-C, 5V 1A
**Weight:** ~60g
**Dimensions:** 55mm x 45mm x 15mm

## Warranty & Support

### Warranty

- 90 days from completion
- Covers manufacturing defects
- Does not cover:
  - User damage
  - Battery wear
  - Normal wear and tear
  - Modifications

### Getting Help

**For issues:**
1. Check this manual
2. Review troubleshooting section
3. Check firmware logs
4. Contact support

**Firmware issues:**
- Check GitHub repository
- Review error logs
- Reflash firmware

**Hardware issues:**
- Check connections
- Verify power supply
- Test components individually

## Tips & Tricks

### Maximize Battery Life

1. Disable WiFi if not using smart home
2. Reduce notification frequency
3. Use sleep mode actively
4. Lower screen refresh rate in config

### Best Voice Recognition

1. Speak 6-12 inches from wrist
2. Articulate clearly
3. Quiet environment
4. Use wake word consistently

### Notification Management

1. Filter in iPhone app settings
2. Only relay important notifications
3. Use quick replies for common responses
4. Dismiss old notifications regularly

### Customization

**In firmware (requires reflash):**
- Change wake word
- Adjust timeouts
- Modify display layouts
- Add custom commands

**In iOS app:**
- Filter notification types
- Customize quick replies
- Adjust relay settings

## FAQ

**Q: How water resistant is it?**
A: Not water resistant. Keep dry. Light splashes OK but don't submerge.

**Q: Can I swim with it?**
A: No. Remove before swimming or showering.

**Q: Does it track fitness?**
A: Not currently. Could be added with sensors in future.

**Q: Can I make calls with it?**
A: You can receive call notifications and decline/accept via iPhone.

**Q: Does it work with Android?**
A: Not currently. iOS only for now.

**Q: Can I customize the display?**
A: Yes, by modifying firmware code.

**Q: What's the range?**
A: About 30 feet (10m) from iPhone with clear line of sight.

**Q: Can I replace the battery?**
A: Yes, but requires opening enclosure. Use same spec battery.

**Q: How do I factory reset?**
A: Reflash firmware via USB connection.

## Appendix

### Keyboard Shortcuts (Development)

**PlatformIO:**
- Ctrl+Alt+B: Build
- Ctrl+Alt+U: Upload
- Ctrl+Alt+S: Serial Monitor

**Xcode:**
- ⌘B: Build
- ⌘R: Run
- ⌘.: Stop

### Error Codes

- E01: Display initialization failed
- E02: Bluetooth initialization failed
- E03: Battery critically low
- E04: MQTT connection failed
- E05: WiFi connection failed

### Credits

**Created by:** [Your Name]
**Version:** 1.0.0
**Date:** 2024
**License:** MIT (open source)

### Acknowledgments

- Espressif for ESP32-S3
- Adafruit for libraries
- PlatformIO for development platform
- Community contributors

---

**Enjoy your Wearable Tech Pad!**

For updates and community: [GitHub Repository]
