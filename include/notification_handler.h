#ifndef NOTIFICATION_HANDLER_H
#define NOTIFICATION_HANDLER_H

#include <Arduino.h>
#include "bluetooth_service.h"
#include "config.h"

// Notification actions
enum NotificationAction {
    ACTION_DISMISS,
    ACTION_REPLY,
    ACTION_OPEN,
    ACTION_NONE
};

class NotificationHandler {
public:
    NotificationHandler();
    void begin();
    void update();

    // Notification display
    void showNotification(const Notification& notif);
    void dismissNotification(int index);
    void dismissAll();

    // Quick actions
    void replyToNotification(int index, const char* message);
    void sendQuickReply(int index, int templateIndex);

    // Notification management
    int getUnreadCount();
    Notification getNotification(int index);
    void navigateNotifications(int direction); // +1 or -1

    // Alert management
    void vibrate(int durationMs);
    void playNotificationSound();
    void setAlertLevel(int level); // 0=silent, 1=vibrate, 2=sound+vibrate

    // Quick reply templates
    const char* getQuickReply(int index);
    void setQuickReply(int index, const char* message);

private:
    int currentNotificationIndex;
    int unreadCount;
    int alertLevel;

    // Default quick replies
    const char* quickReplies[5] = {
        "OK",
        "On my way",
        "Call you later",
        "Busy now",
        "Thanks"
    };

    void updateDisplay();
    void triggerAlert();
};

extern NotificationHandler notificationHandler;

#endif // NOTIFICATION_HANDLER_H
