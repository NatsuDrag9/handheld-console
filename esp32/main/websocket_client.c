// #include "./websocket_client.h"

// static const char* TAG = "WEBSOCKET";

// // Unique client ID received from server
// static char client_id[37] = { 0 }; // 36 chars for UUID + null terminator


// static uint8_t msgpack_buffer[MSGPACK_BUFFER_SIZE];
// static cmp_ctx_t cmp_ctx;
// static size_t msgpack_buffer_pos = 0;

// // Custom file writer for MessagePack
// static bool msgpack_file_writer(cmp_ctx_t* ctx, const void* data, size_t count) {
//     uint8_t* buf = (uint8_t*)ctx->buf;
//     size_t pos = *(size_t*)ctx->buf_data;

//     if (pos + count > MSGPACK_BUFFER_SIZE)
//         return false;

//     memcpy(&buf[pos], data, count);
//     *(size_t*)ctx->buf_data = pos + count;
//     return true;
// }

// // Custom file reader for MessagePack
// static bool msgpack_file_reader(cmp_ctx_t* ctx, void* data, size_t count) {
//     uint8_t* buf = (uint8_t*)ctx->buf;
//     size_t pos = *(size_t*)ctx->buf_data;

//     if (pos + count > MSGPACK_BUFFER_SIZE)
//         return false;

//     memcpy(data, &buf[pos], count);
//     *(size_t*)ctx->buf_data = pos + count;
//     return true;
// }

// // Initialize MessagePack context
// static void init_msgpack_context(void) {
//     msgpack_buffer_pos = 0;
//     cmp_init(&cmp_ctx, msgpack_buffer, msgpack_file_reader, msgpack_file_writer, &msgpack_buffer_pos);
// }

// // Read string value from MessagePack map with error checking
// static bool read_map_string(cmp_ctx_t* ctx, const char* key, char* out_str, uint32_t max_len) {
//     uint32_t map_size;
//     uint32_t key_len;
//     char map_key[64];
//     uint32_t str_len;

//     // Reset the buffer position
//     *(size_t*)ctx->buf_data = 0;

//     if (!cmp_read_map(ctx, &map_size)) {
//         ESP_LOGE(TAG, "Failed to read map size: %s", cmp_strerror(ctx));
//         return false;
//     }

//     for (uint32_t i = 0; i < map_size; i++) {
//         // Read the key
//         if (!cmp_read_str_size(ctx, &key_len)) {
//             ESP_LOGE(TAG, "Failed to read key size: %s", cmp_strerror(ctx));
//             return false;
//         }

//         if (key_len >= sizeof(map_key)) {
//             ESP_LOGE(TAG, "Key too long");
//             return false;
//         }

//         if (!cmp_ctx.read(ctx, map_key, key_len)) {
//             ESP_LOGE(TAG, "Failed to read key string: %s", cmp_strerror(ctx));
//             return false;
//         }

//         map_key[key_len] = '\0';

//         // Check if this is the key we're looking for
//         if (strcmp(map_key, key) == 0) {
//             // Read the string value
//             if (!cmp_read_str_size(ctx, &str_len)) {
//                 ESP_LOGE(TAG, "Failed to read string size: %s", cmp_strerror(ctx));
//                 return false;
//             }

//             if (str_len >= max_len) {
//                 ESP_LOGE(TAG, "String too long");
//                 return false;
//             }

//             if (!cmp_ctx.read(ctx, out_str, str_len)) {
//                 ESP_LOGE(TAG, "Failed to read string value: %s", cmp_strerror(ctx));
//                 return false;
//             }

//             out_str[str_len] = '\0';
//             return true;
//         }
//         else {
//             // Skip this value
//             if (!cmp_skip_object(ctx)) {
//                 ESP_LOGE(TAG, "Failed to skip object: %s", cmp_strerror(ctx));
//                 return false;
//             }
//         }
//     }

//     return false;
// }

