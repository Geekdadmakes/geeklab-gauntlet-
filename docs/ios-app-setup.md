# iOS Companion App Setup Guide

This guide walks you through setting up and installing the iOS companion app for the Wearable Tech Pad.

## Prerequisites

### Requirements
- Mac computer with macOS 12.0 or later
- Xcode 14.0 or later
- Apple Developer account (free or paid)
- iPhone running iOS 15.0 or later
- Lightning/USB-C cable

## Creating the Xcode Project

Since the project files are Swift source code, you'll need to create an Xcode project first.

### 1. Create New Project

1. Open Xcode
2. File → New → Project
3. Select **iOS** → **App**
4. Click **Next**

### 2. Configure Project

Fill in the following:

- **Product Name:** GauntletCompanion
- **Team:** Select your Apple Developer team
- **Organization Identifier:** com.yourname.gauntlet
- **Bundle Identifier:** com.yourname.gauntlet.GauntletCompanion
- **Interface:** SwiftUI
- **Language:** Swift
- **Storage:** None
- **Include Tests:** Yes (optional)

Click **Next** and save to `ios-app/` directory.

### 3. Add Source Files

1. Delete the default `ContentView.swift` if it was created
2. Add the Swift files from the project:
   - Drag and drop all `.swift` files from `ios-app/` into the project
   - Ensure "Copy items if needed" is checked
   - Select "Create groups"

Files to add:
- `GauntletCompanionApp.swift`
- `ContentView.swift`
- `BluetoothManager.swift`
- `NotificationService.swift`
- `MessageHandler.swift`
- `VoiceCommandRelay.swift`

## Project Configuration

### 1. Enable Required Capabilities

In Xcode:
1. Select project in Navigator
2. Select target "GauntletCompanion"
3. Go to "Signing & Capabilities" tab
4. Click "+ Capability"

Add these capabilities:

**Bluetooth:**
- Click "+ Capability"
- Add "Background Modes"
- Check "Uses Bluetooth LE accessories"
- Check "Acts as a Bluetooth LE accessory"

**Notifications:**
- Click "+ Capability"
- Add "Push Notifications"

### 2. Update Info.plist

Add these privacy descriptions:

1. Right-click `Info.plist`
2. Open as "Source Code"
3. Add:

```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app needs Bluetooth to communicate with your Gauntlet wearable device.</string>

<key>NSBluetoothPeripheralUsageDescription</key>
<string>This app needs Bluetooth to communicate with your Gauntlet wearable device.</string>

<key>NSUserNotificationsUsageDescription</key>
<string>This app relays notifications to your Gauntlet wearable device.</string>

<key>UIBackgroundModes</key>
<array>
    <string>bluetooth-central</string>
    <string>bluetooth-peripheral</string>
</array>
```

### 3. Update OpenAI API Key

Edit `VoiceCommandRelay.swift`:

```swift
private let openAIKey = "sk-YOUR_ACTUAL_OPENAI_API_KEY"
```

**Security Note:** For production, use:
- Keychain for secure storage
- Environment variables
- Secrets management system

## Building the App

### 1. Select Target Device

In Xcode toolbar:
- Click device dropdown
- Select your iPhone (connected via cable)
- Or select "Any iOS Device" for archiving

### 2. Build Project

Press **⌘B** or Product → Build

Check for errors:
- Missing imports → Add frameworks
- Undefined symbols → Check file membership
- Swift version errors → Update build settings

### 3. Fix Common Build Errors

**Error: "No such module 'CoreBluetooth'"**
- Solution: No action needed, it's built into iOS

**Error: "Cannot find 'UIApplication' in scope"**
- Solution: Add `import UIKit` to file

**Error: "Use of undeclared type"**
- Solution: Ensure all files are in project target

## Installing on iPhone

### 1. Trust Developer Certificate

First time setup:
1. Connect iPhone to Mac
2. On iPhone: Settings → General → VPN & Device Management
3. Trust your developer certificate

### 2. Run on Device

1. Select your iPhone in Xcode
2. Press **⌘R** or click Run button
3. Enter Mac password if prompted
4. App will install and launch on iPhone

### 3. Grant Permissions

When app launches:
1. Allow Bluetooth access
2. Allow Notification access
3. Keep app open during first pairing

## Testing the App

### 1. Test Bluetooth Scanning

1. Power on Gauntlet wearable
2. In app, tap "Scan for Gauntlet"
3. Should discover device within 5 seconds
4. Connection status should turn green

### 2. Test Notifications

1. Send yourself a test text message
2. Notification should relay to Gauntlet
3. Check "Relayed" count in app

