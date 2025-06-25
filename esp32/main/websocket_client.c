#include "./websocket_client.h"
#include <inttypes.h> // For PRIu32 format specifier
#include "../components/cmp/cmp.h"

static const char* TAG = "WEBSOCKET";

// Unique client ID received from server
static char client_id[37] = { 0 }; // 36 chars for UUID + null terminator

// WebSocket client handle (global for access from callbacks)
static esp_websocket_client_handle_t ws_client = NULL;

// Callback functions
static websocket_status_callback_t status_callback = NULL;
static websocket_game_callback_t game_callback = NULL;

// Custom buffer reader for MessagePack
static bool buffer_reader(struct cmp_ctx_s* ctx, void* data, size_t limit) {
    uint8_t** buffer_ptr = (uint8_t**)ctx->buf;
    memcpy(data, *buffer_ptr, limit);
    *buffer_ptr += limit;
    return true;
}

// Skip function for MessagePack
static bool buffer_skipper(struct cmp_ctx_s* ctx, size_t count) {
    uint8_t** buffer_ptr = (uint8_t**)ctx->buf;
    *buffer_ptr += count;
    return true;
}

// Custom buffer writer for MessagePack
static size_t buffer_writer(struct cmp_ctx_s* ctx, const void* data, size_t count) {
    uint8_t** buffer_ptr = (uint8_t**)ctx->buf;
    memcpy(*buffer_ptr, data, count);
    *buffer_ptr += count;
    return count;
}

// Forward declarations for STM32 integration
static void handle_server_game_data(const char* data_type, const char* game_data, const char* metadata);
static void handle_server_chat_message(const char* message, const char* metadata);