// // Process the MessagePack data received from WebSocket
// static void process_msgpack_data(const uint8_t* data, size_t len) {
//     // Copy the received data to our buffer
//     if (len > MSGPACK_BUFFER_SIZE) {
//         ESP_LOGE(TAG, "MessagePack data too large (%d bytes)", len);
//         return;
//     }

//     memcpy(msgpack_buffer, data, len);

//     // Initialize the context for reading
//     msgpack_buffer_pos = 0;

//     char message_type[32] = { 0 };

//     // Try to read the message type
//     if (read_map_string(&cmp_ctx, "type", message_type, sizeof(message_type))) {
//         ESP_LOGI(TAG, "Message type: %s", message_type);

//         // Handle different message types
//         if (strcmp(message_type, "connection") == 0) {
//             // Read the client ID
//             if (read_map_string(&cmp_ctx, "id", client_id, sizeof(client_id))) {
//                 ESP_LOGI(TAG, "Received client ID: %s", client_id);
//             }
//         }
//         else if (strcmp(message_type, "echo") == 0) {
//             ESP_LOGI(TAG, "Received echo message");
//         }
//         else if (strcmp(message_type, "broadcast") == 0) {
//             ESP_LOGI(TAG, "Received broadcast message");
//         }
//         else if (strcmp(message_type, "command") == 0) {
//             char command[32] = { 0 };
//             if (read_map_string(&cmp_ctx, "command", command, sizeof(command))) {
//                 ESP_LOGI(TAG, "Received command: %s", command);

//                 // Process command
//                 if (strcmp(command, "restart") == 0) {
//                     ESP_LOGI(TAG, "Restarting...");
//                     esp_restart();
//                 }
//                 else if (strcmp(command, "sleep") == 0) {
//                     ESP_LOGI(TAG, "Sleep command received (not implemented)");
//                 }
//                 else {
//                     ESP_LOGI(TAG, "Unknown command: %s", command);
//                 }
//             }
//         }
//     }
//     else {
//         ESP_LOGE(TAG, "Could not read message type");
//     }
// }

// // Create and send a sensor data message
// static void send_sensor_data(esp_websocket_client_handle_t client,
//     const char* sensor_type,
//     float value,
//     const char* unit) {
//     if (strlen(client_id) == 0) {
//         ESP_LOGW(TAG, "Not sending sensor data - not connected yet");
//         return;
//     }

//     // Initialize context for writing
//     init_msgpack_context();

//     // Write the message as a map
//     uint32_t map_size = unit ? 4 : 3; // Add unit field if provided

//     if (!cmp_write_map(&cmp_ctx, map_size)) {
//         ESP_LOGE(TAG, "Failed to write map");
//         return;
//     }

//     // Write type field
//     if (!cmp_write_str(&cmp_ctx, "type", 4) ||
//         !cmp_write_str(&cmp_ctx, "sensorData", 10)) {
//         ESP_LOGE(TAG, "Failed to write type field");
//         return;
//     }

//     // Write sensorType field
//     if (!cmp_write_str(&cmp_ctx, "sensorType", 10) ||
//         !cmp_write_str(&cmp_ctx, sensor_type, strlen(sensor_type))) {
//         ESP_LOGE(TAG, "Failed to write sensorType field");
//         return;
//     }

//     // Write value field
//     if (!cmp_write_str(&cmp_ctx, "value", 5) ||
//         !cmp_write_float(&cmp_ctx, value)) {
//         ESP_LOGE(TAG, "Failed to write value field");
//         return;
//     }

//     // Write unit field if provided
//     if (unit) {
//         if (!cmp_write_str(&cmp_ctx, "unit", 4) ||
//             !cmp_write_str(&cmp_ctx, unit, strlen(unit))) {
//             ESP_LOGE(TAG, "Failed to write unit field");
//             return;
//         }
//     }

