/*
 * Serial Output Example - Arduino IDE for ESP32
 * IoT Course - Spring 2026
 *
 * Demonstrates Serial (UART) output, chip info, and timing functions.
 * All output is visible in the QEMU console.
 *
 * To run in QEMU:
 *   cd esp32-qemu
 *   ./run-arduino-example.sh 02-serial-output
 */

void setup() {
    Serial.begin(115200);

    Serial.println();
    Serial.println("==========================================");
    Serial.println("  Arduino Serial Output (ESP32 QEMU)");
    Serial.println("  IoT Course - Spring 2026");
    Serial.println("==========================================");
    Serial.println();

    // Print chip information
    Serial.printf("ESP32 Chip Model:    %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision:       %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency:       %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size:          %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free Heap:           %d bytes\n", ESP.getFreeHeap());
    Serial.printf("SDK Version:         %s\n", ESP.getSdkVersion());
    Serial.println();

    Serial.println("Starting counter (10 iterations)...");
    Serial.println();
}

int counter = 0;

void loop() {
    unsigned long uptime = millis();

    Serial.printf("[%8lu ms] Counter: %d", uptime, counter);

    // Demonstrate some basic math / logic
    if (counter % 2 == 0) {
        Serial.print("  (even)");
    } else {
        Serial.print("  (odd)");
    }

    // Simulated sensor reading using analogRead noise
    int simulated = random(200, 300);
    float voltage = simulated * (3.3 / 4095.0);
    Serial.printf("  | Simulated ADC: %d (%.2fV)", simulated, voltage);

    Serial.println();

    counter++;
    delay(1000);

    if (counter >= 10) {
        Serial.println();
        Serial.println("==========================================");
        Serial.println("  Demo complete!");
        Serial.println("  Press Ctrl+A then X to exit QEMU");
        Serial.println("==========================================");

        // Stop looping
        while (true) {
            delay(1000);
        }
    }
}
