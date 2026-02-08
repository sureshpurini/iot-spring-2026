/*
 * ESP-IDF GPIO Read Example for ESP32
 * IoT Course - Spring 2026
 *
 * This example demonstrates GPIO input reading
 * using the ESP-IDF framework with interrupt handling.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_PIN GPIO_NUM_4  // Button input pin
#define LED_PIN GPIO_NUM_2     // Built-in LED

static const char *TAG = "GPIO_READ";
static QueueHandle_t gpio_evt_queue = NULL;

// Interrupt service routine
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Task to handle GPIO events
static void gpio_task(void *arg)
{
    uint32_t gpio_num;
    int led_state = 0;

    while (1) {
        if (xQueueReceive(gpio_evt_queue, &gpio_num, portMAX_DELAY)) {
            int button_state = gpio_get_level(gpio_num);
            ESP_LOGI(TAG, "Button %s", button_state ? "RELEASED" : "PRESSED");

            if (!button_state) {  // Button pressed (active low)
                led_state = !led_state;
                gpio_set_level(LED_PIN, led_state);
                ESP_LOGI(TAG, "LED toggled to %s", led_state ? "ON" : "OFF");
            }
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 GPIO Read Example Starting...");

    // Configure LED output
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_conf);

    // Configure button input with interrupt
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&btn_conf);

    // Create event queue and start task
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

    // Install ISR service and add handler
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, gpio_isr_handler, (void *)BUTTON_PIN);

    ESP_LOGI(TAG, "GPIO configured. Press button to toggle LED.");
}