// MessagePack parsing function
static void process_msgpack_data(const uint8_t* data, size_t len) {
    ESP_LOGI(TAG, "Processing MessagePack data (%zu bytes)", len);

    if (len == 0) {
        ESP_LOGE(TAG, "Empty MessagePack data");
        return;
    }

    // Make a copy of the data buffer pointer so we can modify it
    const uint8_t* buffer_ptr = data;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, buffer_reader, buffer_skipper, buffer_writer);

    // Read the object header (expecting a map)
    cmp_object_t obj;
    if (!cmp_read_object(&cmp, &obj)) {
        ESP_LOGE(TAG, "Failed to read object: %s", cmp_strerror(&cmp));
        return;
    }

    if (!cmp_object_is_map(&obj)) {
        ESP_LOGE(TAG, "Expected a map but got type %d", obj.type);
        return;
    }

    uint32_t map_size;
    if (!cmp_object_as_map(&obj, &map_size)) {
        ESP_LOGE(TAG, "Failed to get map size: %s", cmp_strerror(&cmp));
        return;
    }

    ESP_LOGI(TAG, "Received a MessagePack map with %" PRIu32 " entries", map_size);

    // Variables to store parsed values
    char type_str[32] = { 0 };
    char message_id[64] = { 0 };
    char message_text[128] = { 0 };
    char game_data[128] = { 0 };
    char player_id[64] = { 0 };
    char metadata[64] = { 0 };
    char data_type[32] = { 0 };
    char nested_type[32] = { 0 };
    bool found_type = false;
    bool found_id = false;
    bool found_nested_message = false;

    // Read each key-value pair
    for (uint32_t i = 0; i < map_size; i++) {
        ESP_LOGI(TAG, "Reading entry %" PRIu32 "/%" PRIu32, i + 1, map_size);

        // Read the key
        cmp_object_t key_obj;
        if (!cmp_read_object(&cmp, &key_obj)) {
            ESP_LOGE(TAG, "Failed to read key object at position %" PRIu32 ": %s", i, cmp_strerror(&cmp));
            return;
        }

        if (!cmp_object_is_str(&key_obj)) {
            ESP_LOGE(TAG, "Expected string key at position %" PRIu32 " but got type %d", i, key_obj.type);
            return;
        }

        uint32_t key_len;
        if (!cmp_object_as_str(&key_obj, &key_len)) {
            ESP_LOGE(TAG, "Failed to get key length at position %" PRIu32, i);
            return;
        }

        char key[32] = { 0 };
        if (key_len >= sizeof(key)) {
            ESP_LOGE(TAG, "Key too long at position %" PRIu32 " (len=%" PRIu32 ")", i, key_len);
            return;
        }

        if (!cmp_object_to_str(&cmp, &key_obj, key, sizeof(key))) {
            ESP_LOGE(TAG, "Failed to read key string at position %" PRIu32, i);
            return;
        }

        ESP_LOGI(TAG, "Found key: '%s'", key);

        // Read the value based on the key
        cmp_object_t value_obj;
        if (!cmp_read_object(&cmp, &value_obj)) {
            ESP_LOGE(TAG, "Failed to read value object for key '%s': %s", key, cmp_strerror(&cmp));
            return;
        }

        // Process specific keys based on message type
        if (strcmp(key, "type") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                if (cmp_object_to_str(&cmp, &value_obj, type_str, sizeof(type_str))) {
                    found_type = true;
                    ESP_LOGI(TAG, "Found type: '%s'", type_str);
                }
            }
        }
        else if (strcmp(key, "id") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                if (cmp_object_to_str(&cmp, &value_obj, message_id, sizeof(message_id))) {
                    found_id = true;
                    ESP_LOGI(TAG, "Found id: '%s'", message_id);
                }
            }
        }
        else if (strcmp(key, "message") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                if (cmp_object_to_str(&cmp, &value_obj, message_text, sizeof(message_text))) {
                    ESP_LOGI(TAG, "Found message: '%s'", message_text);
                }
            }
            else if (cmp_object_is_map(&value_obj)) {
                ESP_LOGI(TAG, "Found nested message object - parsing for game data");
                found_nested_message = true;

                // Parse nested message for echo handling
                uint32_t nested_map_size;
                if (cmp_object_as_map(&value_obj, &nested_map_size)) {
                    // Parse nested fields similar to main parsing
                    for (uint32_t j = 0; j < nested_map_size; j++) {
                        cmp_object_t nested_key_obj;
                        if (!cmp_read_object(&cmp, &nested_key_obj)) break;

                        char nested_key[32] = { 0 };
                        if (cmp_object_is_str(&nested_key_obj)) {
                            cmp_object_to_str(&cmp, &nested_key_obj, nested_key, sizeof(nested_key));
                        }

                        cmp_object_t nested_value_obj;
                        if (!cmp_read_object(&cmp, &nested_value_obj)) break;

                        if (strcmp(nested_key, "type") == 0 && cmp_object_is_str(&nested_value_obj)) {
                            cmp_object_to_str(&cmp, &nested_value_obj, nested_type, sizeof(nested_type));
                        }
                        else if (strcmp(nested_key, "data") == 0 && cmp_object_is_str(&nested_value_obj)) {
                            cmp_object_to_str(&cmp, &nested_value_obj, game_data, sizeof(game_data));
                        }
                        else if (strcmp(nested_key, "metadata") == 0 && cmp_object_is_str(&nested_value_obj)) {
                            cmp_object_to_str(&cmp, &nested_value_obj, metadata, sizeof(metadata));
                        }
                    }
                }
            }
        }
        else if (strcmp(key, "data_type") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, data_type, sizeof(data_type));
                ESP_LOGI(TAG, "Found data_type: '%s'", data_type);
            }
        }
        else if (strcmp(key, "data") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, game_data, sizeof(game_data));
                ESP_LOGI(TAG, "Found data: '%s'", game_data);
            }
        }
        else if (strcmp(key, "player_id") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, player_id, sizeof(player_id));
                ESP_LOGI(TAG, "Found player_id: '%s'", player_id);
            }
        }
        else if (strcmp(key, "metadata") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, metadata, sizeof(metadata));
                ESP_LOGI(TAG, "Found metadata: '%s'", metadata);
            }
        }
        else if (strcmp(key, "timestamp") == 0) {
            if (cmp_object_is_uinteger(&value_obj)) {
                uint64_t timestamp;
                if (cmp_object_as_ulong(&value_obj, &timestamp)) {
                    ESP_LOGI(TAG, "Found timestamp: %" PRIu64, timestamp);
                }
            }
        }
        else {
            ESP_LOGI(TAG, "Skipping unknown key: '%s'", key);
        }
    }

    // Process the message based on type
    if (found_type) {
        ESP_LOGI(TAG, "Processing message type: %s", type_str);

        if (strcmp(type_str, "connection") == 0) {
            if (found_id) {
                strncpy(client_id, message_id, sizeof(client_id) - 1);
                client_id[sizeof(client_id) - 1] = '\0';
                ESP_LOGI(TAG, "Successfully set client ID: %s", client_id);

                // Notify status callback about connection
                if (status_callback) {
                    status_callback(true, client_id);
                }

                // Send status to STM32
                uart_send_status(WEBSOCKET_CONNECTED, 0, "WebSocket Connected");
            }
            else {
                ESP_LOGE(TAG, "Connection message but no ID found");
            }
        }
        else if (strcmp(type_str, "ping") == 0) {
            ESP_LOGI(TAG, "Received ping message - connection alive");
        }
        else if (strcmp(type_str, "echo") == 0) {
            ESP_LOGI(TAG, "Received echo message");
            // Handle echo messages if needed, but don't forward to STM32
            // Echo messages are for acknowledgment only
        }
        else if (strcmp(type_str, "game_data") == 0 ||
            strcmp(type_str, "player_action") == 0 ||
            strcmp(type_str, "game_state") == 0) {
            ESP_LOGI(TAG, "GAME CHANNEL: Received game data from server");
            ESP_LOGI(TAG, "Game data details: type=%s, data=%s", data_type, game_data);

            // Call game callback if registered
            if (game_callback) {
                game_callback(type_str, game_data, metadata);
            }

            // Forward to STM32 via game data channel
            handle_server_game_data(data_type, game_data, metadata);
        }
        else if (strcmp(type_str, "chat_message") == 0) {
            ESP_LOGI(TAG, "CHAT CHANNEL: Received chat message from server");
            ESP_LOGI(TAG, "Chat message: %s", message_text);

            // Forward to STM32 via chat channel
            handle_server_chat_message(message_text, metadata);
        }
        else if (strcmp(type_str, "broadcast") == 0) {
            ESP_LOGI(TAG, "Received broadcast message");
            // Handle broadcast messages (server announcements, etc.)
        }
        else if (strcmp(type_str, "command") == 0) {
            ESP_LOGI(TAG, "Received server command");
            // Process server commands
        }
        else {
            ESP_LOGW(TAG, "Unknown message type: %s", type_str);
        }
    }
    else {
        ESP_LOGE(TAG, "No message type found in MessagePack data");
    }
}

