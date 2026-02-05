#include "sensor_manager.h"

SensorManager sensorMgr;

SensorManager::SensorManager() :
    bme(nullptr),
    gps(nullptr),
    rtc(nullptr),
    imu(nullptr),
    ioExpander(nullptr),
    healthSensor(nullptr),
    gpsSerial(nullptr),
    i2c1(nullptr),
    i2c2(nullptr),
    lastEnvUpdate(0),
    lastGPSUpdate(0),
    lastMotionUpdate(0),
    lastRTCUpdate(0),
    lastHealthUpdate(0),
    lastAccelMagnitude(0),
    lastStepTime(0)
{
    // Initialize status
    status.bme280Available = false;
    status.gpsAvailable = false;
    status.rtcAvailable = false;
    status.imuAvailable = false;
    status.ioExpanderAvailable = false;
    status.healthSensorAvailable = false;
    status.needsTimeSync = false;

    // Initialize data structures
    envData = {0, 0, 0, 0, 0};
    gpsData = {0, 0, 0, 0, 0, false, DateTime()};
    motionData = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    healthData = {0, 0, 0, 0, false, false, 0, 0};
}

bool SensorManager::begin() {
    Serial.println("Initializing sensors (DS3231 RTC + MPU6050 IMU + GPS NEO6M + BME280)...");

    // Initialize I2C Bus 1 (for MPU6050 and BME280)
    i2c1 = &Wire;
    i2c1->begin(I2C1_SDA, I2C1_SCL);
    Serial.printf("I2C Bus 1 initialized on GPIO %d (SDA) and %d (SCL)\n", I2C1_SDA, I2C1_SCL);
    delay(100);  // Give I2C bus time to stabilize

    // Scan I2C Bus 1 for devices
    Serial.println("Scanning I2C Bus 1 for devices...");
    int deviceCount = 0;
    for (byte addr = 1; addr < 127; addr++) {
        i2c1->beginTransmission(addr);
        byte error = i2c1->endTransmission();
        if (error == 0) {
            Serial.printf("  Device found at address 0x%02X\n", addr);
            deviceCount++;
        }
    }
    Serial.printf("I2C scan complete. Found %d device(s).\n", deviceCount);

    // Initialize BME280 environmental sensor (try both possible addresses)
    Serial.println("Attempting BME280 initialization...");
    bme = new Adafruit_BME280();

    // Try address 0x76 first (most common)
    if (bme->begin(0x76, i2c1)) {
        status.bme280Available = true;
        Serial.println("✓ BME280 initialized at address 0x76");

        bme->setSampling(Adafruit_BME280::MODE_NORMAL,
                        Adafruit_BME280::SAMPLING_X2,  // temperature
                        Adafruit_BME280::SAMPLING_X16, // pressure
                        Adafruit_BME280::SAMPLING_X1,  // humidity
                        Adafruit_BME280::FILTER_X16,
                        Adafruit_BME280::STANDBY_MS_500);
        Serial.println("  ✓ BME280 configured");
    }
    // Try address 0x77 (alternative address)
    else if (bme->begin(0x77, i2c1)) {
        status.bme280Available = true;
        Serial.println("✓ BME280 initialized at address 0x77");

        bme->setSampling(Adafruit_BME280::MODE_NORMAL,
                        Adafruit_BME280::SAMPLING_X2,
                        Adafruit_BME280::SAMPLING_X16,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::FILTER_X16,
                        Adafruit_BME280::STANDBY_MS_500);
        Serial.println("  ✓ BME280 configured");
    }
    else {
        Serial.println("✗ BME280 not found at address 0x76 or 0x77!");
        status.bme280Available = false;
    }

    // Initialize I2C bus 2 (for DS3231 RTC)
    i2c2 = &Wire1;
    i2c2->begin(I2C2_SDA, I2C2_SCL);
    Serial.printf("I2C Bus 2 initialized on GPIO %d (SDA) and %d (SCL)\n", I2C2_SDA, I2C2_SCL);

    // Initialize DS3231 RTC
    Serial.println("Attempting DS3231 RTC initialization...");
    delay(100);  // Give I2C bus time to stabilize

    rtc = new RTC_DS3231();
    if (rtc->begin(i2c2)) {
        status.rtcAvailable = true;
        Serial.println("✓ DS3231 RTC initialized at address 0x68");

        // Check if RTC lost power or has invalid time
        if (rtc->lostPower()) {
            Serial.println("  ⚠️ RTC lost power - setting to compile time");
            rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));
            currentTime = rtc->now();
            Serial.printf("  ✓ RTC set to: %04d-%02d-%02d %02d:%02d:%02d\n",
                         currentTime.year(), currentTime.month(), currentTime.day(),
                         currentTime.hour(), currentTime.minute(), currentTime.second());
            status.needsTimeSync = false;
        } else {
            // RTC has battery backup - check if time is valid
            currentTime = rtc->now();
            Serial.printf("  ✓ RTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
                         currentTime.year(), currentTime.month(), currentTime.day(),
                         currentTime.hour(), currentTime.minute(), currentTime.second());

            // If time is stale (year < 2025), update to compile time
            if (currentTime.year() < 2025) {
                Serial.println("  ⚠️ RTC time invalid (year < 2025) - setting to compile time");
                rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));
                currentTime = rtc->now();
                Serial.printf("  ✓ RTC set to: %04d-%02d-%02d %02d:%02d:%02d\n",
                             currentTime.year(), currentTime.month(), currentTime.day(),
                             currentTime.hour(), currentTime.minute(), currentTime.second());
            }
            status.needsTimeSync = false;
        }

        // Make sure oscillator is enabled (clear EOSC bit)
        rtc->writeSqwPinMode(DS3231_OFF);  // This also ensures oscillator is running

        delay(100);  // Give RTC time to start ticking

        // Test if RTC is actually ticking
        delay(2000);  // Wait 2 seconds
        DateTime testTime = rtc->now();
        Serial.printf("  RTC test (2s later): %04d-%02d-%02d %02d:%02d:%02d\n",
                     testTime.year(), testTime.month(), testTime.day(),
                     testTime.hour(), testTime.minute(), testTime.second());

        if (testTime.second() == currentTime.second()) {
            Serial.println("  ⚠️ WARNING: RTC may not be ticking!");
        } else {
            Serial.println("  ✓ RTC is ticking normally");
        }
    } else {
        Serial.println("✗ DS3231 RTC not found at address 0x68!");
        status.rtcAvailable = false;
    }

    // Initialize MPU6050 (IMU)
    Serial.println("Attempting MPU6050 IMU initialization...");
    delay(100);

    imu = new MPU6050(*i2c1);
    if (imu->begin() == 0) {
        status.imuAvailable = true;
        Serial.println("✓ MPU6050 initialized at address 0x69");

        // Calculate offsets (calibration)
        Serial.println("  Calibrating IMU... Keep device still for 3 seconds!");
        imu->calcOffsets(true, true);  // Gyro and accelerometer
        Serial.println("  ✓ Calibration complete");

        motionData.stepCount = 0;
    } else {
        Serial.println("✗ MPU6050 not found at address 0x69!");
        status.imuAvailable = false;
    }

    // Initialize MAX30102 (Heart Rate & SpO2)
    Serial.println("Attempting MAX30102 health sensor initialization...");
    delay(100);

    healthSensor = new MAX30105();
    if (healthSensor->begin(*i2c1, I2C_SPEED_STANDARD)) {
        status.healthSensorAvailable = true;
        Serial.println("✓ MAX30102 initialized at address 0x57");

        // Configure sensor
        byte ledBrightness = 0x1F;  // Options: 0=Off to 255=50mA
        byte sampleAverage = 4;      // Options: 1, 2, 4, 8, 16, 32
        byte ledMode = 2;            // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
        int sampleRate = 100;        // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
        int pulseWidth = 411;        // Options: 69, 118, 215, 411
        int adcRange = 4096;         // Options: 2048, 4096, 8192, 16384

        healthSensor->setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
        healthSensor->setPulseAmplitudeRed(0x0A);  // Turn Red LED to low for power savings
        healthSensor->setPulseAmplitudeIR(0x1F);   // Turn IR LED to full power

        Serial.println("  ✓ MAX30102 configured for heart rate & SpO2");
    } else {
        Serial.println("✗ MAX30102 not found at address 0x57!");
        status.healthSensorAvailable = false;
    }

    // rtc = new RTC_DS3231();
    // if (rtc->begin(i2c2)) {
    //     status.rtcAvailable = true;
    //     Serial.println("✓ DS3231 RTC initialized (0x68)");
    //
    //     if (rtc->lostPower()) {
    //         Serial.println("  RTC lost power, setting time to compile time");
    //         rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));
    //     }
    //
    //     currentTime = rtc->now();
    // } else {
    //     Serial.println("✗ DS3231 RTC not found!");
    // }

    // Initialize PCF8574 I/O Expander - TEMPORARILY DISABLED
    Serial.println("⚠ PCF8574 I/O Expander initialization skipped (debugging)");
    status.ioExpanderAvailable = false;

    // ioExpander = new PCF8574(0x20, i2c1);
    // if (ioExpander->begin()) {
    //     status.ioExpanderAvailable = true;
    //     Serial.println("✓ PCF8574 I/O Expander initialized (0x20)");
    // } else {
    //     Serial.println("✗ PCF8574 I/O Expander not found!");
    // }

    // Initialize GPS (NEO6M via UART)
    Serial.println("Initializing GPS (NEO6M) on Serial2...");
    gpsSerial = &Serial2;
    gpsSerial->begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
    gps = new TinyGPSPlus();
    status.gpsAvailable = true;  // Assume available, check for fix later
    Serial.printf("✓ GPS Serial initialized on GPIO %d (RX) and %d (TX) at %d baud\n", GPS_RX, GPS_TX, GPS_BAUD);

    Serial.printf("\nSensor Summary: BME:%d GPS:%d RTC:%d IMU:%d Health:%d IO:%d\n",
                  status.bme280Available,
                  status.gpsAvailable,
                  status.rtcAvailable,
                  status.imuAvailable,
                  status.healthSensorAvailable,
                  status.ioExpanderAvailable);

    return isReady();
}

