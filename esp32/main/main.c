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
        // Send current WiFi status back to STM32
        if (wifi_is_connected()) {
            char ip_str[32];
            wifi_get_ip_string(ip_str, sizeof(ip_str));
            char status_msg[64];
            snprintf(status_msg, sizeof(status_msg), "WiFi Connected: %s", ip_str);
            uart_send_status(SYSTEM_STATUS_WIFI_CONNECTED, 0, status_msg);
        }
        else {
            uart_send_status(SYSTEM_STATUS_WIFI_DISCONNECTED, 0, "WiFi Disconnected");
        }
    }
    else if (strcmp(command->command, "websocket_status") == 0) {
        // TODO: Implement actual WebSocket status check
        uart_send_status(SYSTEM_STATUS_WEBSOCKET_CONNECTED, 0, "WebSocket Connected");
    }
    else if (strcmp(command->command, "start_game") == 0) {
        // Handle game start command from STM32
        ESP_LOGI(TAG, "Game start command received from STM32");

        // Check if we're ready for game (WiFi connected)
        if (wifi_is_connected()) {
            uart_send_status(SYSTEM_STATUS_GAME_READY, 0, "Game Ready");
            uart_send_ack();
        }
        else {
            uart_send_status(SYSTEM_STATUS_ERROR, 1, "WiFi Not Connected");
            uart_send_nack();
        }
    }
    else if (strcmp(command->command, "end_game") == 0) {
        // Handle game end command from STM32  
        ESP_LOGI(TAG, "Game end command received from STM32");
        uart_send_status(SYSTEM_STATUS_GAME_ENDED, 0, "Game Ended");
        uart_send_ack();
    }
    else if (strcmp(command->command, "ping") == 0) {
        // Respond to ping with current status
        ESP_LOGI(TAG, "Ping received from STM32");
        char status_msg[64];
        if (wifi_is_connected()) {
            char ip_str[32];
            wifi_get_ip_string(ip_str, sizeof(ip_str));
            snprintf(status_msg, sizeof(status_msg), "Pong - WiFi: %s", ip_str);
        }
        else {
            snprintf(status_msg, sizeof(status_msg), "Pong - WiFi: Disconnected");
        }
        uart_send_command("pong", status_msg);
    }
    else {
        ESP_LOGW(TAG, "Unknown command received: %s", command->command);
        uart_send_nack();
    }
}

static void uart_status_callback(const uart_status_t* status) {
    ESP_LOGI(TAG, "Received status from STM32: System=%d, Error=%d, Message='%s'",
        status->system_status, status->error_code, status->status_message);

    // Handle STM32 status updates
    switch (status->system_status) {
    case SYSTEM_STATUS_STM32_READY:
        ESP_LOGI(TAG, "STM32 is ready for communication");
        break;
    case SYSTEM_STATUS_STM32_GAME_READY:
        ESP_LOGI(TAG, "STM32 game logic is ready");

        // Only start game if we have network connectivity
        if (wifi_is_connected()) {
            uart_send_status(SYSTEM_STATUS_GAME_ACTIVE, 0, "Game Session Started");
        }
        else {
            uart_send_status(SYSTEM_STATUS_ERROR, 1, "Cannot start game - No WiFi");
        }
        break;
    default:
        ESP_LOGI(TAG, "STM32 status update received");
        break;
    }
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

    // Phase 1: System Handshake
    ESP_LOGI(TAG, "=== PHASE 1: System Handshake ===");

    // Send initial status (non-blocking)
    uart_send_status(SYSTEM_STATUS_ESP32_STARTED, 0, "ESP32 Started");

    // Brief pause to let STM32 process
    vTaskDelay(pdMS_TO_TICKS(100));

    // Critical handshake - wait for STM32 to confirm it's ready (blocking)
    ESP_LOGI(TAG, "Waiting for STM32 handshake confirmation...");
    esp_err_t handshake_result = uart_send_status_with_ack(SYSTEM_STATUS_ESP32_READY, 0, "ESP32 Ready", 3000);

    if (handshake_result == ESP_OK) {
        ESP_LOGI(TAG, "✓ STM32 handshake successful - proceeding with initialization");
    }
    else if (handshake_result == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "⚠ STM32 handshake timeout - proceeding anyway");
    }
    else {
        ESP_LOGW(TAG, "⚠ STM32 handshake failed - proceeding anyway");
    }

    // Phase 2: WiFi Initialization (Non-blocking)
    ESP_LOGI(TAG, "=== PHASE 2: WiFi Initialization ===");

    // WiFi status updates are now sent from within the WiFi event handlers
    // This provides real-time status updates as WiFi connection progresses
    wifi_init_sta();

    // Verify WiFi connection and send final status if needed
    if (wifi_is_connected()) {
        char ip_str[32];
        wifi_get_ip_string(ip_str, sizeof(ip_str));
        ESP_LOGI(TAG, "✓ WiFi connected successfully: %s", ip_str);
        // Status already sent by WiFi event handler
    }
    else {
        ESP_LOGE(TAG, "✗ WiFi connection failed");
        uart_send_status(SYSTEM_STATUS_ERROR, 2, "WiFi Connection Failed");
        return;
    }

    // Phase 3: WebSocket Connection (Non-blocking)
    ESP_LOGI(TAG, "=== PHASE 3: WebSocket Connection ===");

    uart_send_status(SYSTEM_STATUS_WEBSOCKET_CONNECTING, 0, "WebSocket Connecting");

    websocket_app_main();

    // For now, simulate WebSocket connection
    vTaskDelay(pdMS_TO_TICKS(1000));
    uart_send_status(SYSTEM_STATUS_WEBSOCKET_CONNECTED, 0, "WebSocket Connected");

    // Phase 4: System Ready
    ESP_LOGI(TAG, "=== PHASE 4: System Ready ===");
    ESP_LOGI(TAG, "ESP32 initialization complete - system ready for operation");

    // Main application loop (placeholder)
    uint32_t heartbeat_counter = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 second loop

        // Send periodic heartbeat every 30 seconds
        if (++heartbeat_counter >= 3) {
            uart_send_heartbeat();
            heartbeat_counter = 0;

            // Optional: Send status summary
            if (wifi_is_connected()) {
                char ip_str[32];
                wifi_get_ip_string(ip_str, sizeof(ip_str));
                ESP_LOGI(TAG, "System healthy - WiFi: %s", ip_str);
            }
            else {
                ESP_LOGW(TAG, "System warning - WiFi disconnected");
                uart_send_status(SYSTEM_STATUS_WIFI_DISCONNECTED, 0, "WiFi Lost");
            }
        }
    }

    // This point should not be reached in normal operation
    ESP_LOGE(TAG, "Main application loop ended unexpectedly");
    uart_send_status(SYSTEM_STATUS_ERROR, 1, "ESP32 Error");
}