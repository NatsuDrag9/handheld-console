/*
 * serial_comm.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 *
 */

#include "Console_Peripherals/Hardware/serial_comm.h"
#include "Utils/debug_conf.h"
#include "Utils/comm_utils.h"

/* Communication handles */
static circular_buffer_t rx_buffer;
static message_queue_t msg_queue;
static message_parser_t parser;
static comm_stats_t stats;

/* Message storage */
static uart_message_t message_storage[MESSAGE_QUEUE_SIZE];
static uint8_t parse_buffer[sizeof(uart_message_t)];

/* Protocol state */
static ProtocolState current_state = PROTO_STATE_INIT;
static uint32_t last_activity_time = 0;
static uint32_t last_heartbeat_time = 0;
//static const uint32_t HEARTBEAT_INTERVAL_MS = 5000;

/* Connection status */
static bool is_esp32_ready = false;
static bool is_wifi_connected = false;
static bool is_websocket_connected = false;
static volatile bool ui_needs_status_update = false;

/* Callback functions */
static game_data_received_callback_t game_data_callback = NULL;
static chat_message_received_callback_t chat_message_callback = NULL;
static command_received_callback_t command_callback = NULL;
static status_received_callback_t status_callback = NULL;

/* Private function prototypes */
static void UART_RxCallback(uint8_t data);
static uint8_t calculate_checksum(const uart_message_t* msg);
static bool validate_message(const uart_message_t* msg);
static void process_received_message(const uart_message_t* msg);
static UART_Status send_raw_message(const uart_message_t* msg);
static void handle_esp32_status(const uart_status_t* status);
static void handle_esp32_command(const uart_command_t* command);
static void handle_game_data(const uart_game_data_t* game_data);
static void handle_chat_message(const uart_chat_message_t* chat_message);
static void parse_incoming_data(void);

/* Calculate checksum to match ESP32 */
static uint8_t calculate_checksum(const uart_message_t* msg)
{
    uint8_t checksum = 0;
    const uint8_t* data = (const uint8_t*)msg;

    // Calculate for all bytes except checksum and end_byte (last 2 bytes)
    size_t total_length = sizeof(uart_message_t);
    for (size_t i = 0; i < total_length - 2; i++) {
        checksum ^= data[i];
    }

    return checksum;
}

/* Validate received message */
static bool validate_message(const uart_message_t* msg)
{
    /* Check start and end bytes */
    if (msg->start_byte != MSG_START_BYTE || msg->end_byte != MSG_END_BYTE) {
        DEBUG_PRINTF(false, "Invalid delimiters: start=0x%02X, end=0x%02X\r\n",
                     msg->start_byte, msg->end_byte);
        return false;
    }

    /* Check message type */
    if (msg->msg_type < MSG_TYPE_DATA || msg->msg_type > MSG_TYPE_CHAT) {
        DEBUG_PRINTF(false, "Invalid message type: 0x%02X\r\n", msg->msg_type);
        return false;
    }

    /* Check length */
    if (msg->length > MAX_PAYLOAD_SIZE) {
        DEBUG_PRINTF(false, "Invalid message length: %d\r\n", msg->length);
        return false;
    }

    /* Calculate and verify checksum */
    uint8_t calculated_checksum = calculate_checksum(msg);
    if (msg->checksum != calculated_checksum) {
        DEBUG_PRINTF(false, "Checksum mismatch: expected=0x%02X, calculated=0x%02X\r\n",
                     msg->checksum, calculated_checksum);
        stats.parse_errors++;
        return false;
    }

    return true;
}

/* Parse incoming data from circular buffer */
static void parse_incoming_data(void)
{
    uint8_t data;

    while (circular_buffer_get(&rx_buffer, &data)) {
        if (message_parser_add_byte(&parser, data, MSG_START_BYTE)) {
            stats.bytes_received++;

            // Check if we have a complete message
            if (message_parser_is_complete(&parser)) {
                uart_message_t* msg = (uart_message_t*)message_parser_get_buffer(&parser);

                if (validate_message(msg)) {
                    if (message_queue_put(&msg_queue, msg)) {
                        DEBUG_PRINTF(false, "Message parsed and queued: type=0x%02X\r\n", msg->msg_type);
                        stats.messages_parsed++;
                    } else {
                        DEBUG_PRINTF(false, "Message queue full, dropping message\r\n");
                    }
                } else {
                    DEBUG_PRINTF(false, "Message validation failed\r\n");
                }
                message_parser_reset(&parser);
            }
        }
    }
}

