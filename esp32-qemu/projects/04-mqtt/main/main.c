/**
 * MQTT Pub/Sub Example for ESP32 QEMU
 * IoT Course - Spring 2026
 *
 * Demonstrates:
 * - Ethernet networking in QEMU (OpenCores open_eth via slirp)
 * - MQTT publish/subscribe using esp-mqtt
 * - Publishing simulated sensor data on a schedule
 * - Subscribing to command topics and reacting to messages
 * - QoS levels and last will testament (LWT)
 *
 * Network architecture:
 *   ESP32 (QEMU guest)  --[slirp]--> Docker host (10.0.2.2)
 *   Docker host         --[bridge]-> Mosquitto container (:1883)
 *
 * Topics:
 *   esp32/sensors/temperature  - ESP32 publishes sensor readings here
 *   esp32/sensors/humidity     - ESP32 publishes humidity readings here
 *   esp32/commands             - ESP32 subscribes for incoming commands
 *   esp32/status               - ESP32 publishes online/offline status (LWT)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_eth.h"

#include "mqtt_client.h"

static const char *TAG = "mqtt-demo";

/* ----------------------------------------------------------------
 * Event bits
 * ---------------------------------------------------------------- */
static EventGroupHandle_t eth_event_group;
#define ETH_CONNECTED_BIT   BIT0

static EventGroupHandle_t mqtt_event_group;
#define MQTT_CONNECTED_BIT  BIT0

/* ----------------------------------------------------------------
 * MQTT configuration
 * In QEMU slirp, host is at 10.0.2.2.
 * Mosquitto Docker container exposes port 1883 on the host.
 * ---------------------------------------------------------------- */
#define MQTT_BROKER_URI      "mqtt://10.0.2.2:1883"
#define MQTT_BROKER_FALLBACK "mqtt://test.mosquitto.org:1883"

#define TOPIC_TEMPERATURE    "esp32/sensors/temperature"
#define TOPIC_HUMIDITY       "esp32/sensors/humidity"
#define TOPIC_COMMANDS       "esp32/commands"
#define TOPIC_STATUS         "esp32/status"

#define CLIENT_ID            "esp32-qemu-01"

static esp_mqtt_client_handle_t mqtt_client = NULL;
static int publish_count = 0;

/* ----------------------------------------------------------------
 * Simulated sensor readings
 * ---------------------------------------------------------------- */
static float get_simulated_temperature(void)
{
    return 20.0f + (float)(esp_random() % 100) / 10.0f;
}

static float get_simulated_humidity(void)
{
    return 40.0f + (float)(esp_random() % 300) / 10.0f;
}