//     // Send the MessagePack data
//     esp_websocket_client_send_bin(client, msgpack_buffer, msgpack_buffer_pos, portMAX_DELAY);
//     ESP_LOGI(TAG, "Sent sensor data: %s = %f %s", sensor_type, value, unit ? unit : "");
// }

// static void websocket_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
//     esp_websocket_event_data_t* data = (esp_websocket_event_data_t*)event_data;
//     esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)handler_args;

//     switch (event_id) {
//     case WEBSOCKET_EVENT_CONNECTED:
//         ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
//         break;
//     case WEBSOCKET_EVENT_DISCONNECTED:
//         ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
//         // Clear client ID on disconnection
//         memset(client_id, 0, sizeof(client_id));
//         break;
//     case WEBSOCKET_EVENT_DATA:
//         ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
//         ESP_LOGI(TAG, "Received opcode=%d", data->op_code);

//         if (data->op_code == 0x02) {  // Binary message
//             ESP_LOGI(TAG, "Received binary data, len=%d", data->data_len);

//             // Process the MessagePack data
//             process_msgpack_data((uint8_t*)data->data_ptr, data->data_len);
//         }
//         else {
//             ESP_LOGW(TAG, "Received non-binary message, ignored");
//         }
//         break;
//     case WEBSOCKET_EVENT_ERROR:
//         ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
//         break;
//     }
// }

// void websocket_app_main(void) {
//     // Wait for WiFi connection
//     vTaskDelay(pdMS_TO_TICKS(2000));

//     // Configure websocket client
//     esp_websocket_client_config_t websocket_cfg = {
//         .uri = WEBSOCKET_URI,
//         .disable_auto_reconnect = false,
//         .reconnect_timeout_ms = WEBSOCKET_RECONNECT_TIMEOUT_MS,
//     };

//     esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
//     if (client == NULL) {
//         ESP_LOGE(TAG, "Failed to initialize WebSocket client");
//         return;
//     }

//     esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void*)client);

//     // Start the websocket client
//     esp_err_t err = esp_websocket_client_start(client);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to start WebSocket client: %s", esp_err_to_name(err));
//         return;
//     }

//     ESP_LOGI(TAG, "WebSocket client started");

//     // Send sensor data periodically
//     while (1) {
//         // Skip if not connected yet
//         if (esp_websocket_client_is_connected(client) && strlen(client_id) > 0) {
//             // Simulate a temperature reading
//             float temperature = 24.5 + ((float)esp_random() / UINT32_MAX * 2.0 - 1.0);
//             send_sensor_data(client, "temperature", temperature, "C");
//         }

//         vTaskDelay(pdMS_TO_TICKS(SENSOR_SAMPLING_INTERVAL));
//     }

//     // Cleanup (this code will not be reached in this example)
//     esp_websocket_client_destroy(client);
// }

#include "./websocket_client.h"
#include <inttypes.h> // For PRIu32 format specifier
#include "../components/cmp/cmp.h"

static const char* TAG = "WEBSOCKET";

// Unique client ID received from server
static char client_id[37] = { 0 }; // 36 chars for UUID + null terminator

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