/* UART receive callback - called from interrupt (keep minimal!) */
static void UART_RxCallback(uint8_t data)
{
    // Simply store data in circular buffer - keep ISR minimal
    circular_buffer_put(&rx_buffer, data);
}

/* Handle game data messages */
static void handle_game_data(const uart_game_data_t* game_data)
{
    DEBUG_PRINTF(false, "GAME DATA: Type=%.16s, Data=%.64s, Meta=%.32s\r\n",
                 game_data->data_type, game_data->game_data, game_data->metadata);

    // Call registered callback if available
    if (game_data_callback) {
        game_data_callback(game_data);
    }

    // Send ACK for received game data
    serial_comm_send_ack();
}

/* Handle chat messages */
static void handle_chat_message(const uart_chat_message_t* chat_message)
{
    DEBUG_PRINTF(false, "CHAT MESSAGE: From=%.32s, Type=%.16s, Message=%.96s\r\n",
                 chat_message->sender, chat_message->chat_type, chat_message->message);

    // Call registered callback if available
    if (chat_message_callback) {
        chat_message_callback(chat_message);
    }

    // Send ACK for received chat message
    serial_comm_send_ack();
}

/* Handle ESP32 status messages */
static void handle_esp32_status(const uart_status_t* status)
{
    DEBUG_PRINTF(false, "ESP32 Status: %d, Error: %d, Msg: %.32s\r\n",
                 status->system_status, status->error_code, status->status_message);

    switch (status->system_status) {
    case SYSTEM_STATUS_ESP32_STARTED:
        if (current_state == PROTO_STATE_INIT) {
            current_state = PROTO_STATE_ESP32_READY;
            is_esp32_ready = true;
            DEBUG_PRINTF(false, "ESP32 started and ready \r\n");
        }
        break;

    case SYSTEM_STATUS_ESP32_READY:
        current_state = PROTO_STATE_ESP32_READY;
        is_esp32_ready = true;
        DEBUG_PRINTF(false, "ESP32 initialization complete - sending ack\r\n");

        // CRITICAL: Send ACK immediately for handshake
        serial_comm_send_ack();
        DEBUG_PRINTF(false, "ESP32 critical handshake ACK sent\r\n");
        break;

    case SYSTEM_STATUS_WIFI_CONNECTING:
        if (current_state == PROTO_STATE_ESP32_READY) {
            current_state = PROTO_STATE_WIFI_CONNECTING;
            DEBUG_PRINTF(false, "ESP32 WiFi connecting\r\n");
        }
        break;

    case SYSTEM_STATUS_WIFI_CONNECTED:
        current_state = PROTO_STATE_WIFI_CONNECTED;
        is_wifi_connected = true;
        DEBUG_PRINTF(false, "ESP32 WiFi connected successfully\r\n");
        ui_needs_status_update = true;
        break;

    case SYSTEM_STATUS_WIFI_DISCONNECTED:
        current_state = PROTO_STATE_ESP32_READY;
        is_wifi_connected = false;
        is_websocket_connected = false;
        DEBUG_PRINTF(false, "ESP32 WiFi disconnected\r\n");
        ui_needs_status_update = true;
        break;

    case SYSTEM_STATUS_WEBSOCKET_CONNECTING:
        if (current_state == PROTO_STATE_WIFI_CONNECTED) {
            current_state = PROTO_STATE_WEBSOCKET_CONNECTING;
            DEBUG_PRINTF(false, "ESP32 WebSocket connecting\r\n");
        }
        break;

    case SYSTEM_STATUS_WEBSOCKET_CONNECTED:
        current_state = PROTO_STATE_WEBSOCKET_CONNECTED;
        is_websocket_connected = true;
        DEBUG_PRINTF(false, "ESP32 WebSocket connected successfully\r\n");
        break;

    case SYSTEM_STATUS_WEBSOCKET_DISCONNECTED:
        if (is_websocket_connected) {
            current_state = PROTO_STATE_WIFI_CONNECTED;
            is_websocket_connected = false;
            DEBUG_PRINTF(false, "ESP32 WebSocket disconnected\r\n");
        }
        break;

    case SYSTEM_STATUS_GAME_READY:
        current_state = PROTO_STATE_GAME_READY;
        DEBUG_PRINTF(false, "ESP32 game session ready\r\n");

        // Respond that STM32 game logic is ready
        serial_comm_send_status(SYSTEM_STATUS_STM32_GAME_READY, 0, "STM32_GAME_READY");
        break;

    case SYSTEM_STATUS_GAME_ACTIVE:
    	if (current_state == PROTO_STATE_GAME_READY) {
    		current_state = PROTO_STATE_GAME_ACTIVE;
    		DEBUG_PRINTF(false, "✓ Game session active\r\n");
    	}
        break;

    case SYSTEM_STATUS_GAME_ENDED:
    	if (current_state == PROTO_STATE_GAME_ACTIVE) {
    		current_state = PROTO_STATE_WEBSOCKET_CONNECTED;
    		DEBUG_PRINTF(false, "Game session ended - back to ready state\r\n");
    	}
        break;

    case SYSTEM_STATUS_OPPONENT_CONNECTED:
        DEBUG_PRINTF(false, "Opponent connected\r\n");
        break;

    case SYSTEM_STATUS_OPPONENT_DISCONNECTED:
        DEBUG_PRINTF(false, "Opponent disconnected\r\n");
        break;

    case SYSTEM_STATUS_PLAYER_ASSIGNMENT:
        DEBUG_PRINTF(false, "Player assignment received\r\n");
        break;

    case SYSTEM_STATUS_ERROR:
        current_state = PROTO_STATE_ERROR;
        is_wifi_connected = false;
        is_websocket_connected = false;
        DEBUG_PRINTF(false, "ESP32 error state\r\n");
        break;

    default:
        DEBUG_PRINTF(false, "Unknown ESP32 status: %d\r\n", status->system_status);
        break;
    }

    // Call registered callback if available
    if (status_callback) {
        status_callback(status);
    }
}