/* ----------------------------------------------------------------
 * MQTT event handler
 * ---------------------------------------------------------------- */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch (event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected to broker");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);

        /* Publish online status */
        esp_mqtt_client_publish(mqtt_client, TOPIC_STATUS, "online", 0, 1, 1);

        /* Subscribe to command topic (QoS 1 for reliable delivery) */
        int msg_id = esp_mqtt_client_subscribe(mqtt_client, TOPIC_COMMANDS, 1);
        ESP_LOGI(TAG, "Subscribed to %s, msg_id=%d", TOPIC_COMMANDS, msg_id);

        /* Also subscribe to own sensor topics to see the echo */
        esp_mqtt_client_subscribe(mqtt_client, "esp32/sensors/#", 0);
        ESP_LOGI(TAG, "Subscribed to esp32/sensors/# (wildcard)");
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT disconnected");
        xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT subscribed, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT message published, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "========================================");
        ESP_LOGI(TAG, "MQTT message received!");
        ESP_LOGI(TAG, "  Topic:   %.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "  Payload: %.*s", event->data_len, event->data);
        ESP_LOGI(TAG, "========================================");

        /* React to commands */
        if (event->topic_len > 0 &&
            strncmp(event->topic, TOPIC_COMMANDS, event->topic_len) == 0) {
            if (strncmp(event->data, "toggle_led", event->data_len) == 0) {
                ESP_LOGI(TAG, "Command: toggle_led -> LED toggled (simulated)");
            } else if (strncmp(event->data, "get_status", event->data_len) == 0) {
                ESP_LOGI(TAG, "Command: get_status -> publishing status");
                char status_msg[128];
                snprintf(status_msg, sizeof(status_msg),
                         "{\"uptime_s\":%d,\"publish_count\":%d}",
                         (int)(xTaskGetTickCount() / configTICK_RATE_HZ),
                         publish_count);
                esp_mqtt_client_publish(mqtt_client, TOPIC_STATUS, status_msg, 0, 0, 0);
            } else {
                ESP_LOGW(TAG, "Unknown command: %.*s", event->data_len, event->data);
            }
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT error occurred");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "  Transport error: %s",
                     esp_err_to_name(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGI(TAG, "MQTT event: %d", (int)event_id);
        break;
    }
}

/* ----------------------------------------------------------------
 * Ethernet event handlers (same pattern as 03-rest-api)
 * ---------------------------------------------------------------- */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Ethernet link up");
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "Ethernet link down");
        xEventGroupClearBits(eth_event_group, ETH_CONNECTED_BIT);
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet stopped");
        break;
    default:
        break;
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    if (event_id == IP_EVENT_ETH_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Gateway:        " IPSTR, IP2STR(&event->ip_info.gw));
        ESP_LOGI(TAG, "Netmask:        " IPSTR, IP2STR(&event->ip_info.netmask));
        xEventGroupSetBits(eth_event_group, ETH_CONNECTED_BIT);
    }
}

/* ----------------------------------------------------------------
 * Initialize Ethernet for QEMU (OpenCores open_eth)
 * ---------------------------------------------------------------- */
static esp_eth_handle_t eth_handle = NULL;

static void init_ethernet(void)
{
    eth_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);

    /* OpenCores Ethernet MAC — the virtual NIC provided by QEMU */
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_openeth(&mac_config);

    /* DP83848 PHY — emulated by QEMU's open_eth */
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 0;
    phy_config.reset_gpio_num = -1;
    phy_config.autonego_timeout_ms = 100;
    esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));

    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID,
                                               &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP,
                                               &ip_event_handler, NULL));

    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    ESP_LOGI(TAG, "Waiting for IP address from DHCP...");
}

/* ----------------------------------------------------------------
 * Initialize MQTT client
 * ---------------------------------------------------------------- */
