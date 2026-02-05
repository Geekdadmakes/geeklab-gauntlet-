/**
 * Smart Home MQTT - Home Automation Control
 * Connects to Home Assistant via MQTT for device control
 */

#include "smart_home_mqtt.h"

SmartHomeMQTT::SmartHomeMQTT()
    : mqttClient(nullptr)
    , deviceCount(0)
    , connected(false) {

    memset(devices, 0, sizeof(devices));
}

bool SmartHomeMQTT::begin() {
    Serial.println("Initializing Smart Home MQTT...");

    mqttClient = new PubSubClient(wifiClient);
    mqttClient->setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient->setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->messageCallback(topic, payload, length);
    });

    Serial.println("Smart Home MQTT initialized (not connected)");
    Serial.println("Connect to WiFi to enable smart home features");

    return true;
}

void SmartHomeMQTT::update() {
    if (!connected) {
        // Try to reconnect periodically
        static unsigned long lastReconnect = 0;
        if (millis() - lastReconnect > 30000) {
            connect();
            lastReconnect = millis();
        }
        return;
    }

    if (mqttClient) {
        mqttClient->loop();
    }
}

bool SmartHomeMQTT::connect() {
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, cannot connect MQTT");
        return false;
    }

    if (mqttClient->connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
        Serial.println("MQTT connected!");
        connected = true;

        // Subscribe to device state topics
        for (int i = 0; i < deviceCount; i++) {
            subscribe(devices[i].stateTopic);
        }

        // Publish discovery messages for Home Assistant
        publishDiscovery();

        return true;
    } else {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(mqttClient->state());
        connected = false;
        return false;
    }
}

void SmartHomeMQTT::disconnect() {
    if (mqttClient) {
        mqttClient->disconnect();
    }
    connected = false;
    Serial.println("MQTT disconnected");
}

bool SmartHomeMQTT::isConnected() {
    return connected && mqttClient && mqttClient->connected();
}

void SmartHomeMQTT::addDevice(const SmartDevice& device) {
    if (deviceCount >= 20) {
        Serial.println("Device limit reached");
        return;
    }

    devices[deviceCount] = device;
    deviceCount++;

    // Subscribe to state topic if connected
    if (isConnected()) {
        subscribe(device.stateTopic);
    }

    Serial.print("Added device: ");
    Serial.println(device.name);
}

void SmartHomeMQTT::removeDevice(const char* deviceId) {
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(devices[i].id, deviceId) == 0) {
            // Shift remaining devices
            for (int j = i; j < deviceCount - 1; j++) {
                devices[j] = devices[j + 1];
            }
            deviceCount--;
            Serial.print("Removed device: ");
            Serial.println(deviceId);
            return;
        }
    }
}

SmartDevice* SmartHomeMQTT::getDevice(const char* deviceId) {
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(devices[i].id, deviceId) == 0) {
            return &devices[i];
        }
    }
    return nullptr;
}

int SmartHomeMQTT::getDeviceCount() {
    return deviceCount;
}

bool SmartHomeMQTT::toggleDevice(const char* deviceId) {
    SmartDevice* device = getDevice(deviceId);
    if (!device) {
        Serial.print("Device not found: ");
        Serial.println(deviceId);
        return false;
    }

    bool newState = !device->state;
    return setDeviceState(deviceId, newState);
}

bool SmartHomeMQTT::setDeviceState(const char* deviceId, bool state) {
    SmartDevice* device = getDevice(deviceId);
    if (!device) {
        return false;
    }

    const char* payload = state ? "ON" : "OFF";
    publish(device->commandTopic, payload);

    device->state = state;

    Serial.print("Set device ");
    Serial.print(deviceId);
    Serial.print(" to ");
    Serial.println(payload);

    return true;
}

bool SmartHomeMQTT::setDeviceValue(const char* deviceId, int value) {
    SmartDevice* device = getDevice(deviceId);
    if (!device) {
        return false;
    }

    char payload[16];
    snprintf(payload, sizeof(payload), "%d", value);
    publish(device->commandTopic, payload);

    device->value = value;

    Serial.print("Set device ");
    Serial.print(deviceId);
    Serial.print(" value to ");
    Serial.println(value);

    return true;
}

bool SmartHomeMQTT::getDeviceState(const char* deviceId) {
    SmartDevice* device = getDevice(deviceId);
    if (device) {
        return device->state;
    }
    return false;
}

int SmartHomeMQTT::getDeviceValue(const char* deviceId) {
    SmartDevice* device = getDevice(deviceId);
    if (device) {
        return device->value;
    }
    return 0;
}

void SmartHomeMQTT::messageCallback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to string
    char message[256];
    memcpy(message, payload, min(length, sizeof(message) - 1));
    message[min(length, sizeof(message) - 1)] = '\0';

    Serial.print("MQTT message on topic: ");
    Serial.print(topic);
    Serial.print(" - ");
    Serial.println(message);

    handleStateUpdate(topic, message);
}

void SmartHomeMQTT::publishDiscovery() {
    // Publish Home Assistant MQTT Discovery messages
    Serial.println("Publishing MQTT discovery messages...");

    // Publish wearable as device tracker
    const char* discoveryTopic = "homeassistant/device_tracker/gauntlet/config";
    const char* discoveryPayload = R"({
        "name": "Wearable Gauntlet",
        "unique_id": "gauntlet_wearable",
        "state_topic": "gauntlet/state",
        "json_attributes_topic": "gauntlet/attributes"
    })";

    publish(discoveryTopic, discoveryPayload);

    // Publish battery sensor
    const char* batteryTopic = "homeassistant/sensor/gauntlet_battery/config";
    const char* batteryPayload = R"({
        "name": "Gauntlet Battery",
        "unique_id": "gauntlet_battery",
        "state_topic": "gauntlet/battery",
        "unit_of_measurement": "%",
        "device_class": "battery"
    })";

    publish(batteryTopic, batteryPayload);
}

void SmartHomeMQTT::publishState(const char* sensor, const char* value) {
    char topic[64];
    snprintf(topic, sizeof(topic), "gauntlet/%s", sensor);
    publish(topic, value);
}

void SmartHomeMQTT::subscribe(const char* topic) {
    if (mqttClient && isConnected()) {
        mqttClient->subscribe(topic);
        Serial.print("Subscribed to: ");
        Serial.println(topic);
    }
}

void SmartHomeMQTT::publish(const char* topic, const char* payload) {
    if (mqttClient && isConnected()) {
        mqttClient->publish(topic, payload);
        Serial.print("Published to ");
        Serial.print(topic);
        Serial.print(": ");
        Serial.println(payload);
    }
}

void SmartHomeMQTT::handleStateUpdate(const char* topic, const char* payload) {
    // Find device by state topic
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(devices[i].stateTopic, topic) == 0) {
            // Update device state
            if (strcmp(payload, "ON") == 0) {
                devices[i].state = true;
            } else if (strcmp(payload, "OFF") == 0) {
                devices[i].state = false;
            } else {
                // Try to parse as numeric value
                devices[i].value = atoi(payload);
            }

            Serial.print("Updated device ");
            Serial.print(devices[i].name);
            Serial.print(" state: ");
            Serial.println(payload);
            return;
        }
    }
}

void SmartHomeMQTT::reconnect() {
    if (isConnected()) {
        return;
    }

    Serial.println("Attempting MQTT reconnection...");
    connect();
}
