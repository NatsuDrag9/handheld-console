#include "./websocket_client.h"
#include "./helpers/websocket-client/websocket_client_to_server.h"
#include "./websocket-client/websocket_client_to_stm32.h"
#include <inttypes.h>

static const char* TAG = "WEBSOCKET";

// Unique client ID received from server
static char client_id[37] = { 0 }; // 36 chars for UUID + null terminator

typedef struct {
    char type_str[32];
    char message_id[64];
    char message_text[128];
    char game_data[128];
    char player_id[64];
    char metadata[64];
    char data_type[32];
    char status_type[32];
    char command[32];
    char command_data[128];
    char session_id[64];
    char status_data[128];
    bool found_type;
    bool found_id;
} msgpack_parse_data_t;

// WebSocket client handle (global for access from callbacks and helpers)
esp_websocket_client_handle_t ws_client = NULL;
// Static allocation - safe since WebSocket task is single-threaded
static msgpack_parse_data_t g_parse_data = { 0 };

// Callback functions
static websocket_status_callback_t status_callback = NULL;
static websocket_game_callback_t game_callback = NULL;
static websocket_connection_callback_t connection_callback = NULL;
static websocket_status_message_callback_t status_message_callback = NULL;

// MessagePack utility functions (shared with helper modules)
bool websocket_buffer_reader(struct cmp_ctx_s* ctx, void* data, size_t limit) {
    uint8_t** buffer_ptr = (uint8_t**)ctx->buf;
    memcpy(data, *buffer_ptr, limit);
    *buffer_ptr += limit;
    return true;
}

bool websocket_buffer_skipper(struct cmp_ctx_s* ctx, size_t count) {
    uint8_t** buffer_ptr = (uint8_t**)ctx->buf;
    *buffer_ptr += count;
    return true;
}

size_t websocket_buffer_writer(struct cmp_ctx_s* ctx, const void* data, size_t count) {
    uint8_t** buffer_ptr = (uint8_t**)ctx->buf;
    memcpy(*buffer_ptr, data, count);
    *buffer_ptr += count;
    return count;
}

