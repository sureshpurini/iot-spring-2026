/*
 * Arduino GPIO Read Example for ESP32
 * IoT Course - Spring 2026
 *
 * This example demonstrates GPIO input reading
 * with a button connected to GPIO 4.
 */

#define BUTTON_PIN 4  // Button input pin
#define LED_PIN 2     // Built-in LED

int buttonState = 0;
int lastButtonState = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 GPIO Read Example Starting...");

    // Configure pins
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // Use internal pull-up
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    // Read button state (LOW when pressed due to pull-up)
    buttonState = digitalRead(BUTTON_PIN);

    // Detect state change
    if (buttonState != lastButtonState) {
        if (buttonState == LOW) {
            Serial.println("Button PRESSED");
            digitalWrite(LED_PIN, HIGH);
        } else {
            Serial.println("Button RELEASED");
            digitalWrite(LED_PIN, LOW);
        }
        lastButtonState = buttonState;
    }

    delay(50);  // Debounce delay
}