// Handle game data received from server and forward to STM32 (GAME CHANNEL)
static void handle_server_game_data(const char* data_type, const char* game_data, const char* metadata) {
    ESP_LOGI(TAG, "GAME CHANNEL: Forwarding game data to STM32");
    ESP_LOGI(TAG, "Details: type=%s, data=%s", data_type, game_data);

    // Forward to STM32 via UART as game data
    esp_err_t result = uart_send_game_data(data_type, game_data, metadata);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to forward game data to STM32: %s", esp_err_to_name(result));
    }
}

// Handle chat messages received from server and forward to STM32 (CHAT CHANNEL)
static void handle_server_chat_message(const char* message, const char* metadata) {
    ESP_LOGI(TAG, "CHAT CHANNEL: Forwarding chat message to STM32");
    ESP_LOGI(TAG, "Chat content: %s", message);

    // Forward to STM32 via UART as chat message
    esp_err_t result = uart_send_chat_message(message, "server", "websocket");
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to forward chat message to STM32: %s", esp_err_to_name(result));
    }
}

// Create and send game data message to server
static esp_err_t send_game_data_to_server(const char* data_type, const char* game_data, const char* metadata) {
    if (strlen(client_id) == 0) {
        ESP_LOGW(TAG, "Not sending game data - not connected yet");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Preparing to send game data to server: type=%s", data_type);

    // Create buffer for MessagePack data
    uint8_t buffer[MSGPACK_BUFFER_SIZE];
    uint8_t* buffer_ptr = buffer;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, buffer_reader, buffer_skipper, buffer_writer);

    // Calculate map size based on provided data
    uint32_t map_size = 4; // type, data, player_id, timestamp
    if (metadata && strlen(metadata) > 0) {
        map_size++;
    }

    // Write the map header
    if (!cmp_write_map(&cmp, map_size)) {
        ESP_LOGE(TAG, "Failed to write map header");
        return ESP_FAIL;
    }

    // Write "type" field
    if (!cmp_write_str(&cmp, "type", 4) ||
        !cmp_write_str(&cmp, data_type, strlen(data_type))) {
        ESP_LOGE(TAG, "Failed to write type field");
        return ESP_FAIL;
    }

    // Write "data" field
    if (!cmp_write_str(&cmp, "data", 4) ||
        !cmp_write_str(&cmp, game_data, strlen(game_data))) {
        ESP_LOGE(TAG, "Failed to write data field");
        return ESP_FAIL;
    }

    // Write "player_id" field
    if (!cmp_write_str(&cmp, "player_id", 9) ||
        !cmp_write_str(&cmp, client_id, strlen(client_id))) {
        ESP_LOGE(TAG, "Failed to write player_id field");
        return ESP_FAIL;
    }

    // Write "timestamp" field
    uint32_t timestamp = esp_log_timestamp();
    if (!cmp_write_str(&cmp, "timestamp", 9) ||
        !cmp_write_uint(&cmp, timestamp)) {
        ESP_LOGE(TAG, "Failed to write timestamp field");
        return ESP_FAIL;
    }

    // Write "metadata" field if provided
    if (metadata && strlen(metadata) > 0) {
        if (!cmp_write_str(&cmp, "metadata", 8) ||
            !cmp_write_str(&cmp, metadata, strlen(metadata))) {
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
            data_type, data_size);
    }
    else {
        ESP_LOGE(TAG, "Failed to send game data to server: %s (%d)", esp_err_to_name(err), err);

        // Additional debugging
        if (!esp_websocket_client_is_connected(ws_client)) {
            ESP_LOGE(TAG, "WebSocket connection lost during send attempt");
        }
    }

    return err;
}