// MessagePack parsing function
static void process_msgpack_data(const uint8_t* data, size_t len) {
    ESP_LOGI(TAG, "Processing MessagePack data (%zu bytes)", len);

    if (len == 0) {
        ESP_LOGE(TAG, "Empty MessagePack data");
        return;
    }

    // Clear the parse data structure
    memset(&g_parse_data, 0, sizeof(g_parse_data));

    // Make a copy of the data buffer pointer so we can modify it
    const uint8_t* buffer_ptr = data;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, websocket_buffer_reader,
        websocket_buffer_skipper, websocket_buffer_writer);

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
            // Skip this entry instead of returning - more robust error handling
            cmp_object_t skip_value;
            cmp_read_object(&cmp, &skip_value);
            continue;
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
                if (cmp_object_to_str(&cmp, &value_obj, g_parse_data.type_str, sizeof(g_parse_data.type_str))) {
                    g_parse_data.found_type = true;
                    ESP_LOGI(TAG, "Found type: '%s'", g_parse_data.type_str);
                }
            }
        }
        else if (strcmp(key, "id") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                if (cmp_object_to_str(&cmp, &value_obj, g_parse_data.message_id, sizeof(g_parse_data.message_id))) {
                    g_parse_data.found_id = true;
                    ESP_LOGI(TAG, "Found id: '%s'", g_parse_data.message_id);
                }
            }
        }
        else if (strcmp(key, "message") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                if (cmp_object_to_str(&cmp, &value_obj, g_parse_data.message_text, sizeof(g_parse_data.message_text))) {
                    ESP_LOGI(TAG, "Found message: '%s'", g_parse_data.message_text);
                }
            }
        }
        else if (strcmp(key, "data_type") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, g_parse_data.data_type, sizeof(g_parse_data.data_type));
                ESP_LOGI(TAG, "Found data_type: '%s'", g_parse_data.data_type);
            }
        }
        else if (strcmp(key, "data") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, g_parse_data.game_data, sizeof(g_parse_data.game_data));
                ESP_LOGI(TAG, "Found data: '%s'", g_parse_data.game_data);

                // Handle string-based data for ESP32 clients (player_assignment and opponent_connected)
                if (strcmp(g_parse_data.status_type, "player_assignment") == 0 || strcmp(g_parse_data.status_type, "opponent_connected") == 0) {
                    // Data is in template string format: "playerId: X, sessionId: Y, playerCount: Z, color: W"
                    // Parse the string to extract individual components
                    int parsed_player_id = 1;
                    char parsed_session_id[32] = { 0 };
                    int parsed_player_count = 1;
                    char parsed_color[16] = "blue";

                    // Parse the comma-separated string
                    char* data_copy = strdup(g_parse_data.game_data);  // Heap allocation
                    if (data_copy) {
                        char* token = strtok(data_copy, ",");
                        while (token != NULL) {
                            // Remove leading/trailing whitespace
                            while (*token == ' ') token++;
                            char* end = token + strlen(token) - 1;
                            while (end > token && *end == ' ') *end-- = '\0';

                            if (strncmp(token, "playerId:", 9) == 0) {
                                parsed_player_id = atoi(token + 10); // Skip "playerId: "
                                ESP_LOGI(TAG, "Parsed playerId: %d", parsed_player_id);
                            }
                            else if (strncmp(token, "sessionId:", 10) == 0) {
                                strncpy(parsed_session_id, token + 11, sizeof(parsed_session_id) - 1); // Skip "sessionId: "
                                parsed_session_id[sizeof(parsed_session_id) - 1] = '\0';
                                ESP_LOGI(TAG, "Parsed sessionId: '%s'", parsed_session_id);
                            }
                            else if (strncmp(token, "playerCount:", 12) == 0) {
                                parsed_player_count = atoi(token + 13); // Skip "playerCount: "
                                ESP_LOGI(TAG, "Parsed playerCount: %d", parsed_player_count);
                            }
                            else if (strncmp(token, "color:", 6) == 0) {
                                strncpy(parsed_color, token + 7, sizeof(parsed_color) - 1); // Skip "color: "
                                parsed_color[sizeof(parsed_color) - 1] = '\0';
                                ESP_LOGI(TAG, "Parsed color: '%s'", parsed_color);
                            }

                            token = strtok(NULL, ",");
                        }
                        free(data_copy);

                        // Format the parsed data for STM32 (same format as before)
                        snprintf(g_parse_data.status_data, sizeof(g_parse_data.status_data), "%d:%s:%d:%s",
                            parsed_player_id, parsed_session_id, parsed_player_count, parsed_color);
                        ESP_LOGI(TAG, "Formatted player assignment data: '%s'", g_parse_data.status_data);
                    }
                    else {
                        ESP_LOGE(TAG, "Failed to allocate memory for string parsing");
                        strncpy(g_parse_data.status_data, g_parse_data.game_data, sizeof(g_parse_data.status_data) - 1);
                        g_parse_data.status_data[sizeof(g_parse_data.status_data) - 1] = '\0';
                    }
                }
                else {
                    // For other message types, copy the string data directly
                    strncpy(g_parse_data.status_data, g_parse_data.game_data, sizeof(g_parse_data.status_data) - 1);
                    g_parse_data.status_data[sizeof(g_parse_data.status_data) - 1] = '\0';
                }
            }
        }
        else if (strcmp(key, "player_id") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, g_parse_data.player_id, sizeof(g_parse_data.player_id));
                ESP_LOGI(TAG, "Found player_id: '%s'", g_parse_data.player_id);
            }
        }
        else if (strcmp(key, "metadata") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, g_parse_data.metadata, sizeof(g_parse_data.metadata));
                ESP_LOGI(TAG, "Found metadata: '%s'", g_parse_data.metadata);
            }
        }
        else if (strcmp(key, "status") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, g_parse_data.status_type, sizeof(g_parse_data.status_type));
                ESP_LOGI(TAG, "Found status: '%s'", g_parse_data.status_type);
            }
        }
        else if (strcmp(key, "command") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, g_parse_data.command, sizeof(g_parse_data.command));
                ESP_LOGI(TAG, "Found command: '%s'", g_parse_data.command);
            }
        }
        else if (strcmp(key, "sessionId") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, g_parse_data.session_id, sizeof(g_parse_data.session_id));
                ESP_LOGI(TAG, "Found sessionId: '%s'", g_parse_data.session_id);
            }
        }
        else if (strcmp(key, "parameters") == 0) {
            if (cmp_object_is_str(&value_obj)) {
                cmp_object_to_str(&cmp, &value_obj, g_parse_data.command_data, sizeof(g_parse_data.command_data));
                ESP_LOGI(TAG, "Found command data: '%s'", g_parse_data.command_data);
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

    // Process the message based on type - route to STM32 converter
    if (g_parse_data.found_type) {
        ESP_LOGI(TAG, "Routing message type: %s", g_parse_data.type_str);

        // Handle connection messages to set client ID
        if (strcmp(g_parse_data.type_str, "connection") == 0) {
            if (g_parse_data.found_id) {
                strncpy(client_id, g_parse_data.message_id, sizeof(client_id) - 1);
                client_id[sizeof(client_id) - 1] = '\0';
                ESP_LOGI(TAG, "Successfully set client ID: %s", client_id);

                // Notify status callback about connection
                if (status_callback) {
                    status_callback(true, client_id);
                }

                // Notify connection callback
                if (connection_callback) {
                    connection_callback(g_parse_data.message_id, g_parse_data.message_text);
                }
            }
        }

        // Handle status messages with specific callback
        if (strcmp(g_parse_data.type_str, "status") == 0) {
            if (status_message_callback) {
                status_message_callback(g_parse_data.status_type, g_parse_data.message_text);
            }
        }

        // Route all messages to STM32 converter
        ws_to_stm32_process_server_message(g_parse_data.type_str, g_parse_data.message_id, g_parse_data.message_text,
            g_parse_data.data_type, g_parse_data.game_data, g_parse_data.metadata,
            g_parse_data.status_type, g_parse_data.command, g_parse_data.command_data,
            g_parse_data.session_id, g_parse_data.player_id, g_parse_data.status_data);

        // Call game callback if registered (for compatibility)
        if (game_callback && strlen(g_parse_data.data_type) > 0) {
            game_callback(g_parse_data.data_type, g_parse_data.game_data, g_parse_data.metadata);
        }
    }
    else {
        ESP_LOGE(TAG, "No message type found in MessagePack data");
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

        // Send connection status to STM32
        uart_send_status(SYSTEM_STATUS_WEBSOCKET_CONNECTED, 0, "WebSocket Connected...");
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
        uart_send_status(SYSTEM_STATUS_WEBSOCKET_DISCONNECTED, 1, "WebSocket Disconnected");
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

        // Send status to STM32
        uart_send_status(SYSTEM_STATUS_ERROR, 1, "Error when connecting to websocket");
        uart_send_status(SYSTEM_STATUS_WEBSOCKET_DISCONNECTED, 1, "WebSocket Disconnected");
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

    // Initialize integration modules
    websocket_init_integrations();

    /*     // Main communication loop - handle bidirectional data flow
        while (1) {
            // Check WebSocket connection status
            if (esp_websocket_client_is_connected(ws_client)) {
                if (strlen(client_id) > 0) {
                    // Connection is fully established
                    // To Do: Add periodic heartbeat or other maintenance tasks here
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
        esp_websocket_client_destroy(ws_client); */
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

void websocket_register_connection_callback(websocket_connection_callback_t callback) {
    connection_callback = callback;
}

void websocket_register_status_message_callback(websocket_status_message_callback_t callback) {
    status_message_callback = callback;
}

void websocket_init_integrations(void) {
    ESP_LOGI(TAG, "Initializing WebSocket integrations");

    // Register STM32 → Server conversion callbacks
    ws_to_server_register_uart_callbacks();

    // Server → STM32 conversion is handled via message routing in process_msgpack_data()

    ESP_LOGI(TAG, "WebSocket integrations initialized");
}