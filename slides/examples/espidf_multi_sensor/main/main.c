/*
 * ESP-IDF Multi-Sensor Example with FreeRTOS Tasks and Timers
 * IoT Course - Spring 2026
 *
 * This example demonstrates two approaches for reading multiple sensors
 * at different rates:
 *
 * 1. FreeRTOS Tasks - Each sensor runs in its own task
 * 2. ESP Timers - Hardware timers call callbacks at precise intervals
 *
 * Problem Scenario:
 * - Sensor A (Accelerometer): Read every 100ms (simulated, scaled for demo)
 * - Sensor B (Temperature): Read every 200ms
 * - Both must run independently without blocking each other
 *
 * Why Arduino Fails:
 * - delay() blocks ALL code execution
 * - millis() polling is complex and imprecise
 * - No true concurrency - just interleaved polling
 *
 * Why ESP-IDF Excels:
 * - FreeRTOS tasks run concurrently (preemptive multitasking)
 * - ESP timers provide hardware-level precision
 * - Clean, maintainable code that scales to many sensors
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG_MAIN = "MULTI_SENSOR";
static const char *TAG_ACCEL = "ACCEL";
static const char *TAG_TEMP = "TEMP";

// Demo timing (scaled up for visibility in QEMU)
// Real values would be 10ms and 20ms
#define ACCEL_PERIOD_MS  500   // Accelerometer: every 500ms (demo)
#define TEMP_PERIOD_MS   1000  // Temperature: every 1000ms (demo)

// Counters for demonstration
static int accel_count = 0;
static int temp_count = 0;

/* ============================================================
 * APPROACH 1: FreeRTOS Tasks
 * Each sensor runs in its own task with independent timing
 * ============================================================ */

/**
 * Simulated accelerometer reading
 * In real code: read from I2C/SPI accelerometer
 */
static void read_accelerometer(void)
{
    accel_count++;
    // Simulate X, Y, Z acceleration values
    int x = (accel_count * 13) % 2000 - 1000;  // -1000 to 1000
    int y = (accel_count * 17) % 2000 - 1000;
    int z = 1000 + (accel_count * 7) % 100;    // ~1000 (gravity)

    ESP_LOGI(TAG_ACCEL, "[Task] Reading #%d: X=%d, Y=%d, Z=%d",
             accel_count, x, y, z);
}

/**
 * Simulated temperature reading
 * In real code: read from I2C temperature sensor
 */
static void read_temperature(void)
{
    temp_count++;
    // Simulate temperature in Celsius (25.0 to 26.0)
    float temp = 25.0 + (float)(temp_count % 10) / 10.0;

    ESP_LOGI(TAG_TEMP, "[Task] Reading #%d: Temperature = %.1f C",
             temp_count, temp);
}

/**
 * Accelerometer task - runs at ACCEL_PERIOD_MS interval
 */
static void accelerometer_task(void *arg)
{
    ESP_LOGI(TAG_ACCEL, "Accelerometer task started (period: %dms)", ACCEL_PERIOD_MS);

    while (1) {
        read_accelerometer();

        // Non-blocking delay - other tasks can run during this time!
        vTaskDelay(pdMS_TO_TICKS(ACCEL_PERIOD_MS));
    }
}

/**
 * Temperature task - runs at TEMP_PERIOD_MS interval
 */
static void temperature_task(void *arg)
{
    ESP_LOGI(TAG_TEMP, "Temperature task started (period: %dms)", TEMP_PERIOD_MS);

    while (1) {
        read_temperature();

        // Non-blocking delay - accelerometer task runs during this time!
        vTaskDelay(pdMS_TO_TICKS(TEMP_PERIOD_MS));
    }
}

/* ============================================================
 * APPROACH 2: ESP Timers (Hardware Timers)
 * More efficient for simple periodic operations
 * ============================================================ */

static esp_timer_handle_t accel_timer = NULL;
static esp_timer_handle_t temp_timer = NULL;
static int accel_timer_count = 0;
static int temp_timer_count = 0;

/**
 * Timer callback for accelerometer
 * Called from timer interrupt context - keep it short!
 */
static void accel_timer_callback(void *arg)
{
    accel_timer_count++;
    int x = (accel_timer_count * 13) % 2000 - 1000;
    int y = (accel_timer_count * 17) % 2000 - 1000;
    int z = 1000 + (accel_timer_count * 7) % 100;

    ESP_LOGI(TAG_ACCEL, "[Timer] Reading #%d: X=%d, Y=%d, Z=%d",
             accel_timer_count, x, y, z);
}

/**
 * Timer callback for temperature
 */
