#include "wifi.h"
#include "websocket_client.h"
#include "uart_comm.h"

static const char* TAG = "main";

// UART callback functions for multiplayer gaming ecosystem
static void uart_game_data_callback(const uart_game_data_t* game_data) {
    ESP_LOGI(TAG, "Received game data from STM32: type=%s, data=%s",
        game_data->data_type, game_data->game_data);

    // This data will be automatically forwarded to WebSocket server
    // via the websocket_init_stm32_integration() callback system
}

static void uart_command_callback(const uart_command_t* command) {
    ESP_LOGI(TAG, "Received command from STM32: %s %s",
        command->command, command->parameters);

    // Process commands from STM32 for gaming ecosystem
    if (strcmp(command->command, "wifi_status") == 0) {
        // Send WiFi status back to STM32
        uart_send_status(1, 0, "WiFi Connected");
    }
    else if (strcmp(command->command, "websocket_status") == 0) {
        // TODO: Send WebSocket connection status to STM32
        uart_send_status(2, 0, "WebSocket Connected");
    }
    else if (strcmp(command->command, "start_game") == 0) {
        // TODO: Handle game start command from STM32
        ESP_LOGI(TAG, "Game start command received from STM32");
        uart_send_ack();
    }
    else if (strcmp(command->command, "end_game") == 0) {
        // TODO: Handle game end command from STM32  
        ESP_LOGI(TAG, "Game end command received from STM32");
        uart_send_ack();
    }
    else if (strcmp(command->command, "reboot") == 0) {
        ESP_LOGI(TAG, "Reboot command received from STM32");
        uart_send_ack();
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
    }
    // TODO: Add more game-specific command processing as game structure is finalized
}

static void uart_status_callback(const uart_status_t* status) {
    ESP_LOGI(TAG, "Received status from STM32: System=%d, Error=%d, Message='%s'",
        status->system_status, status->error_code, status->status_message);
}

static void uart_message_callback(uart_message_type_t type, const uint8_t* data, size_t length) {
    ESP_LOGI(TAG, "Received UART message type: 0x%02X, length: %zu", type, length);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 web socket and UART communication application");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize UART communication first
    ESP_LOGI(TAG, "Initializing UART communication with STM32");
    ret = uart_comm_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize UART communication: %s", esp_err_to_name(ret));
        return;
    }

    // Register UART callbacks
    uart_register_game_data_callback(uart_game_data_callback);
    uart_register_command_callback(uart_command_callback);
    uart_register_status_callback(uart_status_callback);
    uart_register_message_callback(uart_message_callback);

    // Send initial status to STM32
    uart_send_status(1, 0, "ESP32 Started");

    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi in STA mode");
    wifi_init_sta();

    // Notify STM32 that WiFi is initialized
    uart_send_status(2, 0, "WiFi Initialized");

    // Start WebSocket client
    ESP_LOGI(TAG, "Starting WebSocket client");
    websocket_app_main();

    // This point should not be reached in normal operation
    // If we get here, something went wrong
    ESP_LOGE(TAG, "Main application loop ended unexpectedly");
    uart_send_status(0, 1, "ESP32 Error");
}
