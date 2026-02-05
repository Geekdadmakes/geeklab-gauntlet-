/**
 * Notification Handler - Manages and Displays Notifications
 * Provides interface for viewing and responding to notifications
 */

#include "notification_handler.h"
#include "display_manager.h"

extern DisplayManager displayMgr;
extern BluetoothService bluetooth;

NotificationHandler::NotificationHandler()
    : currentNotificationIndex(0)
    , unreadCount(0)
    , alertLevel(2) { // Default: vibrate + sound
}

void NotificationHandler::begin() {
    Serial.println("Notification Handler initialized");
}

void NotificationHandler::update() {
    // Check for new notifications
    int totalNotifs = bluetooth.getNotificationCount();
    int newCount = totalNotifs - (totalNotifs - unreadCount);

    if (newCount > 0) {
        unreadCount += newCount;
        updateDisplay();
    }
}

void NotificationHandler::showNotification(const Notification& notif) {
    // Display the notification
    char timeStr[16];
    unsigned long seconds = notif.timestamp / 1000;
    snprintf(timeStr, sizeof(timeStr), "%lus ago", (millis() - notif.timestamp) / 1000);
    displayMgr.showNotificationDetail(notif.title, notif.message, timeStr);

    // Trigger alert
    triggerAlert();

    // Update unread count
    unreadCount++;
}

void NotificationHandler::dismissNotification(int index) {
    bluetooth.markNotificationRead(index);
    unreadCount--;
    if (unreadCount < 0) unreadCount = 0;

    Serial.print("Dismissed notification ");
    Serial.println(index);

    // Small haptic feedback
    vibrate(50);
}

void NotificationHandler::dismissAll() {
    bluetooth.clearNotifications();
    unreadCount = 0;

    Serial.println("Dismissed all notifications");
}

void NotificationHandler::replyToNotification(int index, const char* message) {
    // Get notification details
    // For now, assume it's a text message notification
    bluetooth.sendTextMessage("REPLY", message);

    Serial.print("Replied to notification ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(message);

    dismissNotification(index);
}

void NotificationHandler::sendQuickReply(int index, int templateIndex) {
    if (templateIndex < 0 || templateIndex >= 5) {
        return;
    }

    const char* reply = quickReplies[templateIndex];
    replyToNotification(index, reply);
}

int NotificationHandler::getUnreadCount() {
    return unreadCount;
}

Notification NotificationHandler::getNotification(int index) {
    // This would get from bluetooth service
    return bluetooth.getLatestNotification();
}

void NotificationHandler::navigateNotifications(int direction) {
    int totalNotifs = bluetooth.getNotificationCount();

    if (totalNotifs == 0) {
        return;
    }

    currentNotificationIndex += direction;

    // Wrap around
    if (currentNotificationIndex < 0) {
        currentNotificationIndex = totalNotifs - 1;
    } else if (currentNotificationIndex >= totalNotifs) {
        currentNotificationIndex = 0;
    }

    updateDisplay();
}

void NotificationHandler::vibrate(int durationMs) {
    digitalWrite(VIBRATION_MOTOR, HIGH);
    delay(durationMs);
    digitalWrite(VIBRATION_MOTOR, LOW);
}

void NotificationHandler::playNotificationSound() {
    // Play a simple beep tone
    // Real implementation would use I2S speaker

    Serial.println("*BEEP*");

    // In real implementation:
    // - Generate tone (e.g., 1kHz for 200ms)
    // - Send to I2S speaker via MAX98357A
}

void NotificationHandler::setAlertLevel(int level) {
    alertLevel = constrain(level, 0, 2);

    Serial.print("Alert level set to: ");
    Serial.println(alertLevel);
}

const char* NotificationHandler::getQuickReply(int index) {
    if (index < 0 || index >= 5) {
        return "";
    }
    return quickReplies[index];
}

void NotificationHandler::setQuickReply(int index, const char* message) {
    if (index < 0 || index >= 5) {
        return;
    }

    // Note: This won't work with const array
    // In real implementation, use dynamic memory
    Serial.print("Quick reply ");
    Serial.print(index);
    Serial.print(" set to: ");
    Serial.println(message);
}

void NotificationHandler::updateDisplay() {
    // Get current notification
    if (bluetooth.getNotificationCount() > 0) {
        Notification notif = bluetooth.getLatestNotification();
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%lus ago", (millis() - notif.timestamp) / 1000);
        displayMgr.showNotificationDetail(notif.title, notif.message, timeStr);
    }
}

void NotificationHandler::triggerAlert() {
    switch (alertLevel) {
        case 0: // Silent
            break;

        case 1: // Vibrate only
            vibrate(200);
            break;

        case 2: // Vibrate + Sound
            vibrate(200);
            playNotificationSound();
            break;
    }
}
