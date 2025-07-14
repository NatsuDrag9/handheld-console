#include "websocket_client_to_server.h"
#include "../../websocket_client.h"
#include "esp_log.h"
#include <string.h>

static const char* TAG = "WS_TO_SERVER";

// External WebSocket client handle (from main websocket_client.c)
extern esp_websocket_client_handle_t ws_client;

// External MessagePack utility functions (from main websocket_client.c)
extern bool websocket_buffer_reader(struct cmp_ctx_s* ctx, void* data, size_t limit);
extern bool websocket_buffer_skipper(struct cmp_ctx_s* ctx, size_t count);
extern size_t websocket_buffer_writer(struct cmp_ctx_s* ctx, const void* data, size_t count);

// UART → WebSocket conversion functions
esp_err_t ws_to_server_convert_connection_message(const uart_connection_message_t* uart_msg,
    esp_websocket_client_handle_t ws_client) {
    if (!uart_msg || !ws_client) {
        ESP_LOGE(TAG, "Invalid parameters for connection message conversion");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Converting connection message to WebSocket format");

    // Create buffer for MessagePack data
    uint8_t buffer[MSGPACK_BUFFER_SIZE];
    uint8_t* buffer_ptr = buffer;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, websocket_buffer_reader,
        websocket_buffer_skipper, websocket_buffer_writer);

    // Write the map header (4 fields: type, clientId, message, timestamp)
    if (!cmp_write_map(&cmp, 4)) {
        ESP_LOGE(TAG, "Failed to write map header for connection message");
        return ESP_FAIL;
    }

    // Write "type" field
    if (!cmp_write_str(&cmp, "type", 4) ||
        !cmp_write_str(&cmp, WS_MSG_TYPE_CONNECTION, strlen(WS_MSG_TYPE_CONNECTION))) {
        ESP_LOGE(TAG, "Failed to write type field for connection message");
        return ESP_FAIL;
    }

    // Write client id field
    if (!cmp_write_str(&cmp, "id", 2) ||
        !cmp_write_str(&cmp, uart_msg->client_id, strlen(uart_msg->client_id))) {
        ESP_LOGE(TAG, "Failed to write id field for connection message");
        return ESP_FAIL;
    }

    // Write "message" field
    if (!cmp_write_str(&cmp, "message", 7) ||
        !cmp_write_str(&cmp, uart_msg->message, strlen(uart_msg->message))) {
        ESP_LOGE(TAG, "Failed to write message field for connection message");
        return ESP_FAIL;
    }

    // Write "timestamp" field
    if (!cmp_write_str(&cmp, "timestamp", 9) ||
        !cmp_write_uint(&cmp, uart_msg->timestamp)) {
        ESP_LOGE(TAG, "Failed to write timestamp field for connection message");
        return ESP_FAIL;
    }

    // Calculate the size of data written
    size_t data_size = buffer_ptr - buffer;

    // Send the MessagePack data
    esp_err_t err = esp_websocket_client_send_bin(ws_client, (const char*)buffer, data_size, portMAX_DELAY);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully sent connection message to server (packed size: %zu bytes)", data_size);
    }
    else {
        ESP_LOGE(TAG, "Failed to send connection message to server: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t ws_to_server_convert_tile_size_validation(const uart_tile_size_validation_t* uart_msg,
    esp_websocket_client_handle_t ws_client) {
    if (!uart_msg || !ws_client) {
        ESP_LOGE(TAG, "Invalid parameters for tile size validation conversion");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Converting tile size validation to WebSocket format");

    // Create buffer for MessagePack data
    uint8_t buffer[MSGPACK_BUFFER_SIZE];
    uint8_t* buffer_ptr = buffer;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, websocket_buffer_reader,
        websocket_buffer_skipper, websocket_buffer_writer);

    // Write the map header (3 fields: type, tileSize, timestamp)
    if (!cmp_write_map(&cmp, 3)) {
        ESP_LOGE(TAG, "Failed to write map header for tile size validation");
        return ESP_FAIL;
    }

    // Write "type" field
    if (!cmp_write_str(&cmp, "type", 4) ||
        !cmp_write_str(&cmp, WS_MSG_TYPE_TILE_SIZE_VALIDATION, strlen(WS_MSG_TYPE_TILE_SIZE_VALIDATION))) {
        ESP_LOGE(TAG, "Failed to write type field for tile size validation");
        return ESP_FAIL;
    }

    // Write "tileSize" field
    if (!cmp_write_str(&cmp, "tileSize", 8) ||
        !cmp_write_uint(&cmp, uart_msg->tile_size)) {
        ESP_LOGE(TAG, "Failed to write tileSize field");
        return ESP_FAIL;
    }

    // Write "timestamp" field
    if (!cmp_write_str(&cmp, "timestamp", 9) ||
        !cmp_write_uint(&cmp, uart_msg->timestamp)) {
        ESP_LOGE(TAG, "Failed to write timestamp field for tile size validation");
        return ESP_FAIL;
    }

    // Calculate the size of data written
    size_t data_size = buffer_ptr - buffer;

    // Send the MessagePack data
    esp_err_t err = esp_websocket_client_send_bin(ws_client, (const char*)buffer, data_size, portMAX_DELAY);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully sent tile size validation to server (packed size: %zu bytes)", data_size);
    }
    else {
        ESP_LOGE(TAG, "Failed to send tile size validation to server: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t ws_to_server_convert_game_data(const uart_game_data_t* uart_msg,
    esp_websocket_client_handle_t ws_client) {
    if (!uart_msg || !ws_client) {
        ESP_LOGE(TAG, "Invalid parameters for game data conversion");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Converting game data to WebSocket format: type=%s", uart_msg->data_type);

    // Create buffer for MessagePack data
    uint8_t buffer[MSGPACK_BUFFER_SIZE];
    uint8_t* buffer_ptr = buffer;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, websocket_buffer_reader,
        websocket_buffer_skipper, websocket_buffer_writer);

    // Calculate map size based on provided data
    uint32_t map_size = 4; // type, data_type, data, timestamp
    if (strlen(uart_msg->metadata) > 0) {
        map_size++;
    }

    // Write the map header
    if (!cmp_write_map(&cmp, map_size)) {
        ESP_LOGE(TAG, "Failed to write map header for game data");
        return ESP_FAIL;
    }

    // Write "type" field
    if (!cmp_write_str(&cmp, "type", 4) ||
        !cmp_write_str(&cmp, WS_MSG_TYPE_GAME_DATA_MESSAGE, strlen(WS_MSG_TYPE_GAME_DATA_MESSAGE))) {
        ESP_LOGE(TAG, "Failed to write type field for game data");
        return ESP_FAIL;
    }

    // Write "data_type" field
    if (!cmp_write_str(&cmp, "data_type", 9) ||
        !cmp_write_str(&cmp, uart_msg->data_type, strlen(uart_msg->data_type))) {
        ESP_LOGE(TAG, "Failed to write data_type field");
        return ESP_FAIL;
    }

    // Write "data" field
    if (!cmp_write_str(&cmp, "data", 4) ||
        !cmp_write_str(&cmp, uart_msg->game_data, strlen(uart_msg->game_data))) {
        ESP_LOGE(TAG, "Failed to write data field");
        return ESP_FAIL;
    }

    // Write "timestamp" field
    if (!cmp_write_str(&cmp, "timestamp", 9) ||
        !cmp_write_uint(&cmp, uart_msg->sequence_num)) {
        ESP_LOGE(TAG, "Failed to write timestamp field");
        return ESP_FAIL;
    }

    // Write "metadata" field if provided
    if (strlen(uart_msg->metadata) > 0) {
        if (!cmp_write_str(&cmp, "metadata", 8) ||
            !cmp_write_str(&cmp, uart_msg->metadata, strlen(uart_msg->metadata))) {
            ESP_LOGE(TAG, "Failed to write metadata field");
            return ESP_FAIL;
        }
    }

    // Calculate the size of data written
    size_t data_size = buffer_ptr - buffer;

    // Send the MessagePack data
    esp_err_t err = esp_websocket_client_send_bin(ws_client, (const char*)buffer, data_size, portMAX_DELAY);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully sent game data to server: type=%s (packed size: %zu bytes)",
            uart_msg->data_type, data_size);
    }
    else {
        ESP_LOGE(TAG, "Failed to send game data to server: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t ws_to_server_convert_chat_message(const uart_chat_message_t* uart_msg,
    esp_websocket_client_handle_t ws_client) {
    if (!uart_msg || !ws_client) {
        ESP_LOGE(TAG, "Invalid parameters for chat message conversion");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Converting chat message to WebSocket format");

    // Create buffer for MessagePack data
    uint8_t buffer[MSGPACK_BUFFER_SIZE];
    uint8_t* buffer_ptr = buffer;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, websocket_buffer_reader,
        websocket_buffer_skipper, websocket_buffer_writer);

    // Write the map header (3 fields: type, message, timestamp)
    if (!cmp_write_map(&cmp, 3)) {
        ESP_LOGE(TAG, "Failed to write map header for chat");
        return ESP_FAIL;
    }

    // Write "type" field
    if (!cmp_write_str(&cmp, "type", 4) ||
        !cmp_write_str(&cmp, WS_MSG_TYPE_CHAT_MESSAGE, strlen(WS_MSG_TYPE_CHAT_MESSAGE))) {
        ESP_LOGE(TAG, "Failed to write type field for chat");
        return ESP_FAIL;
    }

    // Write "message" field
    if (!cmp_write_str(&cmp, "message", 7) ||
        !cmp_write_str(&cmp, uart_msg->message, strlen(uart_msg->message))) {
        ESP_LOGE(TAG, "Failed to write message field for chat");
        return ESP_FAIL;
    }

    // Write "timestamp" field
    if (!cmp_write_str(&cmp, "timestamp", 9) ||
        !cmp_write_uint(&cmp, uart_msg->timestamp)) {
        ESP_LOGE(TAG, "Failed to write timestamp field for chat");
        return ESP_FAIL;
    }

    // Calculate the size of data written
    size_t data_size = buffer_ptr - buffer;

    // Send the MessagePack data
    esp_err_t err = esp_websocket_client_send_bin(ws_client, (const char*)buffer, data_size, portMAX_DELAY);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully sent chat message to server (packed size: %zu bytes)", data_size);
    }
    else {
        ESP_LOGE(TAG, "Failed to send chat message to server: %s", esp_err_to_name(err));
    }

    return err;
}

// UART callback functions (called when STM32 sends messages)
static void uart_connection_message_callback(const uart_connection_message_t* connection_msg) {
    ESP_LOGI(TAG, "Received connection message from STM32, converting to WebSocket");
    ws_to_server_convert_connection_message(connection_msg, ws_client);
}

static void uart_tile_size_validation_callback(const uart_tile_size_validation_t* tile_size_msg) {
    ESP_LOGI(TAG, "Received tile size validation from STM32, converting to WebSocket");
    ws_to_server_convert_tile_size_validation(tile_size_msg, ws_client);
}

static void uart_game_data_callback(const uart_game_data_t* game_data) {
    ESP_LOGI(TAG, "Received game data from STM32, converting to WebSocket");
    ws_to_server_convert_game_data(game_data, ws_client);
}

static void uart_chat_message_callback(const uart_chat_message_t* chat_message) {
    ESP_LOGI(TAG, "Received chat message from STM32, converting to WebSocket");
    ws_to_server_convert_chat_message(chat_message, ws_client);
}

// Register all UART callbacks
void ws_to_server_register_uart_callbacks(void) {
    ESP_LOGI(TAG, "Registering UART callbacks for STM32 → Server conversion");

    uart_register_connection_message_callback(uart_connection_message_callback);
    uart_register_tile_size_validation_callback(uart_tile_size_validation_callback);
    uart_register_game_data_callback(uart_game_data_callback);
    uart_register_chat_message_callback(uart_chat_message_callback);

    ESP_LOGI(TAG, "All STM32 → Server UART callbacks registered successfully");
}