void SensorManager::update() {
    unsigned long now = millis();

    // Update GPS (feed parser)
    while (gpsSerial && gpsSerial->available() > 0) {
        gps->encode(gpsSerial->read());
    }

    // Update environmental data
    if (now - lastEnvUpdate >= ENV_UPDATE_INTERVAL) {
        updateEnvironmental();
        lastEnvUpdate = now;
    }

    // Update GPS data
    if (now - lastGPSUpdate >= GPS_UPDATE_INTERVAL) {
        updateGPS();
        lastGPSUpdate = now;
    }

    // Update motion data
    if (now - lastMotionUpdate >= MOTION_UPDATE_INTERVAL) {
        updateMotion();
        lastMotionUpdate = now;
    }

    // Update RTC
    if (now - lastRTCUpdate >= RTC_UPDATE_INTERVAL) {
        updateRTC();
        lastRTCUpdate = now;
    }

    // Update health sensor
    if (now - lastHealthUpdate >= HEALTH_UPDATE_INTERVAL) {
        updateHealth();
        lastHealthUpdate = now;
    }
}

SensorStatus SensorManager::getStatus() {
    return status;
}

bool SensorManager::isReady() {
    // At minimum, we need RTC for timekeeping
    return status.rtcAvailable;
}

// ============================================================================
// Environmental Sensors (BME280)
// ============================================================================

