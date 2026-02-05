#include "touch_manager.h"

TouchManager touchMgr;

TouchManager::TouchManager() :
    touch(nullptr),
    touchSpi(nullptr),
    lastTouchTime(0),
    pressStartTime(0),
    wasTouched(false),
    debounceTime(50),
    pressureThreshold(200),
    swipeThreshold(50),
    swipeTimeout(500),
    lastSwipeTime(0)
{
    // Initialize calibration values from config
    calXMin = TOUCH_CAL_X_MIN;
    calXMax = TOUCH_CAL_X_MAX;
    calYMin = TOUCH_CAL_Y_MIN;
    calYMax = TOUCH_CAL_Y_MAX;

    // Initialize touch points
    currentPoint = {0, 0, false, 0};
    lastPoint = {0, 0, false, 0};
    pressStartPoint = {0, 0, false, 0};
}

bool TouchManager::begin() {
    // Initialize touch on separate SPI bus (FSPI on ESP32-S3)
    Serial.println("Initializing touch screen on separate SPI bus...");

    // Setup touch SPI with safe GPIO pins (avoiding flash conflicts)
    touchSpi = new SPIClass(FSPI);
    touchSpi->begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);

    Serial.println("Touch SPI bus initialized");

    // Initialize XPT2046 touchscreen with custom SPI
    touch = new XPT2046_Touchscreen(TOUCH_CS, TOUCH_IRQ);
    touch->setRotation(1);  // Match display rotation (landscape)

    if (!touch->begin(*touchSpi)) {
        Serial.println("ERROR: Touch screen initialization failed!");
        return false;
    }

    Serial.println("Touch screen initialized successfully");
    Serial.printf("Calibration: X[%d-%d], Y[%d-%d]\n",
                  calXMin, calXMax, calYMin, calYMax);

    return true;
}

void TouchManager::update() {
    if (!touch) return;

    bool touched = touch->touched();
    unsigned long now = millis();

    // Debounce
    if (touched && (now - lastTouchTime < debounceTime)) {
        return;
    }

    if (touched) {
        TS_Point p = touch->getPoint();

        // Check if valid touch (pressure threshold)
        if (isValidTouch(p)) {
            // Map raw coordinates to screen coordinates
            int x = mapToScreen(p.x, calXMin, calXMax, SCREEN_WIDTH);
            int y = mapToScreen(p.y, calYMin, calYMax, SCREEN_HEIGHT);

            // Clamp to screen bounds
            x = constrain(x, 0, SCREEN_WIDTH - 1);
            y = constrain(y, 0, SCREEN_HEIGHT - 1);

            // Update current point
            lastPoint = currentPoint;
            currentPoint.x = x;
            currentPoint.y = y;
            currentPoint.pressed = true;
            currentPoint.timestamp = now;

            // Track press start for long press detection
            if (!wasTouched) {
                pressStartPoint = currentPoint;
                pressStartTime = now;
            }

            lastTouchTime = now;
            wasTouched = true;

#ifdef DEBUG_TOUCH
            Serial.printf("Touch: Raw(%d,%d) -> Screen(%d,%d)\n",
                         p.x, p.y, x, y);
#endif
        }
    } else {
        if (wasTouched) {
            // Touch released - keep currentPoint data for swipe detection
            // but mark as not pressed
            currentPoint.pressed = false;
            wasTouched = false;

            // Don't clear pressStartPoint here - it's needed for swipe detection
            // It will be cleared after swipe is detected
        }
    }
}

bool TouchManager::isTouched() {
    return currentPoint.pressed;
}

TouchPoint TouchManager::getPoint() {
    return currentPoint;
}

TouchPoint TouchManager::getRawPoint() {
    if (!touch) return {0, 0, false, 0};

    TS_Point p = touch->getPoint();
    return {(int)p.x, (int)p.y, touch->touched(), millis()};
}

void TouchManager::getCalibrated(int& x, int& y) {
    x = currentPoint.x;
    y = currentPoint.y;
}

bool TouchManager::isInBounds(int x, int y, int w, int h) {
    if (!currentPoint.pressed) return false;

    return (currentPoint.x >= x && currentPoint.x <= x + w &&
            currentPoint.y >= y && currentPoint.y <= y + h);
}

bool TouchManager::isButtonPressed(int btnX, int btnY, int btnW, int btnH) {
    return isInBounds(btnX, btnY, btnW, btnH);
}

SwipeDirection TouchManager::detectSwipe() {
    if (!pressStartPoint.pressed || currentPoint.pressed) {
        return SWIPE_NONE;
    }

    // Debounce: prevent multiple swipe detections within 500ms
    unsigned long currentTime = millis();
    if (currentTime - lastSwipeTime < 500) {
        return SWIPE_NONE;
    }

    unsigned long swipeDuration = currentPoint.timestamp - pressStartPoint.timestamp;
    if (swipeDuration > swipeTimeout) {
        // Reset press start point if timeout exceeded
        pressStartPoint.pressed = false;
        return SWIPE_NONE;
    }

    int dx = currentPoint.x - pressStartPoint.x;
    int dy = currentPoint.y - pressStartPoint.y;

    // Check if movement is significant enough
    if (abs(dx) < swipeThreshold && abs(dy) < swipeThreshold) {
        // Not a swipe, just a tap - reset press start point
        pressStartPoint.pressed = false;
        return SWIPE_NONE;
    }

    // Determine primary direction
    SwipeDirection direction = SWIPE_NONE;
    if (abs(dx) > abs(dy)) {
        // Horizontal swipe
        direction = (dx > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
    } else {
        // Vertical swipe
        direction = (dy > 0) ? SWIPE_DOWN : SWIPE_UP;
    }

    // Update last swipe time and clear press start to prevent repeated detection
    if (direction != SWIPE_NONE) {
        lastSwipeTime = currentTime;
        pressStartPoint.pressed = false;  // Clear so we don't detect this swipe again
    }

    return direction;
}

bool TouchManager::isLongPress(unsigned long duration) {
    if (!currentPoint.pressed || !wasTouched) {
        return false;
    }

    unsigned long pressDuration = millis() - pressStartTime;

    // Check if still in roughly same position
    int dx = abs(currentPoint.x - pressStartPoint.x);
    int dy = abs(currentPoint.y - pressStartPoint.y);

    return (pressDuration >= duration && dx < 10 && dy < 10);
}

void TouchManager::setCalibration(int xMin, int xMax, int yMin, int yMax) {
    calXMin = xMin;
    calXMax = xMax;
    calYMin = yMin;
    calYMax = yMax;

    Serial.printf("Touch calibration updated: X[%d-%d], Y[%d-%d]\n",
                  calXMin, calXMax, calYMin, calYMax);
}

void TouchManager::resetCalibration() {
    calXMin = TOUCH_CAL_X_MIN;
    calXMax = TOUCH_CAL_X_MAX;
    calYMin = TOUCH_CAL_Y_MIN;
    calYMax = TOUCH_CAL_Y_MAX;
}

void TouchManager::setDebounceTime(unsigned long ms) {
    debounceTime = ms;
}

void TouchManager::setPressureThreshold(int threshold) {
    pressureThreshold = threshold;
}

int TouchManager::mapToScreen(int raw, int rawMin, int rawMax, int screenSize) {
    return map(raw, rawMin, rawMax, 0, screenSize);
}

bool TouchManager::isValidTouch(TS_Point p) {
    // XPT2046 uses z as pressure value
    // Higher z = more pressure
    // Typical valid range: 200-4000
    return (p.z > pressureThreshold && p.z < 4000);
}