// Process the MessagePack data received from WebSocket
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

    // Variables to store message type
    char type_str[32] = { 0 };
    uint32_t type_size = 0;
    bool found_type = false;

    // Read each key-value pair to find the "type" key
    for (uint32_t i = 0; i < map_size; i++) {
        // Read the key
        char key[32] = { 0 };
        uint32_t key_size = sizeof(key);

        if (!cmp_read_object(&cmp, &obj) || !cmp_object_is_str(&obj)) {
            ESP_LOGE(TAG, "Expected string key at position %" PRIu32, i);
            return;
        }

        if (!cmp_object_as_str(&obj, &key_size)) {
            ESP_LOGE(TAG, "Failed to get key size at position %" PRIu32, i);
            return;
        }

        if (key_size >= sizeof(key)) {
            ESP_LOGE(TAG, "Key too long at position %" PRIu32, i);
            return;
        }

        if (!cmp_object_to_str(&cmp, &obj, key, sizeof(key))) {
            ESP_LOGE(TAG, "Failed to read key string at position %" PRIu32, i);
            return;
        }

        // Check if this is the "type" key
        if (strcmp(key, "type") == 0) {
            // Read the value
            if (!cmp_read_object(&cmp, &obj) || !cmp_object_is_str(&obj)) {
                ESP_LOGE(TAG, "Expected string value for 'type'");
                return;
            }

            type_size = sizeof(type_str);
            if (!cmp_object_to_str(&cmp, &obj, type_str, type_size)) {
                ESP_LOGE(TAG, "Failed to read 'type' value");
                return;
            }

            found_type = true;
            ESP_LOGI(TAG, "Found message type: %s", type_str);
        }
        else {
            // Skip the value for other keys
            if (!cmp_read_object(&cmp, &obj)) {
                ESP_LOGE(TAG, "Failed to read value for key '%s'", key);
                return;
            }
        }
    }

    if (!found_type) {
        ESP_LOGW(TAG, "Message has no 'type' field");
        return;
    }

    // Process based on message type
    if (strcmp(type_str, "connection") == 0) {
        // Need to reread the entire message to find the "id" field
        buffer_ptr = data;  // Reset the buffer pointer
        cmp_init(&cmp, (void*)&buffer_ptr, buffer_reader, buffer_skipper, buffer_writer);

        // Read the map header again
        if (!cmp_read_object(&cmp, &obj) || !cmp_object_is_map(&obj)) {
            ESP_LOGE(TAG, "Failed to re-read map header for connection message");
            return;
        }

        if (!cmp_object_as_map(&obj, &map_size)) {
            ESP_LOGE(TAG, "Failed to get map size for connection message");
            return;
        }

        // Look for the "id" field
        bool found_id = false;
        for (uint32_t i = 0; i < map_size; i++) {
            // Read the key
            char key[32] = { 0 };
            uint32_t key_size = sizeof(key);

            if (!cmp_read_object(&cmp, &obj) || !cmp_object_is_str(&obj)) {
                ESP_LOGE(TAG, "Expected string key at position %" PRIu32 " in connection message", i);
                return;
            }

            if (!cmp_object_as_str(&obj, &key_size)) {
                ESP_LOGE(TAG, "Failed to get key size at position %" PRIu32 " in connection message", i);
                return;
            }

            if (key_size >= sizeof(key)) {
                ESP_LOGE(TAG, "Key too long at position %" PRIu32 " in connection message", i);
                return;
            }

            if (!cmp_object_to_str(&cmp, &obj, key, sizeof(key))) {
                ESP_LOGE(TAG, "Failed to read key string at position %" PRIu32 " in connection message", i);
                return;
            }

            // Check if this is the "id" key
            if (strcmp(key, "id") == 0) {
                // Read the value
                if (!cmp_read_object(&cmp, &obj) || !cmp_object_is_str(&obj)) {
                    ESP_LOGE(TAG, "Expected string value for 'id'");
                    return;
                }

                uint32_t id_size = sizeof(client_id) - 1;  // Leave room for null terminator
                if (!cmp_object_to_str(&cmp, &obj, client_id, sizeof(client_id))) {
                    ESP_LOGE(TAG, "Failed to read 'id' value");
                    return;
                }

                found_id = true;
                ESP_LOGI(TAG, "Received client ID: %s", client_id);
                break;
            }
            else {
                // Skip the value for other keys
                if (!cmp_read_object(&cmp, &obj)) {
                    ESP_LOGE(TAG, "Failed to read value for key '%s'", key);
                    return;
                }
            }
        }

        if (!found_id) {
            ESP_LOGW(TAG, "Connection message but no client ID found");
        }
    }
    else if (strcmp(type_str, "echo") == 0) {
        ESP_LOGI(TAG, "Received echo message");
    }
    else if (strcmp(type_str, "broadcast") == 0) {
        ESP_LOGI(TAG, "Received broadcast message");
    }
    else if (strcmp(type_str, "command") == 0) {
        // Need to reread the entire message to find the "command" field
        buffer_ptr = data;  // Reset the buffer pointer
        cmp_init(&cmp, (void*)&buffer_ptr, buffer_reader, buffer_skipper, buffer_writer);

        // Read the map header again
        if (!cmp_read_object(&cmp, &obj) || !cmp_object_is_map(&obj)) {
            ESP_LOGE(TAG, "Failed to re-read map header for command message");
            return;
        }

        if (!cmp_object_as_map(&obj, &map_size)) {
            ESP_LOGE(TAG, "Failed to get map size for command message");
            return;
        }

        // Look for the "command" field
        bool found_command = false;
        char command[32] = { 0 };

        for (uint32_t i = 0; i < map_size; i++) {
            // Read the key
            char key[32] = { 0 };
            uint32_t key_size = sizeof(key);

            if (!cmp_read_object(&cmp, &obj) || !cmp_object_is_str(&obj)) {
                ESP_LOGE(TAG, "Expected string key at position %" PRIu32 " in command message", i);
                return;
            }

            if (!cmp_object_as_str(&obj, &key_size)) {
                ESP_LOGE(TAG, "Failed to get key size at position %" PRIu32 " in command message", i);
                return;
            }

            if (key_size >= sizeof(key)) {
                ESP_LOGE(TAG, "Key too long at position %" PRIu32 " in command message", i);
                return;
            }

            if (!cmp_object_to_str(&cmp, &obj, key, sizeof(key))) {
                ESP_LOGE(TAG, "Failed to read key string at position %" PRIu32 " in command message", i);
                return;
            }

            // Check if this is the "command" key
            if (strcmp(key, "command") == 0) {
                // Read the value
                if (!cmp_read_object(&cmp, &obj) || !cmp_object_is_str(&obj)) {
                    ESP_LOGE(TAG, "Expected string value for 'command'");
                    return;
                }

                uint32_t cmd_size = sizeof(command) - 1;  // Leave room for null terminator
                if (!cmp_object_to_str(&cmp, &obj, command, sizeof(command))) {
                    ESP_LOGE(TAG, "Failed to read 'command' value");
                    return;
                }

                found_command = true;
                ESP_LOGI(TAG, "Received command: %s", command);
                break;
            }
            else {
                // Skip the value for other keys
                if (!cmp_read_object(&cmp, &obj)) {
                    ESP_LOGE(TAG, "Failed to read value for key '%s'", key);
                    return;
                }
            }
        }

        if (found_command) {
            // Process command
            if (strcmp(command, "restart") == 0) {
                ESP_LOGI(TAG, "Restarting device...");
                esp_restart();
            }
            else if (strcmp(command, "sleep") == 0) {
                ESP_LOGI(TAG, "Sleep command received (not implemented)");
            }
            else {
                ESP_LOGW(TAG, "Unknown command: %s", command);
            }
        }
        else {
            ESP_LOGW(TAG, "Command message but no command field found");
        }
    }
    else {
        ESP_LOGW(TAG, "Unknown message type: %s", type_str);
    }
}

