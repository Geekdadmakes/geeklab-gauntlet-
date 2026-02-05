#ifndef BLUETOOTH_SERVICE_H
#define BLUETOOTH_SERVICE_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "config.h"

// Notification types
enum NotificationType {
    NOTIF_CALL,
    NOTIF_TEXT,
    NOTIF_EMAIL,
    NOTIF_APP,
    NOTIF_SYSTEM
};

// Notification structure
struct Notification {
    NotificationType type;
    char title[32];
    char message[128];
    unsigned long timestamp;
    bool read;
};

// Message structure for text responses
struct Message {
    char recipient[32];
    char content[256];
};

class BluetoothService : public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks {
public:
    BluetoothService();
    bool begin();
    void update();

    // Connection management
    bool isConnected();
    void disconnect();
    int getConnectionStrength();

    // Notification handling
    void handleIncomingNotification(const uint8_t* data, size_t length);
    Notification getLatestNotification();
    Notification getNotification(int index);  // Get notification by index
    int getNotificationCount();
    int getUnreadCount();  // Get count of unread notifications
    void clearNotifications();
    void markNotificationRead(int index);
    void markAllRead();

    // Message sending
    bool sendTextMessage(const char* recipient, const char* message);
    bool sendQuickReply(const char* message);

    // Command interface
    void sendCommand(const char* command);
    void registerCommandCallback(void (*callback)(const char*));

    // Time sync
    void requestTimeSync();

    // BLE callbacks
    void onConnect(NimBLEServer* pServer) override;
    void onDisconnect(NimBLEServer* pServer) override;
    void onWrite(NimBLECharacteristic* pCharacteristic) override;

private:
    void handleTimeSync(const char* json);
    NimBLEServer* bleServer;
    NimBLEService* bleService;
    NimBLECharacteristic* notificationChar;
    NimBLECharacteristic* messageChar;
    NimBLECharacteristic* commandChar;

    bool connected;
    Notification notifications[MAX_NOTIFICATIONS];
    int notificationCount;
    void (*commandCallback)(const char*);

    void setupBLEServer();
    void parseNotification(const uint8_t* data, size_t length, Notification& notif);
};

extern BluetoothService bluetooth;

#endif // BLUETOOTH_SERVICE_H