/* Handle ESP32 commands */
static void handle_esp32_command(const uart_command_t* command)
{
    DEBUG_PRINTF(false, "ESP32 Command: %.32s %.64s\r\n",
                 command->command, command->parameters);

    if (strncmp(command->command, "game_ready", 10) == 0) {
        current_state = PROTO_STATE_GAME_READY;
        DEBUG_PRINTF(false, "Game session ready\r\n");
        // Respond that STM32 game logic is ready
        serial_comm_send_status(SYSTEM_STATUS_STM32_GAME_READY, 0, "STM32_GAME_READY");
        serial_comm_send_ack();
    }
    else if (strncmp(command->command, "start_game", 10) == 0) {
        current_state = PROTO_STATE_GAME_ACTIVE;
        DEBUG_PRINTF(false, "Game session started\r\n");
        serial_comm_send_ack();
    }
    else if (strncmp(command->command, "end_game", 8) == 0) {
        current_state = PROTO_STATE_WEBSOCKET_CONNECTED;
        DEBUG_PRINTF(false, "Game session ended\r\n");
        serial_comm_send_ack();
    }
    else if (strncmp(command->command, "ping", 4) == 0) {
        DEBUG_PRINTF(false, "Ping received, sending ACK\r\n");
        serial_comm_send_ack();
    }
    else if (strncmp(command->command, "pong", 4) == 0) {
        DEBUG_PRINTF(false, "Pong received from ESP32\r\n");
    }
    else if (strncmp(command->command, "reset", 5) == 0) {
        DEBUG_PRINTF(false, "Reset command received\r\n");
        serial_comm_reset();
        serial_comm_send_ack();
    }

    // Call registered callback if available
    if (command_callback) {
        command_callback(command);
    }
}


