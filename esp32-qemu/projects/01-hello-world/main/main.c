/**
 * Hello World Example for ESP32 QEMU
 * IoT Course - Spring 2026
 *
 * This simple example demonstrates:
 * - Basic ESP-IDF application structure
 * - UART output (visible in QEMU console)
 * - FreeRTOS task creation
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

void app_main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("   Hello from ESP32 running in QEMU!\n");
    printf("   IoT Course - Spring 2026\n");
    printf("========================================\n\n");

    // Print chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("ESP32 Chip Information:\n");
    printf("  - Cores: %d\n", chip_info.cores);
    printf("  - Features: %s%s%s\n",
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi " : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT " : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE " : "");
    printf("  - Revision: %d\n", chip_info.revision);

    // Simple counting loop
    int count = 0;
    while (1) {
        printf("Counter: %d\n", count++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait 1 second

        // Stop after 10 iterations (for demo purposes)
        if (count >= 10) {
            printf("\nDemo complete! Press Ctrl+A then X to exit QEMU.\n");
            break;
        }
    }

    // Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
