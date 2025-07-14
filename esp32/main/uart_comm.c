#include "uart_comm.h"

static ack_tracker_t ack_tracker = { 0 };
static const char* TAG = "UART_COMM";

// Global variables
static QueueHandle_t uart_queue = NULL;
static TaskHandle_t uart_task_handle = NULL;
static bool uart_initialized = false;

// Statistics
static struct {
    uint32_t messages_sent;
    uint32_t messages_received;
    uint32_t errors;
    uint32_t checksum_errors;
} uart_stats = { 0 };

// Callback functions
static uart_message_callback_t message_callback = NULL;
static uart_game_data_callback_t game_data_callback = NULL;
static uart_chat_message_callback_t chat_message_callback = NULL;
static uart_command_callback_t command_callback = NULL;
static uart_status_callback_t status_callback = NULL;
static uart_connection_message_callback_t connection_message_callback = NULL;
static uart_tile_size_validation_callback_t tile_size_validation_callback = NULL;

// Private function declarations
static void uart_event_task(void* pvParameters);
static uint8_t calculate_checksum(const uint8_t* data, size_t length);
static bool validate_message(const uart_message_t* msg);
static void process_received_message(const uart_message_t* msg);
static esp_err_t uart_send_raw_message(const uart_message_t* msg);
static void print_message_hex(const char* prefix, const uart_message_t* msg);
void debug_struct_sizes(void);

void debug_struct_sizes(void) {
    ESP_LOGI(TAG, "=== ESP32 STRUCT SIZE DEBUG ===");
    ESP_LOGI(TAG, "uart_message_t size: %zu bytes", sizeof(uart_message_t));
    ESP_LOGI(TAG, "uart_game_data_t size: %zu bytes", sizeof(uart_game_data_t));
    ESP_LOGI(TAG, "uart_chat_message_t size: %zu bytes", sizeof(uart_chat_message_t));
    ESP_LOGI(TAG, "uart_command_t size: %zu bytes", sizeof(uart_command_t));
    ESP_LOGI(TAG, "uart_status_t size: %zu bytes", sizeof(uart_status_t));

    uart_message_t test_msg;
    ESP_LOGI(TAG, "start_byte offset: %zu", (size_t)&test_msg.start_byte - (size_t)&test_msg);
    ESP_LOGI(TAG, "msg_type offset: %zu", (size_t)&test_msg.msg_type - (size_t)&test_msg);
    ESP_LOGI(TAG, "length offset: %zu", (size_t)&test_msg.length - (size_t)&test_msg);
    ESP_LOGI(TAG, "data offset: %zu", (size_t)&test_msg.data - (size_t)&test_msg);
    ESP_LOGI(TAG, "checksum offset: %zu", (size_t)&test_msg.checksum - (size_t)&test_msg);
    ESP_LOGI(TAG, "end_byte offset: %zu", (size_t)&test_msg.end_byte - (size_t)&test_msg);
    ESP_LOGI(TAG, "===============================");
}

// Initialize ACK tracking system
esp_err_t uart_init_ack_system(void) {
    ack_tracker.ack_semaphore = xSemaphoreCreateBinary();
    if (ack_tracker.ack_semaphore == NULL) {
        ESP_LOGE(TAG, "Failed to create ACK semaphore");
        return ESP_FAIL;
    }
    ack_tracker.waiting_for_ack = false;
    ESP_LOGI(TAG, "ACK tracking system initialized");
    return ESP_OK;
}


// Deinitialize ACK tracking system
void uart_deinit_ack_system(void) {
    if (ack_tracker.ack_semaphore) {
        vSemaphoreDelete(ack_tracker.ack_semaphore);
        ack_tracker.ack_semaphore = NULL;
    }
    ack_tracker.waiting_for_ack = false;
}

// Calculate simple checksum
static uint8_t calculate_checksum(const uint8_t* data, size_t length) {
    uint8_t checksum = 0;
    // Calculate checksum for all bytes except checksum and end_byte (last 2 bytes)
    for (size_t i = 0; i < length - 2; i++) {
        checksum ^= data[i];
    }
    ESP_LOGD(TAG, "Calculated checksum: 0x%02X for %zu bytes", checksum, length - 2);
    return checksum;
}

