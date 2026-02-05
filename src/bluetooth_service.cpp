/**
 * Bluetooth Service - BLE Communication with iPhone
 * Handles notifications, messages, and commands
 */

#include "bluetooth_service.h"
#include "time_sync.h"
#include <WiFi.h>

BluetoothService bluetooth;

BluetoothService::BluetoothService()
    : bleServer(nullptr)
    , bleService(nullptr)
    , notificationChar(nullptr)
    , messageChar(nullptr)
    , commandChar(nullptr)
    , connected(false)
    , notificationCount(0)
    , commandCallback(nullptr) {

    memset(notifications, 0, sizeof(notifications));
}

bool BluetoothService::begin() {
    Serial.println("Initializing BLE...");

    // Initialize vibration motor for haptic feedback
    pinMode(VIBRATION_MOTOR, OUTPUT);
    digitalWrite(VIBRATION_MOTOR, LOW);

    // Initialize NimBLE
    NimBLEDevice::init(DEVICE_NAME);

    // Set power level (ESP_PWR_LVL_P9 = +9dBm, maximum power for best range)
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    // Set MTU size for larger data transfers
    NimBLEDevice::setMTU(512);

    setupBLEServer();

    // Start advertising
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(BLE_SERVICE_UUID);
    advertising->setScanResponse(true);

    // Set advertising interval (100ms min, 200ms max for balance of power/responsiveness)
    advertising->setMinInterval(160);  // 160 * 0.625ms = 100ms
    advertising->setMaxInterval(320);  // 320 * 0.625ms = 200ms

    advertising->start();

    Serial.println("BLE advertising started");
    Serial.printf("Device Name: %s\n", DEVICE_NAME);
    Serial.printf("Service UUID: %s\n", BLE_SERVICE_UUID);

    return true;
}

