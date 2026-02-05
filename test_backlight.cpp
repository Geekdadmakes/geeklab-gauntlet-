// Simple Backlight Test
// This should make the display backlight turn on at full brightness

#include <Arduino.h>

#define BACKLIGHT_PIN 21  // GPIO 21 for backlight

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== BACKLIGHT TEST ===");
  Serial.println("Turning on backlight...");

  // Set backlight pin as output
  pinMode(BACKLIGHT_PIN, OUTPUT);

  // Turn backlight ON (full brightness)
  digitalWrite(BACKLIGHT_PIN, HIGH);
  analogWrite(BACKLIGHT_PIN, 255);

  Serial.println("Backlight should be ON now!");
  Serial.println("If display is still black, check LED pin connection.");
}

void loop() {
  // Blink to show program is running
  delay(1000);
  Serial.println("Still running... backlight on");
}