// Validate received message
static bool validate_message(const uart_message_t* msg) {
    // Check start and end bytes
    if (msg->start_byte != 0xAA || msg->end_byte != 0x55) {
        ESP_LOGW(TAG, "Invalid message delimiters: start=0x%02X, end=0x%02X",
            msg->start_byte, msg->end_byte);
        return false;
    }

    // Check message type
    if (msg->msg_type < UART_MSG_GAME_DATA || msg->msg_type > UART_MSG_TILE_SIZE_VALIDATION) {
        ESP_LOGW(TAG, "Invalid message type: 0x%02X", msg->msg_type);
        return false;
    }


    // Check length
    if (msg->length > 250) {
        ESP_LOGW(TAG, "Invalid message length: %d", msg->length);
        return false;
    }

    // Calculate and verify checksum
    uint8_t calculated_checksum = calculate_checksum((const uint8_t*)msg,
        sizeof(uart_message_t) - 1);
    if (msg->checksum != calculated_checksum) {
        ESP_LOGW(TAG, "Checksum mismatch: expected=0x%02X, calculated=0x%02X",
            msg->checksum, calculated_checksum);
        uart_stats.checksum_errors++;
        return false;
    }

    return true;
}

// Helper function for debugging
static void print_message_hex(const char* prefix, const uart_message_t* msg) {
    ESP_LOGI(TAG, "%s - Message hex dump:", prefix);
    uint8_t* bytes = (uint8_t*)msg;
    for (int i = 0; i < sizeof(uart_message_t); i += 16) {
        char hex_line[64] = { 0 };
        char ascii_line[17] = { 0 };
        for (int j = 0; j < 16 && (i + j) < sizeof(uart_message_t); j++) {
            sprintf(hex_line + j * 3, "%02X ", bytes[i + j]);
            ascii_line[j] = (bytes[i + j] >= 32 && bytes[i + j] <= 126) ? bytes[i + j] : '.';
        }
        ESP_LOGI(TAG, "%04X: %-48s %s", i, hex_line, ascii_line);
    }
}

// Process received message
static void process_received_message(const uart_message_t* msg) {
    ESP_LOGI(TAG, "Processing message type: 0x%02X, length: %d",
        msg->msg_type, msg->length);

    // Call generic message callback if registered
    if (message_callback) {
        message_callback((uart_message_type_t)msg->msg_type, msg->data, msg->length);
    }

    // Process specific message types
    switch (msg->msg_type) {
    case UART_MSG_GAME_DATA:
        if (game_data_callback && msg->length == sizeof(uart_game_data_t)) {
            uart_game_data_t* game_data = (uart_game_data_t*)msg->data;
            ESP_LOGI(TAG, "Game Data: type=%s, data=%s", game_data->data_type, game_data->game_data);
            game_data_callback(game_data);
        }
        break;

    case UART_MSG_CHAT:
        if (chat_message_callback && msg->length == sizeof(uart_chat_message_t)) {
            uart_chat_message_t* chat_message = (uart_chat_message_t*)msg->data;
            ESP_LOGI(TAG, "Chat Message: from=%s, type=%s, message=%s",
                chat_message->sender, chat_message->chat_type, chat_message->message);
            chat_message_callback(chat_message);
        }
        break;

    case UART_MSG_COMMAND:
        if (command_callback && msg->length == sizeof(uart_command_t)) {
            uart_command_t* command = (uart_command_t*)msg->data;
            ESP_LOGI(TAG, "Received command: %s %s", command->command, command->parameters);
            command_callback(command);
        }
        break;

    case UART_MSG_STATUS:
        if (status_callback && msg->length == sizeof(uart_status_t)) {
            uart_status_t* status = (uart_status_t*)msg->data;
            ESP_LOGI(TAG, "Received status: system=%d, error=%d, message=%s",
                status->system_status, status->error_code, status->status_message);
            status_callback(status);
        }
        break;

    case UART_MSG_CONNECTION:
        if (connection_message_callback && msg->length == sizeof(uart_connection_message_t)) {
            uart_connection_message_t* connection_msg = (uart_connection_message_t*)msg->data;
            ESP_LOGI(TAG, "Connection Message: message=%s, timestamp=%lu",
                connection_msg->message, connection_msg->timestamp);
            connection_message_callback(connection_msg);
        }
        break;

    case UART_MSG_TILE_SIZE_VALIDATION:
        if (tile_size_validation_callback && msg->length == sizeof(uart_tile_size_validation_t)) {
            uart_tile_size_validation_t* tile_size_msg = (uart_tile_size_validation_t*)msg->data;
            ESP_LOGI(TAG, "Tile Size Validation: tile_size=%d, timestamp=%lu",
                tile_size_msg->tile_size, tile_size_msg->timestamp);
            tile_size_validation_callback(tile_size_msg);
        }
        break;

    case UART_MSG_ACK:
        ESP_LOGI(TAG, "Received ACK from STM32");

        // Signal ACK received if we're waiting for one
        if (ack_tracker.waiting_for_ack && ack_tracker.ack_semaphore) {
            BaseType_t higher_priority_task_woken = pdFALSE;
            xSemaphoreGiveFromISR(ack_tracker.ack_semaphore, &higher_priority_task_woken);
            if (higher_priority_task_woken) {
                portYIELD_FROM_ISR();
            }
        }
        break;

    case UART_MSG_NACK:
        ESP_LOGI(TAG, "Received NACK from STM32");
        break;

    case UART_MSG_HEARTBEAT:
        ESP_LOGI(TAG, "Received heartbeat from STM32");
        // Automatically respond to heartbeat
        uart_send_ack();
        break;

    default:
        ESP_LOGW(TAG, "Unknown message type: 0x%02X", msg->msg_type);
        break;
    }

    uart_stats.messages_received++;
}