// UART callback to handle game data from STM32
static void stm32_game_data_callback(const uart_game_data_t* stm32_data) {
    ESP_LOGI(TAG, "Received game data from STM32: type=%s, data=%s",
        stm32_data->data_type, stm32_data->game_data);

    // Forward STM32 data to WebSocket server
    esp_err_t result = send_game_data_to_server(stm32_data->data_type,
        stm32_data->game_data,
        stm32_data->metadata);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to forward STM32 data to server");
    }
}

// UART callback to handle chat messages from STM32
static void stm32_chat_message_callback(const uart_chat_message_t* stm32_chat) {
    ESP_LOGI(TAG, "Received chat message from STM32: %s", stm32_chat->message);

    // Forward STM32 chat to WebSocket server
    esp_err_t result = websocket_send_chat_message(stm32_chat->message);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to forward STM32 chat to server");
    }
}

static void websocket_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_websocket_event_data_t* data = (esp_websocket_event_data_t*)event_data;
    esp_websocket_client_handle_t client __attribute__((unused)) = (esp_websocket_client_handle_t)handler_args;

    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");

        // Clear any old client ID
        memset(client_id, 0, sizeof(client_id));

        // Send initial status to STM32
        uart_send_status(WEBSOCKET_CONNECTING, 0, "WebSocket Connecting...");

        // Send a test message to ensure the connection is working
        vTaskDelay(pdMS_TO_TICKS(1000)); // Give server time to send connection message
        ESP_LOGI(TAG, "Sending connection test message to server");
        websocket_send_game_data("connection_test", "ESP32 connected", "initial_test");
        break;

    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        // Clear client ID on disconnection
        memset(client_id, 0, sizeof(client_id));

        // Notify status callback about disconnection
        if (status_callback) {
            status_callback(false, "");
        }

        // Send status to STM32
        uart_send_status(WEBSOCKET_DISCONNECTED, 1, "WebSocket Disconnected");
        break;

    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d, data_len=%d", data->op_code, data->data_len);

        // Check for binary message (opcode 0x02)
        if (data->op_code == 0x02 && data->data_ptr && data->data_len > 0) {
            ESP_LOGI(TAG, "Received binary data, len=%d", data->data_len);

            // Process the MessagePack data
            process_msgpack_data((uint8_t*)data->data_ptr, data->data_len);
        }
        else if (data->op_code == 0x01 && data->data_ptr && data->data_len > 0) {
            // Text message (opcode 0x01)
            ESP_LOGW(TAG, "Received text message (not processing): %.*s",
                data->data_len, data->data_ptr);
        }
        else {
            ESP_LOGW(TAG, "Received non-binary/empty message, ignored");
        }
        break;

    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGE(TAG, "WEBSOCKET_EVENT_ERROR");

        // Notify status callback about error
        if (status_callback) {
            status_callback(false, "");
        }
        break;

    default:
        ESP_LOGI(TAG, "Other WebSocket event: %ld", event_id);
        break;
    }
}

