/*
 * ESP-IDF Blink Example for ESP32
 * IoT Course - Spring 2026
 *
 * This example demonstrates basic GPIO output control
 * using the ESP-IDF framework with FreeRTOS.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_PIN GPIO_NUM_2  // Built-in LED on most ESP32 boards
#define BLINK_PERIOD_MS 1000

static const char *TAG = "BLINK";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Blink Example Starting...");

    // Configure GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Blink loop
    int led_state = 0;
    while (1) {
        led_state = !led_state;
        gpio_set_level(LED_PIN, led_state);
        ESP_LOGI(TAG, "LED %s", led_state ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD_MS));
    }
}