// Send raw message
static esp_err_t uart_send_raw_message(const uart_message_t* msg) {
    if (!uart_initialized) {
        ESP_LOGE(TAG, "UART not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // print_message_hex("SENDING", msg);

    int bytes_written = uart_write_bytes(UART_PORT_NUM, msg, sizeof(uart_message_t));
    if (bytes_written != sizeof(uart_message_t)) {
        ESP_LOGE(TAG, "Failed to write complete message: %d/%zu bytes",
            bytes_written, sizeof(uart_message_t));
        uart_stats.errors++;
        return ESP_FAIL;
    }

    // Add small delay to ensure data is transmitted
    vTaskDelay(pdMS_TO_TICKS(10));

    uart_stats.messages_sent++;
    ESP_LOGI(TAG, "Sent message type: 0x%02X, length: %d, total size: %zu",
        msg->msg_type, msg->length, sizeof(uart_message_t));
    return ESP_OK;
}

// UART event task
static void uart_event_task(void* pvParameters) {
    uart_event_t event;
    uint8_t* data_buffer = (uint8_t*)malloc(UART_RX_BUFFER_SIZE);
    uart_message_t msg;
    size_t msg_index = 0;
    bool receiving_message = false;

    if (!data_buffer) {
        ESP_LOGE(TAG, "Failed to allocate data buffer");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "UART event task started");

    while (1) {
        if (xQueueReceive(uart_queue, (void*)&event, pdMS_TO_TICKS(100))) {
            switch (event.type) {
            case UART_DATA:
            {
                int len = uart_read_bytes(UART_PORT_NUM, data_buffer,
                    event.size, pdMS_TO_TICKS(100));

                if (len > 0) {
                    // Process received bytes
                    for (int i = 0; i < len; i++) {
                        uint8_t byte = data_buffer[i];

                        if (!receiving_message && byte == 0xAA) {
                            // Start of new message
                            receiving_message = true;
                            msg_index = 0;
                            ((uint8_t*)&msg)[msg_index++] = byte;
                        }
                        else if (receiving_message) {
                            ((uint8_t*)&msg)[msg_index++] = byte;

                            // Check if we have a complete message
                            if (msg_index >= sizeof(uart_message_t)) {
                                if (validate_message(&msg)) {
                                    process_received_message(&msg);
                                }
                                else {
                                    uart_stats.errors++;
                                }
                                receiving_message = false;
                                msg_index = 0;
                            }

                            // Reset if message is too long
                            if (msg_index >= sizeof(uart_message_t)) {
                                receiving_message = false;
                                msg_index = 0;
                                ESP_LOGW(TAG, "Message too long, resetting");
                            }
                        }
                    }
                }
            }
            break;

            case UART_FIFO_OVF:
                ESP_LOGW(TAG, "UART FIFO overflow");
                uart_flush_input(UART_PORT_NUM);
                xQueueReset(uart_queue);
                uart_stats.errors++;
                break;

            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "UART ring buffer full");
                uart_flush_input(UART_PORT_NUM);
                xQueueReset(uart_queue);
                uart_stats.errors++;
                break;

            case UART_BREAK:
                ESP_LOGW(TAG, "UART break detected");
                break;

            case UART_PARITY_ERR:
                ESP_LOGW(TAG, "UART parity error");
                uart_stats.errors++;
                break;

            case UART_FRAME_ERR:
                ESP_LOGW(TAG, "UART frame error");
                uart_stats.errors++;
                break;

            case UART_PATTERN_DET:
                ESP_LOGD(TAG, "UART pattern detected");
                break;

            default:
                ESP_LOGD(TAG, "UART event type: %d", event.type);
                break;
            }
        }
    }

    free(data_buffer);
    vTaskDelete(NULL);
}

// Public function implementations
esp_err_t uart_comm_init(void) {
    if (uart_initialized) {
        ESP_LOGW(TAG, "UART already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing UART communication");

    // Configure UART parameters
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_BITS,
        .parity = UART_PARITY,
        .stop_bits = UART_STOP_BITS,
        .flow_ctrl = UART_FLOW_CTRL,
        .source_clk = UART_SOURCE_CLK,
    };

    // Install UART driver
    esp_err_t ret = uart_driver_install(UART_PORT_NUM, UART_RX_BUFFER_SIZE,
        UART_TX_BUFFER_SIZE, UART_QUEUE_SIZE,
        &uart_queue, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure UART parameters
    ret = uart_param_config(UART_PORT_NUM, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters: %s", esp_err_to_name(ret));
        uart_driver_delete(UART_PORT_NUM);
        return ret;
    }

    // Set UART pins
    ret = uart_set_pin(UART_PORT_NUM, UART_TXD_PIN, UART_RXD_PIN,
        UART_RTS_PIN, UART_CTS_PIN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(ret));
        uart_driver_delete(UART_PORT_NUM);
        return ret;
    }

    // Enable pattern detection
    uart_enable_pattern_det_baud_intr(UART_PORT_NUM, UART_PATTERN_CHR,
        UART_PATTERN_QUEUE_SIZE, 9, 0, 0);

    // Create UART event task
    BaseType_t task_ret = xTaskCreate(uart_event_task, "uart_events",
        4096, NULL, 12, &uart_task_handle);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UART event task");
        uart_driver_delete(UART_PORT_NUM);
        return ESP_FAIL;
    }

    uart_initialized = true;
    ESP_LOGI(TAG, "UART communication initialized successfully");
    ESP_LOGI(TAG, "TXD: GPIO%d, RXD: GPIO%d, Baud: %d",
        UART_TXD_PIN, UART_RXD_PIN, UART_BAUD_RATE);

    // Check message length
    debug_struct_sizes();

    // Initialize ACK tracking system
    esp_err_t ack_result = uart_init_ack_system();
    if (ack_result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ACK system");
        uart_driver_delete(UART_PORT_NUM);
        return ack_result;
    }

    uart_initialized = true;
    ESP_LOGI(TAG, "UART communication initialized successfully");

    return ESP_OK;
}

