#include "./wifi.h"
#include "uart_comm.h"  // Add this include

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char* TAG = "wifi_station";
static int s_retry_num = 0;

// Check if WiFi is currently connected
bool wifi_is_connected(void)
{
    if (s_wifi_event_group == NULL) {
        return false;
    }

    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

// Get current IP address as string
void wifi_get_ip_string(char* buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0) {
        return;
    }

    if (!wifi_is_connected()) {
        snprintf(buffer, buffer_size, "Not Connected");
        return;
    }

    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        snprintf(buffer, buffer_size, IPSTR, IP2STR(&ip_info.ip));
    }
    else {
        snprintf(buffer, buffer_size, "IP Unknown");
    }
}

// Get detailed WiFi connection information
esp_err_t wifi_get_connection_info(wifi_ap_record_t* ap_info)
{
    if (!wifi_is_connected() || !ap_info) {
        return ESP_ERR_INVALID_STATE;
    }

    return esp_wifi_sta_get_ap_info(ap_info);
}

static void event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi station started, attempting connection");

        // Notify STM32 that WiFi connection is starting
        uart_send_status(SYSTEM_STATUS_WIFI_CONNECTING, 0, "WiFi Connecting");

        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*)event_data;
        ESP_LOGI(TAG, "WiFi disconnected, reason: %d", event->reason);

        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry %d/%d to connect to the AP", s_retry_num, EXAMPLE_ESP_MAXIMUM_RETRY);

            // Notify STM32 of retry attempt
            char retry_msg[32];
            snprintf(retry_msg, sizeof(retry_msg), "WiFi Retry %d/%d", s_retry_num, EXAMPLE_ESP_MAXIMUM_RETRY);
            uart_send_status(SYSTEM_STATUS_WIFI_CONNECTING, 0, retry_msg);
        }
        else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to WiFi after maximum retries");

            // Notify STM32 of WiFi failure
            uart_send_status(SYSTEM_STATUS_WIFI_DISCONNECTED, 1, "WiFi Failed");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        // Immediately notify STM32 of successful connection with IP info
        char ip_msg[64];
        snprintf(ip_msg, sizeof(ip_msg), "WiFi Connected: " IPSTR, IP2STR(&event->ip_info.ip));
        uart_send_status(SYSTEM_STATUS_WIFI_CONNECTED, 0, ip_msg);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler,
        NULL,
        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_handler,
        NULL,
        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP SSID:%s password:%s",
            EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        // Status already sent in event handler
    }
    else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
            EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        // Status already sent in event handler
    }
    else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        uart_send_status(SYSTEM_STATUS_ERROR, 3, "WiFi Unexpected Error");
    }
}

// Get status string for debugging
const char* wifi_get_status_string(void)
{
    if (!wifi_is_connected()) {
        return "Disconnected";
    }
    return "Connected";
}