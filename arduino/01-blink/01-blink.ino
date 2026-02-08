/*
 * Blink Example - Arduino IDE for ESP32
 * IoT Course - Spring 2026
 *
 * Blinks the built-in LED on GPIO 2.
 * Works in QEMU (GPIO state is tracked in emulation).
 *
 * To run in QEMU:
 *   cd esp32-qemu
 *   ./run-arduino-example.sh 01-blink
 */

#define LED_PIN 2

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    Serial.println();
    Serial.println("==========================================");
    Serial.println("  Arduino Blink Example (ESP32 QEMU)");
    Serial.println("  IoT Course - Spring 2026");
    Serial.println("==========================================");
    Serial.println();
}

void loop() {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON  (GPIO 2 = HIGH)");
    delay(1000);

    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF (GPIO 2 = LOW)");
    delay(1000);
}