/* Process received message */
static void process_received_message(const uart_message_t* msg)
{
    DEBUG_PRINTF(false, "Processing message type: 0x%02X, length: %d\r\n",
                 msg->msg_type, msg->length);

    switch (msg->msg_type) {
    case MSG_TYPE_DATA:
        if (msg->length == sizeof(uart_game_data_t)) {
            handle_game_data((uart_game_data_t*)msg->data);
        }
        break;

    case MSG_TYPE_CHAT:
        if (msg->length == sizeof(uart_chat_message_t)) {
            handle_chat_message((uart_chat_message_t*)msg->data);
        }
        break;

    case MSG_TYPE_COMMAND:
        if (msg->length == sizeof(uart_command_t)) {
            handle_esp32_command((uart_command_t*)msg->data);
        }
        break;

    case MSG_TYPE_STATUS:
        if (msg->length == sizeof(uart_status_t)) {
            handle_esp32_status((uart_status_t*)msg->data);
        }
        break;

    case MSG_TYPE_ACK:
        DEBUG_PRINTF(false, "Received ACK from ESP32\r\n");
        break;

    case MSG_TYPE_NACK:
        DEBUG_PRINTF(false, "Received NACK from ESP32\r\n");
        break;

    case MSG_TYPE_HEARTBEAT:
        DEBUG_PRINTF(false, "Received heartbeat from ESP32\r\n");
        serial_comm_send_ack();
        break;

    default:
        DEBUG_PRINTF(false, "Unknown message type: 0x%02X\r\n", msg->msg_type);
        break;
    }
}

/* Send raw message */
static UART_Status send_raw_message(const uart_message_t* msg)
{
    UART_Status status = UART_SendBuffer(UART_PORT_2, (uint8_t*)msg,
                                        sizeof(uart_message_t), 1000);
    if (status == UART_OK) {
        stats.bytes_sent += sizeof(uart_message_t);
        DEBUG_PRINTF(false, "Message sent successfully\r\n");
    }
    else {
        DEBUG_PRINTF(false, "Failed to send message, status: %d\r\n", status);
    }
    return status;
}

/* Public Function Implementations */
UART_Status serial_comm_init(void)
{
    /* Initialize all communication utilities */
    circular_buffer_init(&rx_buffer);
    message_queue_init(&msg_queue, message_storage, MESSAGE_QUEUE_SIZE, sizeof(uart_message_t));
    message_parser_init(&parser, parse_buffer, sizeof(uart_message_t));
    comm_stats_init(&stats);

    /* Initialize protocol state */
    current_state = PROTO_STATE_INIT;
    is_esp32_ready = false;
    is_wifi_connected = false;
    is_websocket_connected = false;

    /* Initialize callbacks to NULL */
    game_data_callback = NULL;
    chat_message_callback = NULL;
    command_callback = NULL;
    status_callback = NULL;

    /* Initialize UART driver */
    UART_Status status = UART_Init();
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "UART initialization failed: %d\r\n", status);
        return status;
    }

    /* Register callback for interrupt-based reception */
    status = UART_RegisterRxCallback(UART_PORT_2, UART_RxCallback);
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "UART callback registration failed: %d\r\n", status);
        return status;
    }

    /* Enable Rx interrupt */
    status = UART_EnableRxInterrupt(UART_PORT_2);
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "UART interrupt enable failed: %d\r\n", status);
        return status;
    }

    /* Record start time */
    last_activity_time = get_current_ms();
    last_heartbeat_time = get_current_ms();

    DEBUG_PRINTF(false, "STM32 UART Communication Initialized (Clean Architecture)\r\n");

    // Send initial status to ESP32
    serial_comm_send_status(SYSTEM_STATUS_STM32_READY, 0, "STM32_READY");

    return UART_OK;
}

UART_Status serial_comm_deinit(void)
{
    UART_DisableRxInterrupt(UART_PORT_2);

    /* Reset protocol state */
    current_state = PROTO_STATE_INIT;
    is_esp32_ready = false;
    is_wifi_connected = false;
    ui_needs_status_update = false;
    is_websocket_connected = false;

    /* Clear callbacks */
    game_data_callback = NULL;
    chat_message_callback = NULL;
    command_callback = NULL;
    status_callback = NULL;

    /* Clear all buffers */
    circular_buffer_flush(&rx_buffer);
    message_queue_flush(&msg_queue);
    message_parser_reset(&parser);

    DEBUG_PRINTF(false, "UART Communication Deinitialized\r\n");
    return UART_OK;
}

/* Non-blocking message check - parses data and checks queue */
bool serial_comm_is_message_ready(void)
{
//    uint32_t current_time = get_current_ms();

    /* Parse any available data from circular buffer */
    parse_incoming_data();

    /* Check if we have any parsed messages ready */
    return !message_queue_is_empty(&msg_queue);
}

void serial_comm_process_messages(void)
{
    uart_message_t msg;

    /* Process all available messages */
    while (message_queue_get(&msg_queue, &msg)) {
        process_received_message(&msg);
        last_activity_time = get_current_ms();
    }
}