// Create and send a sensor data message
static void send_sensor_data(esp_websocket_client_handle_t client,
    const char* sensor_type,
    float value,
    const char* unit) {

    if (strlen(client_id) == 0) {
        ESP_LOGW(TAG, "Not sending sensor data - not connected yet");
        return;
    }

    ESP_LOGI(TAG, "Preparing to send sensor data: %s = %f %s",
        sensor_type, value, unit ? unit : "");

    // Create buffer for MessagePack data
    uint8_t buffer[MSGPACK_BUFFER_SIZE];
    uint8_t* buffer_ptr = buffer;

    // Initialize CMP context
    cmp_ctx_t cmp;
    cmp_init(&cmp, (void*)&buffer_ptr, buffer_reader, buffer_skipper, buffer_writer);

    // Calculate map size - add unit field if provided
    uint32_t map_size = unit ? 4 : 3;

    // Write the map header
    if (!cmp_write_map(&cmp, map_size)) {
        ESP_LOGE(TAG, "Failed to write map header");
        return;
    }

    // Write "type" field
    if (!cmp_write_str(&cmp, "type", 4) ||
        !cmp_write_str(&cmp, "sensorData", 10)) {
        ESP_LOGE(TAG, "Failed to write type field");
        return;
    }

    // Write "sensorType" field
    if (!cmp_write_str(&cmp, "sensorType", 10) ||
        !cmp_write_str(&cmp, sensor_type, strlen(sensor_type))) {
        ESP_LOGE(TAG, "Failed to write sensorType field");
        return;
    }

    // Write "value" field
    if (!cmp_write_str(&cmp, "value", 5) ||
        !cmp_write_float(&cmp, value)) {
        ESP_LOGE(TAG, "Failed to write value field");
        return;
    }

    // Write "unit" field if provided
    if (unit) {
        if (!cmp_write_str(&cmp, "unit", 4) ||
            !cmp_write_str(&cmp, unit, strlen(unit))) {
            ESP_LOGE(TAG, "Failed to write unit field");
            return;
        }
    }

    // Calculate the size of data written
    size_t data_size = buffer_ptr - buffer;

    // Send the MessagePack data
    esp_err_t err = esp_websocket_client_send_bin(client, (const char*)buffer, data_size, portMAX_DELAY);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully sent sensor data: %s = %f %s (packed size: %zu bytes)",
            sensor_type, value, unit ? unit : "", data_size);
    }
    else {
        ESP_LOGE(TAG, "Failed to send sensor data: %s", esp_err_to_name(err));
    }
}

