#include "wifi.h"
#include "websocket_client.h"

static const char* TAG = "main";

void app_main(void)
{

    ESP_LOGI(TAG, "Starting ESP32 web socket application");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi in STA mode");
    wifi_init_sta();

    // Start WebSocket client
    ESP_LOGI(TAG, "Starting WebSocket client");
    websocket_app_main();
}