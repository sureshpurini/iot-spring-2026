/**
 * GPIO and Timer Example for ESP32 QEMU
 * IoT Course - Spring 2026
 *
 * This example demonstrates:
 * - GPIO output configuration
 * - Hardware timer usage
 * - Interrupt handling
 * - FreeRTOS queues for ISR communication
 *
 * Note: In QEMU, GPIO states are simulated but not connected
 * to external peripherals. You'll see the state changes in logs.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"

static const char *TAG = "GPIO_TIMER";

// LED GPIO (simulated in QEMU)
#define LED_GPIO    2

// Timer configuration
#define TIMER_RESOLUTION_HZ   1000000  // 1MHz, 1us per tick
#define TIMER_ALARM_PERIOD_US 500000   // 500ms

// Queue for timer events
static QueueHandle_t timer_queue = NULL;

// Timer callback
static bool IRAM_ATTR timer_alarm_callback(gptimer_handle_t timer,
                                           const gptimer_alarm_event_data_t *edata,
                                           void *user_ctx)
{
    BaseType_t high_task_wakeup = pdFALSE;
    uint64_t timer_count = edata->count_value;

    // Send timer count to queue
    xQueueSendFromISR(timer_queue, &timer_count, &high_task_wakeup);

    return high_task_wakeup == pdTRUE;
}

// LED blink task
static void led_task(void *arg)
{
    static int led_state = 0;
    uint64_t timer_count;

    while (1) {
        // Wait for timer event
        if (xQueueReceive(timer_queue, &timer_count, portMAX_DELAY)) {
            // Toggle LED
            led_state = !led_state;
            gpio_set_level(LED_GPIO, led_state);

            ESP_LOGI(TAG, "LED %s (timer count: %llu)",
                     led_state ? "ON" : "OFF", timer_count);
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "   GPIO & Timer Example");
    ESP_LOGI(TAG, "   Running in QEMU");
    ESP_LOGI(TAG, "========================================");

    // Create timer event queue
    timer_queue = xQueueCreate(10, sizeof(uint64_t));

    // Configure LED GPIO
    ESP_LOGI(TAG, "Configuring GPIO %d as output", LED_GPIO);
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // Configure timer
    ESP_LOGI(TAG, "Configuring hardware timer");
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // Set up alarm
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = TIMER_ALARM_PERIOD_US,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // Register callback
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_alarm_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // Enable and start timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
    ESP_LOGI(TAG, "Timer started with %d us period", TIMER_ALARM_PERIOD_US);

    // Create LED task
    xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);

    ESP_LOGI(TAG, "System running. Press Ctrl+A then X to exit QEMU.");
}