// Getter Functions
bool serial_comm_is_esp32_ready(void)
{
    return is_esp32_ready;
}

bool serial_comm_is_wifi_connected(void)
{
    return is_wifi_connected;
}

bool serial_comm_is_websocket_connected(void)
{
    return is_websocket_connected;
}

bool serial_comm_needs_ui_update(void) {
    return ui_needs_status_update;
}

void serial_comm_clear_ui_update_flag(void) {
    ui_needs_status_update = false;
}

ProtocolState serial_comm_get_state(void)
{
    return current_state;
}

// Message Sending Functions
UART_Status serial_comm_send_message(MessageType type, const uint8_t* data, uint8_t length)
{
    if (length > MAX_PAYLOAD_SIZE) {
        DEBUG_PRINTF(false, "Message data too long: %d bytes (max %d)\r\n",
                     length, MAX_PAYLOAD_SIZE);
        return UART_ERROR;
    }

    uart_message_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.start_byte = MSG_START_BYTE;
    msg.msg_type = (uint8_t)type;
    msg.length = length;

    if (data && length > 0) {
        memcpy(msg.data, data, length);
    }

    // Calculate checksum AFTER setting all other fields
    msg.checksum = calculate_checksum(&msg);
    msg.end_byte = MSG_END_BYTE;

    DEBUG_PRINTF(false, "Sending message: type=0x%02X, length=%d, checksum=0x%02X\r\n",
                 type, length, msg.checksum);

    return send_raw_message(&msg);
}

UART_Status serial_comm_send_game_data(const char* data_type, const char* game_data, const char* metadata)
{
    uart_game_data_t payload;
    memset(&payload, 0, sizeof(payload));

    strncpy(payload.data_type, data_type, sizeof(payload.data_type) - 1);
    strncpy(payload.game_data, game_data, sizeof(payload.game_data) - 1);
    if (metadata) {
        strncpy(payload.metadata, metadata, sizeof(payload.metadata) - 1);
    }
    payload.sequence_num = stats.bytes_sent; // Use bytes sent as sequence

    DEBUG_PRINTF(false, "Sending game data: type='%s', data='%s', meta='%s'\r\n",
                 data_type, game_data, metadata ? metadata : "");

    return serial_comm_send_message(MSG_TYPE_DATA, (const uint8_t*)&payload, sizeof(payload));
}

UART_Status serial_comm_send_chat_message(const char* message, const char* sender, const char* chat_type)
{
    uart_chat_message_t payload;
    memset(&payload, 0, sizeof(payload));

    strncpy(payload.message, message, sizeof(payload.message) - 1);
    strncpy(payload.sender, sender ? sender : "STM32", sizeof(payload.sender) - 1);
    strncpy(payload.chat_type, chat_type ? chat_type : "general", sizeof(payload.chat_type) - 1);
    payload.timestamp = get_current_ms();

    DEBUG_PRINTF(false, "Sending chat message: from='%s', type='%s', message='%s'\r\n",
                 payload.sender, payload.chat_type, payload.message);

    return serial_comm_send_message(MSG_TYPE_CHAT, (const uint8_t*)&payload, sizeof(payload));
}

UART_Status serial_comm_send_command(const char* command, const char* parameters)
{
    uart_command_t cmd;
    memset(&cmd, 0, sizeof(cmd));

    strncpy(cmd.command, command, sizeof(cmd.command) - 1);
    if (parameters) {
        strncpy(cmd.parameters, parameters, sizeof(cmd.parameters) - 1);
    }

    return serial_comm_send_message(MSG_TYPE_COMMAND, (const uint8_t*)&cmd, sizeof(cmd));
}

UART_Status serial_comm_send_status(system_status_type_t system_status, uint8_t error_code, const char* message)
{
    uart_status_t status;
    memset(&status, 0, sizeof(status));

    status.system_status = system_status;
    status.error_code = error_code;
    if (message) {
        strncpy(status.status_message, message, sizeof(status.status_message) - 1);
    }

    DEBUG_PRINTF(false, "Sending status: system=%d, error=%d, message='%s'\r\n",
                 system_status, error_code, message ? message : "");

    return serial_comm_send_message(MSG_TYPE_STATUS, (const uint8_t*)&status, sizeof(status));
}