void SensorManager::updateEnvironmental() {
    if (!status.bme280Available || bme == nullptr) return;

    envData.temperature = bme->readTemperature();
    envData.humidity = bme->readHumidity();
    envData.pressure = bme->readPressure() / 100.0F;  // Convert Pa to hPa
    envData.altitude = bme->readAltitude(1013.25);    // Sea level pressure
    envData.timestamp = millis();

    // Debug output every 5 seconds
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 5000) {
        Serial.printf("BME280: T=%.1f H=%.1f P=%.1f\n",
                     envData.temperature, envData.humidity, envData.pressure);
        lastDebug = millis();
    }
}

float SensorManager::getTemperature() {
    return envData.temperature;
}

float SensorManager::getHumidity() {
    return envData.humidity;
}

float SensorManager::getPressure() {
    return envData.pressure;
}

float SensorManager::getAltitude() {
    return envData.altitude;
}

EnvironmentalData SensorManager::getEnvironmentalData() {
    return envData;
}

// ============================================================================
// GPS (NEO6M)
// ============================================================================

void SensorManager::updateGPS() {
    if (!status.gpsAvailable || !gps) return;

    gpsData.latitude = gps->location.lat();
    gpsData.longitude = gps->location.lng();
    gpsData.speed = gps->speed.kmph();
    gpsData.altitude = gps->altitude.meters();
    gpsData.satellites = gps->satellites.value();
    gpsData.hasFixx = gps->location.isValid();

    if (gps->date.isValid() && gps->time.isValid()) {
        gpsData.timestamp = DateTime(gps->date.year(), gps->date.month(),
                                     gps->date.day(), gps->time.hour(),
                                     gps->time.minute(), gps->time.second());
    }
}

double SensorManager::getLatitude() {
    return gpsData.latitude;
}

