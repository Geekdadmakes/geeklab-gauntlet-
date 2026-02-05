#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <TinyGPSPlus.h>
#include <RTClib.h>
#include <MPU6050_light.h>
#include <PCF8574.h>
#include <MAX30105.h>
#include "config.h"

// Sensor availability flags
struct SensorStatus {
    bool bme280Available;
    bool gpsAvailable;
    bool rtcAvailable;
    bool imuAvailable;
    bool ioExpanderAvailable;
    bool healthSensorAvailable;  // MAX30102 heart rate/SpO2 sensor
    bool needsTimeSync;  // True if RTC needs time sync
};

// Environmental data
struct EnvironmentalData {
    float temperature;      // Celsius
    float humidity;         // Percent
    float pressure;         // hPa
    float altitude;         // Meters
    unsigned long timestamp;
};

// GPS data
struct GPSData {
    double latitude;
    double longitude;
    float speed;            // km/h
    float altitude;         // Meters
    int satellites;
    bool hasFixx;
    DateTime timestamp;
};

// Motion data
struct MotionData {
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float angleX;
    float angleY;
    float angleZ;
    int stepCount;
    unsigned long timestamp;
};

// Health data (MAX30102)
struct HealthData {
    int heartRate;          // BPM (beats per minute)
    int spo2;               // SpO2 percentage (95-100% normal)
    int ir;                 // IR LED reading (signal strength)
    int red;                // Red LED reading
    bool fingerDetected;    // True if finger is on sensor
    bool validReading;      // True if readings are reliable
    uint8_t signalQuality;  // 0-100 quality indicator
    unsigned long timestamp;
};

class SensorManager {
public:
    SensorManager();
    bool begin();
    void update();

    // Sensor status
    SensorStatus getStatus();
    bool isReady();

    // Environmental sensors (BME280)
    float getTemperature();
    float getHumidity();
    float getPressure();
    float getAltitude();
    EnvironmentalData getEnvironmentalData();

    // GPS (NEO6M)
    double getLatitude();
    double getLongitude();
    float getSpeed();
    float getGPSAltitude();
    int getSatellites();
    bool hasGPSFix();
    GPSData getGPSData();

    // Motion sensor (MPU6050)
    float getAccelX();
    float getAccelY();
    float getAccelZ();
    float getGyroX();
    float getGyroY();
    float getGyroZ();
    float getAngleX();
    float getAngleY();
    float getAngleZ();
    int getStepCount();
    MotionData getMotionData();

    // Real-Time Clock (DS3231)
    DateTime getCurrentTime();
    void setRTCTime(DateTime dt);
    float getRTCTemperature();

    // Health Sensor (MAX30102)
    int getHeartRate();
    int getSpO2();
    bool getFingerDetected();
    uint8_t getSignalQuality();
    HealthData getHealthData();

    // I/O Expander (PCF8574)
    bool getExpanderPin(uint8_t pin);
    void setExpanderPin(uint8_t pin, bool state);
    void toggleExpanderPin(uint8_t pin);
    uint8_t readExpanderByte();
    void writeExpanderByte(uint8_t value);

    // Power management
    void setSensorSleep(bool sleep);
    void enableSensor(const char* sensorName, bool enable);

    // Calibration
    void calibrateIMU();
    void resetStepCount();

private:
    // Sensor objects
    Adafruit_BME280* bme;
    TinyGPSPlus* gps;
    RTC_DS3231* rtc;
    MPU6050* imu;
    PCF8574* ioExpander;
    MAX30105* healthSensor;
    HardwareSerial* gpsSerial;

    // I2C buses
    TwoWire* i2c1;  // Primary sensors
    TwoWire* i2c2;  // RTC

    // Sensor status
    SensorStatus status;

    // Cached data
    EnvironmentalData envData;
    GPSData gpsData;
    MotionData motionData;
    HealthData healthData;
    DateTime currentTime;

    // Update timers
    unsigned long lastEnvUpdate;
    unsigned long lastGPSUpdate;
    unsigned long lastMotionUpdate;
    unsigned long lastRTCUpdate;
    unsigned long lastHealthUpdate;

    // Update intervals (ms)
    const unsigned long ENV_UPDATE_INTERVAL = 2000;   // 2 seconds
    const unsigned long GPS_UPDATE_INTERVAL = 1000;   // 1 second
    const unsigned long MOTION_UPDATE_INTERVAL = 100; // 100ms
    const unsigned long RTC_UPDATE_INTERVAL = 1000;   // 1 second
    const unsigned long HEALTH_UPDATE_INTERVAL = 100; // 100ms (fast for smooth graph)

    // Step detection
    float lastAccelMagnitude;
    unsigned long lastStepTime;
    const float STEP_THRESHOLD = 1.2;
    const unsigned long STEP_MIN_INTERVAL = 200; // ms

    // Helper functions
    void updateEnvironmental();
    void updateGPS();
    void updateMotion();
    void updateRTC();
    void updateHealth();
    void detectStep();
    float calculateAccelMagnitude();
};

extern SensorManager sensorMgr;

#endif // SENSOR_MANAGER_H