// Public function implementations
void websocket_app_main(void) {
    ESP_LOGI(TAG, "Starting WebSocket client...");

    // Wait for WiFi connection to be established
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Configure websocket client
    esp_websocket_client_config_t websocket_cfg = {
        .uri = WEBSOCKET_URI,
        .disable_auto_reconnect = false,
        .reconnect_timeout_ms = WEBSOCKET_RECONNECT_TIMEOUT_MS,
        .network_timeout_ms = 10000,
    };

    // Initialize the WebSocket client
    ESP_LOGI(TAG, "Initializing WebSocket client to %s", WEBSOCKET_URI);
    ws_client = esp_websocket_client_init(&websocket_cfg);
    if (ws_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        return;
    }

    // Register event handler
    ESP_LOGI(TAG, "Registering WebSocket event handler");
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void*)ws_client);

    // Start the websocket client
    ESP_LOGI(TAG, "Starting WebSocket client");
    esp_err_t err = esp_websocket_client_start(ws_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WebSocket client: %s", esp_err_to_name(err));
        esp_websocket_client_destroy(ws_client);
        return;
    }

    ESP_LOGI(TAG, "WebSocket client started successfully");

    // Initialize STM32 integration
    websocket_init_stm32_integration();

    // Main communication loop - handle bidirectional data flow
    while (1) {
        // Check WebSocket connection status
        if (esp_websocket_client_is_connected(ws_client)) {
            if (strlen(client_id) > 0) {
                // Connection is fully established
                // TODO: Add periodic heartbeat or other maintenance tasks here
            }
            else {
                ESP_LOGW(TAG, "Connected but no client ID yet, waiting...");
            }
        }
        else {
            ESP_LOGW(TAG, "WebSocket not connected");
        }

        // Wait before next check
        vTaskDelay(pdMS_TO_TICKS(GAME_DATA_POLLING_INTERVAL));
    }

    // This code won't be reached in this example, but here for completeness
    ESP_LOGI(TAG, "Stopping WebSocket client");
    esp_websocket_client_stop(ws_client);
    ESP_LOGI(TAG, "Destroying WebSocket client");
    esp_websocket_client_destroy(ws_client);
}

// Public API implementations
esp_err_t websocket_send_game_data(const char* data_type, const char* game_data, const char* metadata) {
    return send_game_data_to_server(data_type, game_data, metadata);
}

esp_err_t websocket_send_player_action(const char* action, const char* parameters) {
    // Format action as game data
    char action_data[128];
    snprintf(action_data, sizeof(action_data), "{\"action\":\"%s\",\"params\":\"%s\"}",
        action, parameters ? parameters : "");

    return websocket_send_game_data("player_action", action_data, NULL);
}

esp_err_t websocket_send_chat_message(const char* message) {
    if (strlen(client_id) == 0) {
        ESP_LOGW(TAG, "Not sending chat message - not connected yet");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Preparing to send chat message to server: %s", message);

    // Create buffer for MessagePack data
    uint8_t buffer[MSGPACK_BUFFER_SIZE];
    uint8_t* buffer_ptr = buffer;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, buffer_reader, buffer_skipper, buffer_writer);

    // Write the map header (type and message)
    if (!cmp_write_map(&cmp, 3)) {
        ESP_LOGE(TAG, "Failed to write map header for chat");
        return ESP_FAIL;
    }

    // Write "type" field
    if (!cmp_write_str(&cmp, "type", 4) ||
        !cmp_write_str(&cmp, "chat_message", 12)) {
        ESP_LOGE(TAG, "Failed to write type field for chat");
        return ESP_FAIL;
    }

    // Write "message" field
    if (!cmp_write_str(&cmp, "message", 7) ||
        !cmp_write_str(&cmp, message, strlen(message))) {
        ESP_LOGE(TAG, "Failed to write message field for chat");
        return ESP_FAIL;
    }

    // Write "timestamp" field
    uint32_t timestamp = esp_log_timestamp();
    if (!cmp_write_str(&cmp, "timestamp", 9) ||
        !cmp_write_uint(&cmp, timestamp)) {
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

bool websocket_is_connected(void) {
    return ws_client && esp_websocket_client_is_connected(ws_client) && strlen(client_id) > 0;
}

const char* websocket_get_client_id(void) {
    return client_id;
}

void websocket_register_status_callback(websocket_status_callback_t callback) {
    status_callback = callback;
}

void websocket_register_game_callback(websocket_game_callback_t callback) {
    game_callback = callback;
}

void websocket_init_stm32_integration(void) {
    ESP_LOGI(TAG, "Initializing STM32 integration");

    // Register callback to receive game data from STM32
    uart_register_game_data_callback(stm32_game_data_callback);

    // Register callback to receive chat messages from STM32
    uart_register_chat_message_callback(stm32_chat_message_callback);

    ESP_LOGI(TAG, "STM32 integration initialized");
}

void websocket_forward_to_stm32(const char* data_type, const char* game_data, const char* metadata) {
    // Public interface for external forwarding if needed
    handle_server_game_data(data_type, game_data, metadata);
}

void websocket_forward_from_stm32(const uart_game_data_t* stm32_data) {
    // This function is called internally by stm32_game_data_callback
    // Public interface for external forwarding if needed
    stm32_game_data_callback(stm32_data);
}