static void temp_timer_callback(void *arg)
{
    temp_timer_count++;
    float temp = 25.0 + (float)(temp_timer_count % 10) / 10.0;

    ESP_LOGI(TAG_TEMP, "[Timer] Reading #%d: Temperature = %.1f C",
             temp_timer_count, temp);
}

/**
 * Initialize and start the hardware timers
 */
static void setup_timers(void)
{
    ESP_LOGI(TAG_MAIN, "Setting up ESP timers...");

    // Create accelerometer timer
    esp_timer_create_args_t accel_timer_args = {
        .callback = accel_timer_callback,
        .name = "accel_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&accel_timer_args, &accel_timer));

    // Create temperature timer
    esp_timer_create_args_t temp_timer_args = {
        .callback = temp_timer_callback,
        .name = "temp_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&temp_timer_args, &temp_timer));

    // Start timers (period in microseconds)
    ESP_ERROR_CHECK(esp_timer_start_periodic(accel_timer, ACCEL_PERIOD_MS * 1000));
    ESP_ERROR_CHECK(esp_timer_start_periodic(temp_timer, TEMP_PERIOD_MS * 1000));

    ESP_LOGI(TAG_MAIN, "Timers started!");
    ESP_LOGI(TAG_MAIN, "  Accelerometer: every %dms", ACCEL_PERIOD_MS);
    ESP_LOGI(TAG_MAIN, "  Temperature: every %dms", TEMP_PERIOD_MS);
}

/* ============================================================
 * Main Application
 * ============================================================ */

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "================================================");
    ESP_LOGI(TAG_MAIN, "Multi-Sensor Demo: FreeRTOS Tasks vs ESP Timers");
    ESP_LOGI(TAG_MAIN, "================================================");
    ESP_LOGI(TAG_MAIN, "");
    ESP_LOGI(TAG_MAIN, "This demo shows two approaches for reading");
    ESP_LOGI(TAG_MAIN, "multiple sensors at different rates:");
    ESP_LOGI(TAG_MAIN, "");
    ESP_LOGI(TAG_MAIN, "  Accelerometer: every %dms", ACCEL_PERIOD_MS);
    ESP_LOGI(TAG_MAIN, "  Temperature:   every %dms", TEMP_PERIOD_MS);
    ESP_LOGI(TAG_MAIN, "");

    // ==== PHASE 1: Demonstrate FreeRTOS Tasks ====
    ESP_LOGI(TAG_MAIN, "========== PHASE 1: FreeRTOS Tasks ==========");
    ESP_LOGI(TAG_MAIN, "Creating independent tasks for each sensor...");
    ESP_LOGI(TAG_MAIN, "");

    // Create accelerometer task
    xTaskCreate(
        accelerometer_task,    // Task function
        "accel_task",          // Task name (for debugging)
        2048,                  // Stack size (bytes)
        NULL,                  // Parameters
        5,                     // Priority (5 = medium)
        NULL                   // Task handle (not needed)
    );

    // Create temperature task
    xTaskCreate(
        temperature_task,
        "temp_task",
        2048,
        NULL,
        5,
        NULL
    );

    // Let tasks run for a while
    ESP_LOGI(TAG_MAIN, "Tasks are running concurrently...");
    ESP_LOGI(TAG_MAIN, "Watch how readings interleave naturally!");
    ESP_LOGI(TAG_MAIN, "");

    vTaskDelay(pdMS_TO_TICKS(5000));  // Let run for 5 seconds

    // ==== PHASE 2: Demonstrate ESP Timers ====
    ESP_LOGI(TAG_MAIN, "");
    ESP_LOGI(TAG_MAIN, "========== PHASE 2: ESP Hardware Timers ==========");
    ESP_LOGI(TAG_MAIN, "Now using hardware timers instead of tasks...");
    ESP_LOGI(TAG_MAIN, "");

    setup_timers();

    // Main task can do other work or just wait
    ESP_LOGI(TAG_MAIN, "");
    ESP_LOGI(TAG_MAIN, "Main task is free to do other work!");
    ESP_LOGI(TAG_MAIN, "Timer callbacks run automatically in background.");
    ESP_LOGI(TAG_MAIN, "");

    // Keep main task alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // Sleep for 10 seconds
        ESP_LOGI(TAG_MAIN, "Main task still alive. Total readings:");
        ESP_LOGI(TAG_MAIN, "  Tasks  - Accel: %d, Temp: %d", accel_count, temp_count);
        ESP_LOGI(TAG_MAIN, "  Timers - Accel: %d, Temp: %d", accel_timer_count, temp_timer_count);
    }
}