double SensorManager::getLongitude() {
    return gpsData.longitude;
}

float SensorManager::getSpeed() {
    return gpsData.speed;
}

float SensorManager::getGPSAltitude() {
    return gpsData.altitude;
}

int SensorManager::getSatellites() {
    return gpsData.satellites;
}

bool SensorManager::hasGPSFix() {
    return gpsData.hasFixx;
}

GPSData SensorManager::getGPSData() {
    return gpsData;
}

// ============================================================================
// Motion Sensor (MPU6050)
// ============================================================================

void SensorManager::updateMotion() {
    if (!status.imuAvailable || !imu) return;

    imu->update();

    motionData.accelX = imu->getAccX();
    motionData.accelY = imu->getAccY();
    motionData.accelZ = imu->getAccZ();

    motionData.gyroX = imu->getGyroX();
    motionData.gyroY = imu->getGyroY();
    motionData.gyroZ = imu->getGyroZ();

    motionData.angleX = imu->getAngleX();
    motionData.angleY = imu->getAngleY();
    motionData.angleZ = imu->getAngleZ();

    motionData.timestamp = millis();

    // Detect steps
    detectStep();
}

void SensorManager::detectStep() {
    float accelMag = calculateAccelMagnitude();
    unsigned long now = millis();

    // Simple step detection: look for acceleration spike
    if (accelMag > STEP_THRESHOLD &&
        lastAccelMagnitude < STEP_THRESHOLD &&
        (now - lastStepTime) > STEP_MIN_INTERVAL) {

        motionData.stepCount++;
        lastStepTime = now;

#ifdef DEBUG_STEPS
        Serial.printf("Step detected! Count: %d\n", motionData.stepCount);
#endif
    }

    lastAccelMagnitude = accelMag;
}

float SensorManager::calculateAccelMagnitude() {
    return sqrt(motionData.accelX * motionData.accelX +
                motionData.accelY * motionData.accelY +
                motionData.accelZ * motionData.accelZ);
}

float SensorManager::getAccelX() { return motionData.accelX; }
float SensorManager::getAccelY() { return motionData.accelY; }
float SensorManager::getAccelZ() { return motionData.accelZ; }
float SensorManager::getGyroX() { return motionData.gyroX; }
float SensorManager::getGyroY() { return motionData.gyroY; }
float SensorManager::getGyroZ() { return motionData.gyroZ; }
float SensorManager::getAngleX() { return motionData.angleX; }
float SensorManager::getAngleY() { return motionData.angleY; }
float SensorManager::getAngleZ() { return motionData.angleZ; }

int SensorManager::getStepCount() {
    return motionData.stepCount;
}

MotionData SensorManager::getMotionData() {
    return motionData;
}

void SensorManager::calibrateIMU() {
    if (!status.imuAvailable || !imu) return;

    Serial.println("Calibrating IMU... Keep device still!");
    imu->calcOffsets(true, true);
    Serial.println("Calibration complete");
}

void SensorManager::resetStepCount() {
    motionData.stepCount = 0;
}

// ============================================================================
// Real-Time Clock (DS3231)
// ============================================================================

void SensorManager::updateRTC() {
    if (!status.rtcAvailable || !rtc) return;

    currentTime = rtc->now();

    // Debug: Print RTC time every update
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 5000) {
        Serial.printf("RTC Update: %04d-%02d-%02d %02d:%02d:%02d\n",
                     currentTime.year(), currentTime.month(), currentTime.day(),
                     currentTime.hour(), currentTime.minute(), currentTime.second());
        lastDebug = millis();
    }
}

DateTime SensorManager::getCurrentTime() {
    return currentTime;
}

void SensorManager::setRTCTime(DateTime dt) {
    if (!status.rtcAvailable || !rtc) return;

    rtc->adjust(dt);
    currentTime = dt;
    Serial.println("RTC time updated");
}

float SensorManager::getRTCTemperature() {
    if (!status.rtcAvailable || !rtc) return 0.0;

    return rtc->getTemperature();
}

// ============================================================================
// I/O Expander (PCF8574)
// ============================================================================

bool SensorManager::getExpanderPin(uint8_t pin) {
    if (!status.ioExpanderAvailable || !ioExpander) return false;

    return ioExpander->read(pin);
}

void SensorManager::setExpanderPin(uint8_t pin, bool state) {
    if (!status.ioExpanderAvailable || !ioExpander) return;

    ioExpander->write(pin, state ? HIGH : LOW);
}

