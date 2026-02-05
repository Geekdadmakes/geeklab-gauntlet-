#ifndef TOUCH_MANAGER_H
#define TOUCH_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "config.h"

// Swipe directions
enum SwipeDirection {
    SWIPE_NONE = 0,
    SWIPE_UP = 1,
    SWIPE_DOWN = 2,
    SWIPE_LEFT = 3,
    SWIPE_RIGHT = 4
};

// Touch point structure
struct TouchPoint {
    int x;
    int y;
    bool pressed;
    unsigned long timestamp;
};

class TouchManager {
public:
    TouchManager();
    bool begin();
    void update();

    // Touch state
    bool isTouched();
    TouchPoint getPoint();
    TouchPoint getRawPoint();

    // Calibrated coordinates (0-SCREEN_WIDTH, 0-SCREEN_HEIGHT)
    void getCalibrated(int& x, int& y);

    // UI helpers
    bool isInBounds(int x, int y, int w, int h);
    bool isButtonPressed(int btnX, int btnY, int btnW, int btnH);

    // Gesture detection
    SwipeDirection detectSwipe();
    bool isLongPress(unsigned long duration = 1000);

    // Calibration
    void setCalibration(int xMin, int xMax, int yMin, int yMax);
    void resetCalibration();

    // Settings
    void setDebounceTime(unsigned long ms);
    void setPressureThreshold(int threshold);

private:
    XPT2046_Touchscreen* touch;
    SPIClass* touchSpi;  // Separate SPI bus for touch

    // Touch state tracking
    TouchPoint currentPoint;
    TouchPoint lastPoint;
    TouchPoint pressStartPoint;
    unsigned long lastTouchTime;
    unsigned long pressStartTime;
    bool wasTouched;

    // Calibration values
    int calXMin;
    int calXMax;
    int calYMin;
    int calYMax;

    // Settings
    unsigned long debounceTime;
    int pressureThreshold;

    // Gesture detection
    int swipeThreshold;
    unsigned long swipeTimeout;
    unsigned long lastSwipeTime;  // Debounce swipe gestures

    // Helper functions
    int mapToScreen(int raw, int rawMin, int rawMax, int screenSize);
    bool isValidTouch(TS_Point p);
};

extern TouchManager touchMgr;

#endif // TOUCH_MANAGER_H