UART_Status serial_comm_send_ack(void)
{
    return serial_comm_send_message(MSG_TYPE_ACK, NULL, 0);
}

UART_Status serial_comm_send_nack(void)
{
    return serial_comm_send_message(MSG_TYPE_NACK, NULL, 0);
}

UART_Status serial_comm_send_heartbeat(void)
{
    DEBUG_PRINTF(false, "Sending heartbeat to ESP32\r\n");
    return serial_comm_send_message(MSG_TYPE_HEARTBEAT, NULL, 0);
}

// Callback Registration Functions
void serial_comm_register_game_data_callback(game_data_received_callback_t callback)
{
    game_data_callback = callback;
    DEBUG_PRINTF(false, "Game data callback registered\r\n");
}

void serial_comm_register_chat_message_callback(chat_message_received_callback_t callback)
{
    chat_message_callback = callback;
    DEBUG_PRINTF(false, "Chat message callback registered\r\n");
}

void serial_comm_register_command_callback(command_received_callback_t callback)
{
    command_callback = callback;
    DEBUG_PRINTF(false, "Command callback registered\r\n");
}

void serial_comm_register_status_callback(status_received_callback_t callback)
{
    status_callback = callback;
    DEBUG_PRINTF(false, "Status callback registered\r\n");
}

// Utility Functions

void serial_comm_send_debug(const char* message, uint32_t timeout)
{
    DEBUG_PRINTF(false, "%s", message);
}

UART_Status serial_comm_reset(void)
{
    /* Reset protocol state */
    current_state = PROTO_STATE_INIT;
    is_esp32_ready = false;
    is_wifi_connected = false;
    ui_needs_status_update = false;
    is_websocket_connected = false;

    /* Clear all communication buffers */
    circular_buffer_flush(&rx_buffer);
    message_queue_flush(&msg_queue);
    message_parser_reset(&parser);

    DEBUG_PRINTF(false, "Communication reset initiated\r\n");
    last_activity_time = get_current_ms();

    return UART_OK;
}

void serial_comm_print_stats(void)
{
    /* Update stats with current buffer/queue info */
    comm_stats_update_buffer(&stats, &rx_buffer);
    comm_stats_update_queue(&stats, &msg_queue);

    /* Print comprehensive stats */
    DEBUG_PRINTF(false, "=== UART Communication Statistics ===\r\n");
    DEBUG_PRINTF(false, "Messages parsed: %lu\r\n", stats.messages_parsed);
    DEBUG_PRINTF(false, "Parse errors: %lu\r\n", stats.parse_errors);
    DEBUG_PRINTF(false, "Bytes received: %lu\r\n", stats.bytes_received);
    DEBUG_PRINTF(false, "Bytes sent: %lu\r\n", stats.bytes_sent);
    DEBUG_PRINTF(false, "Buffer overflows: %lu\r\n", stats.buffer_overflows);
    DEBUG_PRINTF(false, "Queue overflows: %lu\r\n", stats.queue_overflows);
    DEBUG_PRINTF(false, "Current RX buffer: %d/%d bytes\r\n",
                 circular_buffer_available(&rx_buffer), CIRCULAR_BUFFER_SIZE);
    DEBUG_PRINTF(false, "Current message queue: %d/%d messages\r\n",
                 message_queue_available(&msg_queue), MESSAGE_QUEUE_SIZE);
    DEBUG_PRINTF(false, "===================================\r\n");
}

// Advanced Diagnostic Functions
void serial_comm_get_buffer_status(uint16_t* rx_used, uint16_t* rx_free, uint8_t* msg_count)
{
    if (rx_used) *rx_used = circular_buffer_available(&rx_buffer);
    if (rx_free) *rx_free = circular_buffer_free_space(&rx_buffer);
    if (msg_count) *msg_count = message_queue_available(&msg_queue);
}

bool serial_comm_is_buffer_healthy(void)
{
    /* Consider buffer healthy if not near overflow and no excessive errors */
    uint16_t buffer_usage = circular_buffer_available(&rx_buffer);
    uint16_t buffer_usage_percent = (buffer_usage * 100) / CIRCULAR_BUFFER_SIZE;

    return (buffer_usage_percent < 80) &&
           (stats.buffer_overflows < 10) &&
           (stats.queue_overflows < 5);
}