esp_err_t uart_comm_deinit(void) {
    if (!uart_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing UART communication");

    // Delete task
    if (uart_task_handle) {
        vTaskDelete(uart_task_handle);
        uart_task_handle = NULL;
    }

    // Cleanup ACK system
    uart_deinit_ack_system();

    // Delete driver
    uart_driver_delete(UART_PORT_NUM);
    uart_queue = NULL;
    uart_initialized = false;

    ESP_LOGI(TAG, "UART communication deinitialized");
    return ESP_OK;
}

esp_err_t uart_send_message(uart_message_type_t type, const uint8_t* data, size_t length) {
    if (length > 250) {
        ESP_LOGE(TAG, "Message data too long: %zu bytes (max 250)", length);
        return ESP_ERR_INVALID_ARG;
    }

    uart_message_t msg;
    memset(&msg, 0, sizeof(msg)); // Clear the entire structure

    msg.start_byte = 0xAA;
    msg.msg_type = (uint8_t)type;
    msg.length = (uint8_t)length;

    if (data && length > 0) {
        memcpy(msg.data, data, length);
    }

    msg.checksum = calculate_checksum((const uint8_t*)&msg, sizeof(uart_message_t));
    msg.end_byte = 0x55;

    return uart_send_raw_message(&msg);
}

esp_err_t uart_send_game_data(const char* data_type, const char* game_data, const char* metadata) {
    uart_game_data_t game_payload;
    memset(&game_payload, 0, sizeof(game_payload));

    strncpy(game_payload.data_type, data_type, sizeof(game_payload.data_type) - 1);
    strncpy(game_payload.game_data, game_data, sizeof(game_payload.game_data) - 1);
    if (metadata) {
        strncpy(game_payload.metadata, metadata, sizeof(game_payload.metadata) - 1);
    }
    game_payload.sequence_num = esp_log_timestamp();

    ESP_LOGI(TAG, "Sending game data: type='%s', data='%s', meta='%s', seq=%lu",
        data_type, game_data, metadata ? metadata : "", game_payload.sequence_num);

    return uart_send_message(UART_MSG_GAME_DATA, (const uint8_t*)&game_payload,
        sizeof(game_payload));
}

esp_err_t uart_send_chat_message(const char* message, const char* sender, const char* chat_type) {
    uart_chat_message_t chat_payload;
    memset(&chat_payload, 0, sizeof(chat_payload));

    strncpy(chat_payload.message, message, sizeof(chat_payload.message) - 1);
    strncpy(chat_payload.sender, sender ? sender : "unknown", sizeof(chat_payload.sender) - 1);
    strncpy(chat_payload.chat_type, chat_type ? chat_type : "general", sizeof(chat_payload.chat_type) - 1);
    chat_payload.timestamp = esp_log_timestamp();

    ESP_LOGI(TAG, "Sending chat message: from='%s', type='%s', message='%s'",
        chat_payload.sender, chat_payload.chat_type, chat_payload.message);

    return uart_send_message(UART_MSG_CHAT, (const uint8_t*)&chat_payload,
        sizeof(chat_payload));
}

esp_err_t uart_send_command(const char* command, const char* parameters) {
    uart_command_t cmd = { 0 };

    strncpy(cmd.command, command, sizeof(cmd.command) - 1);
    if (parameters) {
        strncpy(cmd.parameters, parameters, sizeof(cmd.parameters) - 1);
    }

    return uart_send_message(UART_MSG_COMMAND, (const uint8_t*)&cmd, sizeof(cmd));
}

esp_err_t uart_send_status(system_status_type_t system_status, uint8_t error_code, const char* message) {
    uart_status_t status = { 0 };

    status.system_status = system_status;  // Now uses enum directly
    status.error_code = error_code;
    if (message) {
        strncpy(status.status_message, message, sizeof(status.status_message) - 1);
    }

    return uart_send_message(UART_MSG_STATUS, (const uint8_t*)&status, sizeof(status));
}

// Send status message and wait for ACK
esp_err_t uart_send_status_with_ack(system_status_type_t system_status, uint8_t error_code,
    const char* message, uint32_t timeout_ms) {
    if (!uart_initialized || !ack_tracker.ack_semaphore) {
        ESP_LOGE(TAG, "UART or ACK system not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Ensure we're not already waiting for an ACK
    if (ack_tracker.waiting_for_ack) {
        ESP_LOGW(TAG, "Already waiting for ACK, ignoring new request");
        return ESP_ERR_INVALID_STATE;
    }

    // Set up ACK tracking
    ack_tracker.waiting_for_ack = true;
    ack_tracker.ack_start_time = xTaskGetTickCount();
    ack_tracker.timeout_ms = timeout_ms;

    // Send the status message
    esp_err_t send_result = uart_send_status(system_status, error_code, message);
    if (send_result != ESP_OK) {
        ack_tracker.waiting_for_ack = false;
        ESP_LOGE(TAG, "Failed to send status message");
        return send_result;
    }

    ESP_LOGI(TAG, "Status sent, waiting for ACK (timeout: %lu ms)", timeout_ms);

    // Wait for ACK with timeout
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);
    BaseType_t result = xSemaphoreTake(ack_tracker.ack_semaphore, timeout_ticks);

    ack_tracker.waiting_for_ack = false;

    if (result == pdTRUE) {
        ESP_LOGI(TAG, "ACK received successfully");
        return ESP_OK;
    }
    else {
        ESP_LOGW(TAG, "ACK timeout after %lu ms", timeout_ms);
        return ESP_ERR_TIMEOUT;
    }
}

esp_err_t uart_send_connection_message(const char* client_id, const char* message) {
    uart_connection_message_t connection_payload;
    memset(&connection_payload, 0, sizeof(connection_payload));

    strncpy(connection_payload.client_id, client_id, sizeof(connection_payload.client_id) - 1);
    strncpy(connection_payload.message, message, sizeof(connection_payload.message) - 1);
    connection_payload.timestamp = esp_log_timestamp();

    ESP_LOGI(TAG, "Sending connection message: message='%s', timestamp=%lu",
        connection_payload.message, connection_payload.timestamp);

    return uart_send_message(UART_MSG_CONNECTION, (const uint8_t*)&connection_payload,
        sizeof(connection_payload));
}

esp_err_t uart_send_tile_size_validation(uint16_t tile_size) {
    uart_tile_size_validation_t tile_size_payload;
    memset(&tile_size_payload, 0, sizeof(tile_size_payload));

    tile_size_payload.tile_size = tile_size;
    tile_size_payload.timestamp = esp_log_timestamp();

    ESP_LOGI(TAG, "Sending tile size validation: tile_size=%d, timestamp=%lu",
        tile_size_payload.tile_size, tile_size_payload.timestamp);

    return uart_send_message(UART_MSG_TILE_SIZE_VALIDATION, (const uint8_t*)&tile_size_payload,
        sizeof(tile_size_payload));
}

esp_err_t uart_send_ack(void) {
    return uart_send_message(UART_MSG_ACK, NULL, 0);
}

esp_err_t uart_send_nack(void) {
    return uart_send_message(UART_MSG_NACK, NULL, 0);
}

esp_err_t uart_send_heartbeat(void) {
    return uart_send_message(UART_MSG_HEARTBEAT, NULL, 0);
}

// Callback registration functions
void uart_register_message_callback(uart_message_callback_t callback) {
    message_callback = callback;
}

void uart_register_game_data_callback(uart_game_data_callback_t callback) {
    game_data_callback = callback;
}

void uart_register_chat_message_callback(uart_chat_message_callback_t callback) {
    chat_message_callback = callback;
}

void uart_register_command_callback(uart_command_callback_t callback) {
    command_callback = callback;
}

void uart_register_status_callback(uart_status_callback_t callback) {
    status_callback = callback;
}

void uart_register_connection_message_callback(uart_connection_message_callback_t callback) {
    connection_message_callback = callback;
}

void uart_register_tile_size_validation_callback(uart_tile_size_validation_callback_t callback) {
    tile_size_validation_callback = callback;
}


// Utility functions
bool uart_is_connected(void) {
    return uart_initialized;
}

void uart_flush_buffers(void) {
    if (uart_initialized) {
        uart_flush(UART_PORT_NUM);
    }
}

void uart_print_stats(void) {
    ESP_LOGI(TAG, "UART Statistics:");
    ESP_LOGI(TAG, "  Messages sent: %" PRIu32, uart_stats.messages_sent);
    ESP_LOGI(TAG, "  Messages received: %" PRIu32, uart_stats.messages_received);
    ESP_LOGI(TAG, "  Errors: %" PRIu32, uart_stats.errors);
    ESP_LOGI(TAG, "  Checksum errors: %" PRIu32, uart_stats.checksum_errors);
}