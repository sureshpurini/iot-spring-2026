/*
 * Arduino Blink Example for ESP32
 * IoT Course - Spring 2026
 *
 * This example demonstrates basic GPIO output control
 * by blinking the built-in LED on GPIO 2.
 */

#define LED_PIN 2  // Built-in LED on most ESP32 boards

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    Serial.println("ESP32 Blink Example Starting...");

    // Configure LED pin as output
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    // Turn LED ON
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
    delay(1000);  // Wait 1 second

    // Turn LED OFF
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
    delay(1000);  // Wait 1 second
}