### 3. Test Voice Commands

1. On Gauntlet, activate voice command
2. Speak a query
3. iOS app should receive it
4. Response should send back to Gauntlet

## Notification Service Extension (Advanced)

To intercept all system notifications, create a Notification Service Extension:

### 1. Add Extension

1. File → New → Target
2. Select "Notification Service Extension"
3. Name: "GauntletNotificationExtension"
4. Click Finish

### 2. Implement Extension

Edit `NotificationService.swift` in extension:

```swift
import UserNotifications

class NotificationService: UNNotificationServiceExtension {
    override func didReceive(_ request: UNNotificationRequest, withContentHandler contentHandler: @escaping (UNNotificationContent) -> Void) {

        let content = (request.content.mutableCopy() as? UNMutableNotificationContent)

        // Relay to Gauntlet via app group or shared container
        if let content = content {
            relayToGauntlet(
                title: content.title,
                message: content.body
            )
            contentHandler(content)
        }
    }

    func relayToGauntlet(title: String, message: String) {
        // Use UserDefaults app group to communicate with main app
        let shared = UserDefaults(suiteName: "group.com.yourname.gauntlet")
        shared?.set(["title": title, "message": message], forKey: "pendingNotification")
    }
}
```

### 3. Configure App Groups

1. Main app target → Capabilities → App Groups
2. Add "group.com.yourname.gauntlet"
3. Extension target → Capabilities → App Groups
4. Add same group

## Debugging

### Enable Bluetooth Logging

In Xcode:
1. Product → Scheme → Edit Scheme
2. Run → Arguments
3. Add environment variable:
   - Name: `OS_ACTIVITY_MODE`
   - Value: `disable`

### View Console Logs

1. Window → Devices and Simulators
2. Select your iPhone
3. Click "Open Console"
4. Filter for "Gauntlet"

### Common Issues

**App crashes on launch**
- Check crash log in Xcode Organizer
- Verify all IBOutlets are connected
- Check for force unwrapping of optionals

**Bluetooth not discovering**
- Verify Gauntlet is powered on and advertising
- Check UUIDs match firmware exactly
- Restart both devices

**Notifications not relaying**
- Grant notification permissions
- Check notification service extension
- Verify BLE connection is stable

## Distribution

### TestFlight (Beta Testing)

1. Archive app: Product → Archive
2. Upload to App Store Connect
3. Create TestFlight beta
4. Invite testers via email

### App Store Release

1. Archive app
2. Upload to App Store Connect
3. Fill in App Store metadata
4. Submit for review
5. Wait for approval (usually 1-3 days)

### Ad-Hoc Distribution

For personal use:
1. Product → Archive
2. Distribute App → Ad Hoc
3. Select devices
4. Export IPA
5. Install via Apple Configurator

## Maintenance

### Update Dependencies

Check for updates:
```bash
# No package manager needed - uses built-in iOS frameworks
```

### Version Updates

Update version in Xcode:
1. Select project
2. General → Identity
3. Update Version and Build numbers

### Push Updates

Via TestFlight:
1. Archive new build
2. Upload to TestFlight
3. Testers get automatic update

Via App Store:
1. Submit new version
2. Wait for review
3. Users get update notification

## Advanced Features

### Background Notification Relay

Implement `didReceiveRemoteNotification` in AppDelegate:

```swift
func application(_ application: UIApplication, didReceiveRemoteNotification userInfo: [AnyHashable : Any], fetchCompletionHandler completionHandler: @escaping (UIBackgroundFetchResult) -> Void) {

    // Relay notification even when app is in background
    BluetoothManager.shared.sendNotification(
        type: 3,
        title: userInfo["title"] as? String ?? "",
        message: userInfo["body"] as? String ?? ""
    )

    completionHandler(.newData)
}
```

### Siri Shortcuts Integration

Add Intent extension for voice commands:

1. File → New → Target
2. Intents Extension
3. Implement `INSendMessageIntent`
4. Users can create Siri shortcuts

## Resources

- Apple Developer Documentation: https://developer.apple.com/documentation/
- Core Bluetooth Guide: https://developer.apple.com/bluetooth/
- UserNotifications Framework: https://developer.apple.com/documentation/usernotifications
- SwiftUI Tutorials: https://developer.apple.com/tutorials/swiftui

## Next Steps

After successful app installation:

1. Pair with Gauntlet wearable
2. Test all features
3. Configure notification preferences
4. Set up smart home integration
5. Customize voice commands

## Support

For issues:
- Check Xcode console logs
- Review Bluetooth connection status
- Verify firmware is running
- Check iOS permissions