void BluetoothService::setupBLEServer() {
    // Create BLE Server
    bleServer = NimBLEDevice::createServer();
    bleServer->setCallbacks(this);

    // Create BLE Service
    bleService = bleServer->createService(BLE_SERVICE_UUID);

    // Create Notification Characteristic (iPhone -> Gauntlet)
    notificationChar = bleService->createCharacteristic(
        NOTIFICATION_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    notificationChar->setCallbacks(this);

    // Create Message Characteristic (Gauntlet -> iPhone)
    messageChar = bleService->createCharacteristic(
        MESSAGE_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    // Create Command Characteristic (Bidirectional)
    commandChar = bleService->createCharacteristic(
        COMMAND_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    commandChar->setCallbacks(this);

    // Start the service
    bleService->start();
}

void BluetoothService::update() {
    // Keep-alive and connection monitoring
    if (connected) {
        // Could add periodic ping/heartbeat here
    }
}

bool BluetoothService::isConnected() {
    return connected;
}

void BluetoothService::disconnect() {
    if (bleServer) {
        bleServer->disconnect(0);
    }
}

int BluetoothService::getConnectionStrength() {
    if (!connected || !bleServer) {
        return -100;  // No connection
    }

    // Get connection info from NimBLE
    // RSSI ranges: -30 (excellent) to -90 (poor)
    std::vector<uint16_t> connIds = bleServer->getPeerDevices();
    if (connIds.size() > 0) {
        // Get RSSI for first connected device
        // Note: NimBLE doesn't directly expose RSSI in the same way as classic BLE
        // This would require additional implementation with gap_read_rssi
        // For now, return a reasonable connected value
        return -50;  // Medium strength indicator
    }

    return -100;
}

void BluetoothService::handleIncomingNotification(const uint8_t* data, size_t length) {
    if (notificationCount >= MAX_NOTIFICATIONS) {
        // Shift notifications down, remove oldest
        for (int i = 0; i < MAX_NOTIFICATIONS - 1; i++) {
            notifications[i] = notifications[i + 1];
        }
        notificationCount = MAX_NOTIFICATIONS - 1;
    }

    Notification& notif = notifications[notificationCount];
    parseNotification(data, length, notif);
    notif.timestamp = millis();
    notif.read = false;

    notificationCount++;

    // Send acknowledgment back to iPhone
    if (commandChar && connected) {
        char ack[32];
        snprintf(ack, sizeof(ack), "ACK:%d", notificationCount);
        commandChar->setValue((uint8_t*)ack, strlen(ack));
        commandChar->notify();
    }

    // Haptic feedback for incoming notification
    digitalWrite(VIBRATION_MOTOR, HIGH);
    delay(50);
    digitalWrite(VIBRATION_MOTOR, LOW);
}

Notification BluetoothService::getLatestNotification() {
    if (notificationCount > 0) {
        return notifications[notificationCount - 1];
    }
    return Notification();
}

Notification BluetoothService::getNotification(int index) {
    if (index >= 0 && index < notificationCount) {
        return notifications[index];
    }
    return Notification();
}

int BluetoothService::getNotificationCount() {
    return notificationCount;
}

int BluetoothService::getUnreadCount() {
    int unread = 0;
    for (int i = 0; i < notificationCount; i++) {
        if (!notifications[i].read) {
            unread++;
        }
    }
    return unread;
}

void BluetoothService::clearNotifications() {
    notificationCount = 0;
    memset(notifications, 0, sizeof(notifications));
}

void BluetoothService::markNotificationRead(int index) {
    if (index >= 0 && index < notificationCount) {
        notifications[index].read = true;
    }
}

void BluetoothService::markAllRead() {
    for (int i = 0; i < notificationCount; i++) {
        notifications[i].read = true;
    }
}

bool BluetoothService::sendTextMessage(const char* recipient, const char* message) {
    if (!connected || !messageChar) {
        Serial.println("❌ Cannot send message: Not connected");
        return false;
    }

    if (!recipient || !message) {
        Serial.println("❌ Cannot send message: Invalid parameters");
        return false;
    }

    // Format: "RECIPIENT|MESSAGE"
    char buffer[256];
    int written = snprintf(buffer, sizeof(buffer), "%s|%s", recipient, message);

    if (written >= sizeof(buffer)) {
        Serial.println("⚠️  Message truncated due to size limit");
    }

    messageChar->setValue((uint8_t*)buffer, strlen(buffer));
    messageChar->notify();

    Serial.printf("📤 Sent message to: %s (%d bytes)\n", recipient, strlen(buffer));

    return true;
}

bool BluetoothService::sendQuickReply(const char* message) {
    return sendTextMessage("REPLY", message);
}

void BluetoothService::sendCommand(const char* command) {
    if (!connected || !commandChar) {
        return;
    }

    commandChar->setValue((uint8_t*)command, strlen(command));
    commandChar->notify();
}

void BluetoothService::registerCommandCallback(void (*callback)(const char*)) {
    commandCallback = callback;
}

// BLE Callbacks
void BluetoothService::onConnect(NimBLEServer* pServer) {
    connected = true;
    Serial.println("BLE Client connected!");

    // Update connection parameters for optimal performance
    // Min interval: 12 * 1.25ms = 15ms
    // Max interval: 24 * 1.25ms = 30ms
    // Latency: 0 (respond to every connection event)
    // Timeout: 400 * 10ms = 4000ms
    std::vector<uint16_t> connIds = pServer->getPeerDevices();
    if (connIds.size() > 0) {
        pServer->updateConnParams(connIds[0], 12, 24, 0, 400);
        Serial.println("Connection parameters updated for low latency");
    }

    // Haptic feedback - double pulse on connection
    digitalWrite(VIBRATION_MOTOR, HIGH);
    delay(100);
    digitalWrite(VIBRATION_MOTOR, LOW);
    delay(50);
    digitalWrite(VIBRATION_MOTOR, HIGH);
    delay(100);
    digitalWrite(VIBRATION_MOTOR, LOW);
}

void BluetoothService::onDisconnect(NimBLEServer* pServer) {
    connected = false;
    Serial.println("BLE Client disconnected!");

    // Haptic feedback - single long pulse on disconnection
    digitalWrite(VIBRATION_MOTOR, HIGH);
    delay(200);
    digitalWrite(VIBRATION_MOTOR, LOW);

    // Restart advertising
    NimBLEDevice::getAdvertising()->start();
}

void BluetoothService::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string uuid = pCharacteristic->getUUID().toString();
    std::string value = pCharacteristic->getValue();

    if (uuid == NOTIFICATION_CHAR_UUID) {
        // Handle incoming notification
        handleIncomingNotification((uint8_t*)value.c_str(), value.length());
        Serial.printf("📱 Received notification from iPhone (%d bytes, total: %d)\n",
                     value.length(), notificationCount);
    }
    else if (uuid == COMMAND_CHAR_UUID) {
        // Handle incoming command
        Serial.printf("🎮 Received command: %s\n", value.c_str());

        // Check for time sync command
        if (value.find("\"cmd\":\"SET_TIME\"") != std::string::npos) {
            handleTimeSync(value.c_str());
        } else if (commandCallback) {
            commandCallback(value.c_str());
        }
    }
}

void BluetoothService::parseNotification(const uint8_t* data, size_t length, Notification& notif) {
    // Expected format: "TYPE|TITLE|MESSAGE"
    // TYPE: 0=Call, 1=Text, 2=Email, 3=App, 4=System

    if (length < 5) return;

    char buffer[256];
    memcpy(buffer, data, min(length, sizeof(buffer) - 1));
    buffer[min(length, sizeof(buffer) - 1)] = '\0';

    // Parse type
    int type = buffer[0] - '0';
    notif.type = (NotificationType)constrain(type, 0, 4);

    // Parse title and message
    char* titleStart = strchr(buffer, '|');
    if (titleStart) {
        titleStart++; // Skip the '|'
        char* messageStart = strchr(titleStart, '|');

        if (messageStart) {
            *messageStart = '\0';
            messageStart++;

            strncpy(notif.title, titleStart, sizeof(notif.title) - 1);
            strncpy(notif.message, messageStart, sizeof(notif.message) - 1);

            notif.title[sizeof(notif.title) - 1] = '\0';
            notif.message[sizeof(notif.message) - 1] = '\0';
        }
    }
}

void BluetoothService::handleTimeSync(const char* json) {
    // Simple JSON parsing (no library needed for this simple case)
    // Expected: {"cmd":"SET_TIME","timestamp":1738177200,"timezone":"America/New_York","tzOffset":-18000}

    Serial.println("Parsing time sync command...");

    // Extract timestamp
    const char* tsStart = strstr(json, "\"timestamp\":");
    if (!tsStart) {
        Serial.println("  ERROR: No timestamp field found");
        return;
    }
    uint32_t timestamp = atol(tsStart + 12);

    // Extract timezone
    char timezone[64] = "UTC0";
    const char* tzStart = strstr(json, "\"timezone\":\"");
    if (tzStart) {
        const char* tzEnd = strchr(tzStart + 12, '"');
        if (tzEnd) {
            size_t len = min((size_t)(tzEnd - (tzStart + 12)), sizeof(timezone) - 1);
            strncpy(timezone, tzStart + 12, len);
            timezone[len] = '\0';
        }
    }

    // Extract timezone offset
    int tzOffset = 0;
    const char* offsetStart = strstr(json, "\"tzOffset\":");
    if (offsetStart) {
        tzOffset = atoi(offsetStart + 11);
    }

    // Sync via TimeSync manager
    extern TimeSync timeSyncMgr;
    if (timeSyncMgr.syncFromBLE(timestamp, timezone, tzOffset)) {
        Serial.println("✓ Time synced from BLE");

        // Send acknowledgment
        char ack[128];
        snprintf(ack, sizeof(ack),
                 "{\"cmd\":\"TIME_SYNC_ACK\",\"success\":true,\"timestamp\":%lu}",
                 timestamp);
        if (commandChar) {
            commandChar->setValue(ack);
            commandChar->notify();
        }

        // Vibration feedback
        digitalWrite(VIBRATION_MOTOR, HIGH);
        delay(100);
        digitalWrite(VIBRATION_MOTOR, LOW);
    } else {
        Serial.println("  ERROR: Time sync failed");

        // Send error acknowledgment
        const char* errorAck = "{\"cmd\":\"TIME_SYNC_ACK\",\"success\":false}";
        if (commandChar) {
            commandChar->setValue(errorAck);
            commandChar->notify();
        }
    }
}

void BluetoothService::requestTimeSync() {
    if (!connected || !commandChar) {
        Serial.println("Cannot request time sync - BLE not connected");
        return;
    }

    const char* request = "{\"cmd\":\"REQUEST_TIME\"}";
    commandChar->setValue(request);
    commandChar->notify();
    Serial.println("📡 Requested time sync from phone");
}
