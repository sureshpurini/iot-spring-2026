/**
 * REST API Client Example for ESP32 QEMU
 * IoT Course - Spring 2026
 *
 * Demonstrates:
 * - Ethernet networking in QEMU (OpenCores open_eth via slirp)
 * - HTTP GET and POST requests using esp_http_client
 * - JSON payload construction and response parsing
 * - Connecting to a local REST API server
 *
 * Network architecture:
 *   ESP32 (QEMU guest)  --[slirp]--> Docker host (10.0.2.2)
 *   Docker host         --[bridge]-> api-server container (:5000)
 *
 * The QEMU slirp network gives ESP32 IP via DHCP (typically 10.0.2.15).
 * The host is reachable at 10.0.2.2. Since the api-server Docker container
 * exposes port 5000 on the host, ESP32 reaches it at http://10.0.2.2:5000.
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

#include "esp_http_client.h"

static const char *TAG = "rest-api";

/* Event group to signal when we have an IP address */
static EventGroupHandle_t eth_event_group;
#define ETH_CONNECTED_BIT BIT0

/* ----------------------------------------------------------------
 * Server configuration
 * In QEMU slirp, the host machine is always at 10.0.2.2.
 * The Docker api-server exposes port 5000 on the host.
 * ---------------------------------------------------------------- */
#define API_SERVER_HOST "10.0.2.2"
#define API_SERVER_PORT "5000"
#define API_BASE_URL    "http://" API_SERVER_HOST ":" API_SERVER_PORT

/* Fallback: public httpbin.org (requires internet from Docker host) */
#define HTTPBIN_BASE_URL "http://httpbin.org"

/* Buffer for HTTP response */
#define MAX_HTTP_RESPONSE_SIZE 2048
static char response_buffer[MAX_HTTP_RESPONSE_SIZE];
static int response_len;

/* ----------------------------------------------------------------
 * HTTP event handler — collects response data
 * ---------------------------------------------------------------- */
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ON_DATA:
        if (!esp_http_client_is_chunked_response(evt->client)) {
            int copy_len = evt->data_len;
            if (response_len + copy_len < MAX_HTTP_RESPONSE_SIZE - 1) {
                memcpy(response_buffer + response_len, evt->data, copy_len);
                response_len += copy_len;
                response_buffer[response_len] = '\0';
            }
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

/* ----------------------------------------------------------------
 * Ethernet / network event handlers
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
 *
 * In QEMU with -nic user,model=open_eth, QEMU provides:
 *   - A virtual OpenCores Ethernet MAC at its default MMIO address
 *   - A built-in DHCP server (slirp) that assigns 10.0.2.15
 *   - NAT to the host network
 * ---------------------------------------------------------------- */
static esp_eth_handle_t eth_handle = NULL;

static void init_ethernet(void)
{
    eth_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Create default Ethernet netif */
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);

    /* OpenCores Ethernet MAC — the virtual NIC provided by QEMU */
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_openeth(&mac_config);

    /* DP83848 PHY — emulated by QEMU's open_eth */
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 0;
    phy_config.reset_gpio_num = -1;        /* No reset pin in QEMU */
    phy_config.autonego_timeout_ms = 100;  /* QEMU resolves instantly */
    esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));

    /* Attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

    /* Register event handlers */
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID,
                                               &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP,
                                               &ip_event_handler, NULL));

    /* Start Ethernet */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));

    ESP_LOGI(TAG, "Waiting for IP address from DHCP...");
}

/* ----------------------------------------------------------------
 * HTTP helper: perform GET request and print response
 * ---------------------------------------------------------------- */
static esp_err_t http_get(const char *url)
{
    ESP_LOGI(TAG, "GET %s", url);
    response_len = 0;
    memset(response_buffer, 0, sizeof(response_buffer));

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Response status=%d, length=%d", status, response_len);
        ESP_LOGI(TAG, "Body: %s", response_buffer);
    } else {
        ESP_LOGE(TAG, "HTTP GET failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

/* ----------------------------------------------------------------
 * HTTP helper: perform POST request with JSON body
 * ---------------------------------------------------------------- */
static esp_err_t http_post_json(const char *url, const char *json_body)
{
    ESP_LOGI(TAG, "POST %s", url);
    ESP_LOGI(TAG, "Body: %s", json_body);
    response_len = 0;
    memset(response_buffer, 0, sizeof(response_buffer));

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_body, strlen(json_body));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Response status=%d, length=%d", status, response_len);
        ESP_LOGI(TAG, "Body: %s", response_buffer);
    } else {
        ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

/* ----------------------------------------------------------------
 * Simulated sensor reading (since we don't have real ADC in QEMU)
 * ---------------------------------------------------------------- */
static float get_simulated_temperature(void)
{
    /* Simulate temperature between 20.0 and 30.0 C */
    return 20.0f + (float)(esp_random() % 100) / 10.0f;
}

static float get_simulated_humidity(void)
{
    /* Simulate humidity between 40.0 and 70.0 % */
    return 40.0f + (float)(esp_random() % 300) / 10.0f;
}

/* ----------------------------------------------------------------
 * Main application
 * ---------------------------------------------------------------- */
void app_main(void)
{
    printf("\n");
    printf("==========================================\n");
    printf("  ESP32 REST API Client (QEMU)\n");
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

    /* Small delay to let the network stack fully initialize */
    vTaskDelay(pdMS_TO_TICKS(2000));

    /* Step 2: Health check — verify the API server is reachable */
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Step 1: Health check");
    ESP_LOGI(TAG, "========================================");

    esp_err_t err = http_get(API_BASE_URL "/health");
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Local API server not reachable, trying httpbin.org...");
        err = http_get(HTTPBIN_BASE_URL "/get");
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "No server reachable. Check network configuration.");
            return;
        }
        ESP_LOGI(TAG, "Using httpbin.org as fallback");
    }

    /* Step 3: GET — fetch device configuration */
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Step 2: GET device configuration");
    ESP_LOGI(TAG, "========================================");

    http_get(API_BASE_URL "/api/config");

    /* Step 4: POST — send simulated sensor data in a loop */
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Step 3: POST sensor readings (loop)");
    ESP_LOGI(TAG, "========================================");

    for (int i = 0; i < 5; i++) {
        float temp = get_simulated_temperature();
        float humidity = get_simulated_humidity();

        char json[256];
        snprintf(json, sizeof(json),
                 "{\"device\":\"esp32-qemu-01\","
                 "\"temperature\":%.1f,"
                 "\"humidity\":%.1f,"
                 "\"reading_id\":%d}",
                 temp, humidity, i + 1);

        http_post_json(API_BASE_URL "/api/sensors", json);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }

    /* Step 5: GET — verify all readings were stored */
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Step 4: GET all stored readings");
    ESP_LOGI(TAG, "========================================");

    http_get(API_BASE_URL "/api/sensors");

    /* Step 6: GET latest reading */
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Step 5: GET latest reading");
    ESP_LOGI(TAG, "========================================");

    http_get(API_BASE_URL "/api/sensors/latest");

    /* Done */
    printf("\n");
    printf("==========================================\n");
    printf("  Demo complete!\n");
    printf("  Press Ctrl+A then X to exit QEMU\n");
    printf("==========================================\n");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