static void websocket_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_websocket_event_data_t* data = (esp_websocket_event_data_t*)event_data;
    esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)handler_args;

    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        // Clear client ID on disconnection
        memset(client_id, 0, sizeof(client_id));
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
        break;
    default:
        ESP_LOGI(TAG, "Other WebSocket event: %ld", event_id);
        break;
    }
}

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
    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        return;
    }

    // Register event handler
    ESP_LOGI(TAG, "Registering WebSocket event handler");
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void*)client);

    // Start the websocket client
    ESP_LOGI(TAG, "Starting WebSocket client");
    esp_err_t err = esp_websocket_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WebSocket client: %s", esp_err_to_name(err));
        esp_websocket_client_destroy(client);
        return;
    }

    ESP_LOGI(TAG, "WebSocket client started successfully");

    // Send sensor data periodically
    int cycle = 0;
    while (1) {
        // Only send data if connected and we have a client ID
        if (esp_websocket_client_is_connected(client)) {
            if (strlen(client_id) > 0) {
                // Generate simulated sensor readings with some variation
                float temperature = 24.5 + ((float)esp_random() / UINT32_MAX * 2.0 - 1.0);
                float humidity = 45.0 + ((float)esp_random() / UINT32_MAX * 5.0 - 2.5);

                // Send temperature every cycle
                send_sensor_data(client, "temperature", temperature, "C");

                // Send humidity every 3 cycles
                if (cycle % 3 == 0) {
                    send_sensor_data(client, "humidity", humidity, "%");
                }

                cycle++;
            }
            else {
                ESP_LOGW(TAG, "Connected but no client ID yet, waiting...");
            }
        }
        else {
            ESP_LOGW(TAG, "WebSocket not connected, skipping sensor data transmission");
        }

        // Wait for the next sampling interval
        vTaskDelay(pdMS_TO_TICKS(SENSOR_SAMPLING_INTERVAL));
    }

    // This code won't be reached in this example, but here for completeness
    ESP_LOGI(TAG, "Stopping WebSocket client");
    esp_websocket_client_stop(client);
    ESP_LOGI(TAG, "Destroying WebSocket client");
    esp_websocket_client_destroy(client);
}