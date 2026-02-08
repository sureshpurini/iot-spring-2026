/*
 * ESP-IDF Low Power Periodic Sensor Reading Example
 * IoT Course - Spring 2026
 *
 * This example demonstrates efficient power management using ESP32's
 * light sleep mode with timer-based wake-up for periodic sensor readings.
 *
 * Key Concepts:
 * - Light sleep preserves RAM state (unlike deep sleep)
 * - Timer wake-up for periodic operations
 * - Much lower power consumption than busy-waiting with delay()
 *
 * Power Comparison:
 * - Active mode:   ~80mA
 * - delay() wait:  ~80mA (CPU still running!)
 * - Light sleep:   ~0.8mA (100x more efficient)
 * - Deep sleep:    ~10µA (but loses RAM state)
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#define SENSOR_PIN GPIO_NUM_34      // ADC pin for sensor (simulated)
#define SLEEP_DURATION_US 10000000  // 10 seconds in microseconds
#define SLEEP_DURATION_SEC 10

static const char *TAG = "LOW_POWER";

// Simulated sensor reading counter (for demo purposes)
static int reading_count = 0;

/**
 * Simulate reading a sensor value
 * In a real application, this would read from ADC, I2C sensor, etc.
 */
static int read_sensor(void)
{
    // Simulate sensor reading with a counter
    // In real code: return adc1_get_raw(ADC1_CHANNEL_6);
    return (reading_count * 17 + 42) % 4096;  // Simulated ADC value 0-4095
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Low Power Periodic Sensor Reading Demo");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Sleep duration: %d seconds", SLEEP_DURATION_SEC);
    ESP_LOGI(TAG, "Power mode: Light Sleep (RAM preserved)");
    ESP_LOGI(TAG, "");

    // Configure timer wake-up
    esp_err_t ret = esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure timer wake-up: %s", esp_err_to_name(ret));
        // Fall back to regular delay if sleep not supported (e.g., in QEMU)
        ESP_LOGW(TAG, "Falling back to vTaskDelay mode (QEMU compatible)");

        while (1) {
            reading_count++;
            int sensor_value = read_sensor();
            ESP_LOGI(TAG, "[Reading #%d] Sensor value: %d", reading_count, sensor_value);
            ESP_LOGI(TAG, "  (In real hardware, would sleep here for %ds)", SLEEP_DURATION_SEC);
            ESP_LOGI(TAG, "  Using vTaskDelay instead (QEMU mode)...");

            // Use shorter delay in QEMU for demo purposes
            vTaskDelay(pdMS_TO_TICKS(3000));  // 3 seconds for demo

            ESP_LOGI(TAG, "  Woke up! Ready for next reading.");
            ESP_LOGI(TAG, "");
        }
    }

    ESP_LOGI(TAG, "Timer wake-up configured successfully");
    ESP_LOGI(TAG, "");

    // Main sensor reading loop with sleep
    while (1) {
        reading_count++;

        // Read the sensor
        int sensor_value = read_sensor();

        // Log the reading with timestamp
        int64_t timestamp = esp_timer_get_time() / 1000000;  // Convert to seconds
        ESP_LOGI(TAG, "[%llds] Reading #%d: Sensor = %d",
                 timestamp, reading_count, sensor_value);

        // Process the data (in real app: send to cloud, store, etc.)
        ESP_LOGI(TAG, "  Processing data...");

        // Enter light sleep
        ESP_LOGI(TAG, "  Going to light sleep for %d seconds...", SLEEP_DURATION_SEC);
        ESP_LOGI(TAG, "  (Power drops from ~80mA to ~0.8mA)");

        // Actually enter light sleep
        // Note: In QEMU, this may not work as expected
        esp_light_sleep_start();

        // Code continues here after wake-up!
        // This is the key advantage of light sleep over deep sleep
        ESP_LOGI(TAG, "  Woke up! RAM state preserved.");
        ESP_LOGI(TAG, "");
    }
}