void SensorManager::toggleExpanderPin(uint8_t pin) {
    if (!status.ioExpanderAvailable || !ioExpander) return;

    ioExpander->toggle(pin);
}

uint8_t SensorManager::readExpanderByte() {
    if (!status.ioExpanderAvailable || !ioExpander) return 0;

    return ioExpander->read8();
}

void SensorManager::writeExpanderByte(uint8_t value) {
    if (!status.ioExpanderAvailable || !ioExpander) return;

    ioExpander->write8(value);
}

// ============================================================================
// Power Management
// ============================================================================

void SensorManager::setSensorSleep(bool sleep) {
    if (sleep) {
        Serial.println("Putting sensors to sleep...");
        // BME280 will enter sleep mode automatically between readings
        // MPU6050 can be put to sleep
        if (status.imuAvailable && imu) {
            // Note: MPU6050_light library may not have sleep function
            // Would need to write register directly if needed
        }
    } else {
        Serial.println("Waking sensors...");
        // Sensors wake automatically when accessed
    }
}

void SensorManager::enableSensor(const char* sensorName, bool enable) {
    Serial.printf("%s sensor: %s\n", enable ? "Enabling" : "Disabling", sensorName);
    // TODO: Implement per-sensor enable/disable
}


// ============================================================================
// Health Sensor (MAX30102) - Heart Rate & SpO2
// ============================================================================

void SensorManager::updateHealth() {
    if (!status.healthSensorAvailable || !healthSensor) {
        healthData.fingerDetected = false;
        healthData.validReading = false;
        return;
    }

    // Read IR and Red values
    healthData.ir = healthSensor->getIR();
    healthData.red = healthSensor->getRed();

    // Check if finger is detected (IR value above threshold)
    if (healthData.ir < 50000) {
        healthData.fingerDetected = false;
        healthData.validReading = false;
        healthData.heartRate = 0;
        healthData.spo2 = 0;
        healthData.signalQuality = 0;
        return;
    }

    healthData.fingerDetected = true;

    // Calculate signal quality (0-100)
    // Based on IR signal strength
    long signalStrength = healthData.ir;
    if (signalStrength > 100000) {
        healthData.signalQuality = 100;
    } else if (signalStrength > 80000) {
        healthData.signalQuality = 80;
    } else if (signalStrength > 60000) {
        healthData.signalQuality = 60;
    } else {
        healthData.signalQuality = 40;
    }

    // Simple heart rate detection using peak detection
    static long lastPeakTime = 0;
    static long lastIR = 0;
    static bool rising = false;
    static int beatCount = 0;
    static long firstBeatTime = 0;

    // Detect rising edge
    if (healthData.ir > lastIR + 1000 && !rising) {
        rising = true;
    }

    // Detect peak (rising edge ends)
    if (rising && healthData.ir < lastIR) {
        long currentTime = millis();
        
        if (lastPeakTime > 0) {
            long interval = currentTime - lastPeakTime;
            
            // Valid heart rate range: 40-200 BPM
            if (interval > 300 && interval < 1500) {
                int bpm = 60000 / interval;
                
                // Average over multiple beats for stability
                if (beatCount == 0) {
                    firstBeatTime = currentTime;
                }
                beatCount++;
                
                if (beatCount >= 3) {
                    long totalTime = currentTime - firstBeatTime;
                    healthData.heartRate = (beatCount * 60000) / totalTime;
                    healthData.validReading = true;
                    
                    // Reset for next average
                    if (beatCount >= 5) {
                        beatCount = 0;
                    }
                }
            }
        }
        
        lastPeakTime = currentTime;
        rising = false;
    }

    lastIR = healthData.ir;

    // SpO2 calculation (simplified - real algorithm is complex)
    // Ratio of Red to IR correlates with SpO2
    if (healthData.red > 0 && healthData.ir > 0) {
        float ratio = (float)healthData.red / (float)healthData.ir;
        
        // Empirical formula (simplified)
        // Real SpO2 requires complex calibration
        healthData.spo2 = constrain(110 - (int)(ratio * 25), 85, 100);
    } else {
        healthData.spo2 = 0;
    }

    healthData.timestamp = millis();
}

int SensorManager::getHeartRate() {
    return healthData.heartRate;
}

int SensorManager::getSpO2() {
    return healthData.spo2;
}

bool SensorManager::getFingerDetected() {
    return healthData.fingerDetected;
}

uint8_t SensorManager::getSignalQuality() {
    return healthData.signalQuality;
}

HealthData SensorManager::getHealthData() {
    return healthData;
}