static void init_mqtt(void)
{
    mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.client_id = CLIENT_ID,
        /* Last Will Testament: broker publishes this if we disconnect unexpectedly */
        .session.last_will = {
            .topic = TOPIC_STATUS,
            .msg = "offline",
            .msg_len = 7,
            .qos = 1,
            .retain = 1,
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    ESP_LOGI(TAG, "MQTT client started, connecting to %s ...", MQTT_BROKER_URI);
}

/* ----------------------------------------------------------------
 * Sensor publishing task
 * ---------------------------------------------------------------- */
static void sensor_publish_task(void *pvParameters)
{
    /* Wait until MQTT is connected */
    xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Starting sensor publish loop");
    ESP_LOGI(TAG, "  Publishing to: %s, %s", TOPIC_TEMPERATURE, TOPIC_HUMIDITY);
    ESP_LOGI(TAG, "  Interval: 5 seconds");
    ESP_LOGI(TAG, "  Total readings: 10");
    ESP_LOGI(TAG, "========================================");

    for (int i = 0; i < 10; i++) {
        /* Check if still connected */
        EventBits_t bits = xEventGroupGetBits(mqtt_event_group);
        if (!(bits & MQTT_CONNECTED_BIT)) {
            ESP_LOGW(TAG, "MQTT disconnected, waiting to reconnect...");
            xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT,
                                pdFALSE, pdTRUE, pdMS_TO_TICKS(30000));
        }

        float temp = get_simulated_temperature();
        float humidity = get_simulated_humidity();

        /* Publish temperature as JSON */
        char temp_msg[128];
        snprintf(temp_msg, sizeof(temp_msg),
                 "{\"device\":\"%s\",\"value\":%.1f,\"unit\":\"C\",\"reading\":%d}",
                 CLIENT_ID, temp, i + 1);

        int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_TEMPERATURE,
                                             temp_msg, 0, 1, 0);
        ESP_LOGI(TAG, "[%d/10] Published temperature=%.1f C (msg_id=%d)",
                 i + 1, temp, msg_id);

        /* Publish humidity as JSON */
        char hum_msg[128];
        snprintf(hum_msg, sizeof(hum_msg),
                 "{\"device\":\"%s\",\"value\":%.1f,\"unit\":\"%%\",\"reading\":%d}",
                 CLIENT_ID, humidity, i + 1);

        msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_HUMIDITY,
                                         hum_msg, 0, 1, 0);
        ESP_LOGI(TAG, "[%d/10] Published humidity=%.1f %% (msg_id=%d)",
                 i + 1, humidity, msg_id);

        publish_count += 2;

        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    /* Publish final status */
    char final_msg[128];
    snprintf(final_msg, sizeof(final_msg),
             "{\"status\":\"complete\",\"total_published\":%d}", publish_count);
    esp_mqtt_client_publish(mqtt_client, TOPIC_STATUS, final_msg, 0, 1, 0);

    printf("\n");
    printf("==========================================\n");
    printf("  MQTT Demo complete!\n");
    printf("  Published %d messages total\n", publish_count);
    printf("  Press Ctrl+A then X to exit QEMU\n");
    printf("==========================================\n");

    vTaskDelete(NULL);
}

/* ----------------------------------------------------------------
 * Main application
 * ---------------------------------------------------------------- */
void app_main(void)
{
    printf("\n");
    printf("==========================================\n");
    printf("  ESP32 MQTT Pub/Sub Demo (QEMU)\n");
    printf("  IoT Course - Spring 2026\n");
    printf("==========================================\n\n");

    /* Step 1: Initialize Ethernet and wait for IP */
    init_ethernet();

    EventBits_t bits = xEventGroupWaitBits(eth_event_group,
                                           ETH_CONNECTED_BIT,
                                           pdFALSE, pdTRUE,
                                           pdMS_TO_TICKS(30000));
    if (!(bits & ETH_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Failed to get IP address within 30 seconds!");
        ESP_LOGE(TAG, "Make sure QEMU was started with: -nic user,model=open_eth");
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(2000));

    /* Step 2: Initialize MQTT and connect to broker */
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Connecting to MQTT broker...");
    ESP_LOGI(TAG, "  Broker:    %s", MQTT_BROKER_URI);
    ESP_LOGI(TAG, "  Client ID: %s", CLIENT_ID);
    ESP_LOGI(TAG, "  LWT topic: %s", TOPIC_STATUS);
    ESP_LOGI(TAG, "========================================");

    init_mqtt();

    /* Wait for MQTT connection */
    bits = xEventGroupWaitBits(mqtt_event_group,
                               MQTT_CONNECTED_BIT,
                               pdFALSE, pdTRUE,
                               pdMS_TO_TICKS(30000));
    if (!(bits & MQTT_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Failed to connect to MQTT broker within 30 seconds!");
        ESP_LOGW(TAG, "Check that Mosquitto is running: docker compose up -d mqtt-broker");
        return;
    }

    /* Step 3: Launch sensor publishing task */
    xTaskCreate(sensor_publish_task, "sensor_pub", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "MQTT demo running.");
    ESP_LOGI(TAG, "To send a command from your host:");
    ESP_LOGI(TAG, "  mosquitto_pub -h localhost -t esp32/commands -m toggle_led");
    ESP_LOGI(TAG, "  mosquitto_pub -h localhost -t esp32/commands -m get_status");
    ESP_LOGI(TAG, "========================================");
}
