#ifndef SMART_HOME_MQTT_H
#define SMART_HOME_MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "config.h"

// Device types
enum DeviceType {
    DEVICE_LIGHT,
    DEVICE_SWITCH,
    DEVICE_THERMOSTAT,
    DEVICE_LOCK,
    DEVICE_SENSOR
};

// Device structure
struct SmartDevice {
    char id[32];
    char name[32];
    DeviceType type;
    bool state;
    int value; // brightness, temperature, etc.
    char stateTopic[64];
    char commandTopic[64];
};

class SmartHomeMQTT {
public:
    SmartHomeMQTT();
    bool begin();
    void update();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected();

    // Device management
    void addDevice(const SmartDevice& device);
    void removeDevice(const char* deviceId);
    SmartDevice* getDevice(const char* deviceId);
    int getDeviceCount();

    // Device control
    bool toggleDevice(const char* deviceId);
    bool setDeviceState(const char* deviceId, bool state);
    bool setDeviceValue(const char* deviceId, int value);

    // State queries
    bool getDeviceState(const char* deviceId);
    int getDeviceValue(const char* deviceId);

    // MQTT callbacks
    void messageCallback(char* topic, byte* payload, unsigned int length);

    // Home Assistant integration
    void publishDiscovery();
    void publishState(const char* sensor, const char* value);

private:
    WiFiClient wifiClient;
    PubSubClient* mqttClient;
    SmartDevice devices[20];
    int deviceCount;
    bool connected;

    void subscribe(const char* topic);
    void publish(const char* topic, const char* payload);
    void handleStateUpdate(const char* topic, const char* payload);
    void reconnect();
};

extern SmartHomeMQTT smartHome;

#endif // SMART_HOME_MQTT_H
