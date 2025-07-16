/*
 * serial_comm.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 *
 */

#include "Communication/serial_comm.h"
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

/* Game related data */
static char stored_client_id[7] = { 0 };
static bool is_game_over = false;
typedef struct {
    int player_id;
    char session_id[32];
    int player_count;
    char color[16];
    bool valid;
} ParsedPlayerData; // Generic structure to hold parsed player data (local and opponent)

static ParsedPlayerData stored_player_assignment = { 0 };
static ParsedPlayerData stored_opponent_data = { 0 };


/* Error handling state */
static bool network_error_displayed = false;
static uint32_t last_error_display_time = 0;
static const uint32_t ERROR_DISPLAY_COOLDOWN_MS = 5000; // Don't spam error messages

/* Callback functions */
static game_data_received_callback_t game_data_callback = NULL;
static chat_message_received_callback_t chat_message_callback = NULL;
static command_received_callback_t command_callback = NULL;
static status_received_callback_t status_callback = NULL;
static connection_message_callback_t connection_message_callback = NULL;

/* Private function prototypes */
static void UART_RxCallback(uint8_t data);
static uint8_t calculate_checksum(const uart_message_t* msg);
static bool validate_message(const uart_message_t* msg);
static void process_received_message(const uart_message_t* msg);
static UART_Status send_raw_message(const uart_message_t* msg);
static void handle_esp32_status(const uart_status_t* status);
static void handle_connection_message(uart_connection_message_t* msg);
static void handle_esp32_command(const uart_command_t* command);
static void handle_game_data(const uart_game_data_t* game_data);
static void handle_chat_message(const uart_chat_message_t* chat_message);
static void parse_incoming_data(void);
static bool is_tile_size_valid(const char* message, uint8_t array_length);
static bool parse_and_store_player_data(const char* data_string, bool is_local_player);

/* Error handling functions */
static void handle_connection_error(const char* debug_message, const char* user_message);
static void clear_connection_error(void);
static void show_user_error_message(const char* message);

/* Error handling implementation */
static void handle_connection_error(const char* debug_message, const char* user_message) {
    // Always log detailed error for debugging
    DEBUG_PRINTF(false, "Connection Error: %s\r\n", debug_message);

    // Show user-friendly message (with cooldown to prevent spam)
    uint32_t current_time = get_current_ms();
    if (!network_error_displayed ||
        (current_time - last_error_display_time) > ERROR_DISPLAY_COOLDOWN_MS) {

        show_user_error_message(user_message);
        network_error_displayed = true;
        last_error_display_time = current_time;

        DEBUG_PRINTF(false, "Connection lost - will exit game\r\n");
    }

    // Update connection state
    is_wifi_connected = false;
    is_websocket_connected = false;
    current_state = PROTO_STATE_ERROR;
    ui_needs_status_update = true;
}

static void clear_connection_error(void) {
    if (network_error_displayed) {
        // Clear error message from display
        display_manager_clear_screen();
        display_manager_update();
        network_error_displayed = false;
        DEBUG_PRINTF(false, "Connection error cleared\r\n");
    }
}

static void show_user_error_message(const char* message) {
    // Show error message to user through display manager
    display_manager_show_centered_message((char*)message, DISPLAY_HEIGHT / 2);
    DEBUG_PRINTF(false, "User Error Message: %s\r\n", message);
}

/* Multiplayer game communication related functions */
// Check whether tile size is valid
static bool is_tile_size_valid(const char* message, uint8_t array_length) {
    if (!message) {
        return false;
    }

    // Use strncmp to safely compare within the array bounds
    return strncmp(message, TILE_SIZE_ACCEPTED, array_length) == 0;
}

// Helper function to parse player data
static bool parse_and_store_player_data(const char* data_string, bool is_local_player) {
    if (!data_string || strlen(data_string) == 0) {
        DEBUG_PRINTF(false, "Invalid player data string\r\n");
        return false;
    }

    DEBUG_PRINTF(false, "Parsing player data: %s (local=%d)\r\n", data_string, is_local_player);

    ParsedPlayerData temp_data = { 0 };
    temp_data.player_id = 1;  // Default values
    temp_data.player_count = 2;
    strcpy(temp_data.color, "blue");

    // Parse format: "player_id:session_id:player_count:color"
    int parsed_items = sscanf(data_string, "%d:%31[^:]:%d:%15s",
        &temp_data.player_id, temp_data.session_id,
        &temp_data.player_count, temp_data.color);

    if (parsed_items >= 3) { // At minimum we need player_id, session_id, and player_count
        // Validate player_id
        if (temp_data.player_id != 1 && temp_data.player_id != 2) {
            DEBUG_PRINTF(false, "Invalid player ID: %d\r\n", temp_data.player_id);
            return false;
        }

        temp_data.valid = true;

        // Store in appropriate structure
        if (is_local_player) {
            stored_player_assignment = temp_data;
        }
        else {
            stored_opponent_data = temp_data;
        }

        DEBUG_PRINTF(false, "Stored %s player data successfully:\r\n", is_local_player ? "LOCAL" : "OPPONENT");
        DEBUG_PRINTF(false, "  Player ID: %d\r\n", temp_data.player_id);
        DEBUG_PRINTF(false, "  Session ID: %s\r\n", temp_data.session_id);
        DEBUG_PRINTF(false, "  Player Count: %d\r\n", temp_data.player_count);
        if (parsed_items >= 4) {
            DEBUG_PRINTF(false, "  Color: %s\r\n", temp_data.color);
        }

        return true;
    }
    else {
        DEBUG_PRINTF(false, "Failed to parse player data. Parsed %d items\r\n", parsed_items);
        return false;
    }
}

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
                    }
                    else {
                        DEBUG_PRINTF(false, "Message queue full, dropping message\r\n");
                    }
                }
                else {
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

/* Handle connection messages */
static void handle_connection_message(uart_connection_message_t* msg) {
    DEBUG_PRINTF(false, "Connection Message: ID=%.6s, Message=%.63s, Timestamp=%lu\r\n",
        msg->client_id, msg->message, msg->timestamp);

    // Store client ID for later use
    strncpy(stored_client_id, msg->client_id, sizeof(stored_client_id) - 1);
    stored_client_id[sizeof(stored_client_id) - 1] = '\0';

    // Send acknowledgement immediately
    serial_comm_send_connection_message(MSG_ACKNOWLEDGE_WEBSOCKET_SERVER, msg->client_id);

    // Send device tile size for validation
    serial_comm_send_tile_size_validation(MP_DEVICE_TILE_SIZE);

    // Call registered callback if available
    if (connection_message_callback) {
        connection_message_callback(msg);
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
static void handle_esp32_status(const uart_status_t* status) {
    DEBUG_PRINTF(false, "ESP32 Status: %d, Error: %d, Msg: %.32s\r\n",
        status->system_status, status->error_code, status->status_message);

    switch (status->system_status) {
    case SYSTEM_STATUS_ESP32_STARTED:
        if (current_state == PROTO_STATE_INIT) {
            current_state = PROTO_STATE_ESP32_READY;
            is_esp32_ready = true;
            clear_connection_error(); // Clear any previous errors
            DEBUG_PRINTF(false, "ESP32 started and ready\r\n");
        }
        break;

    case SYSTEM_STATUS_ESP32_READY:
        current_state = PROTO_STATE_ESP32_READY;
        is_esp32_ready = true;
        clear_connection_error();
        DEBUG_PRINTF(false, "ESP32 initialization complete - sending ack\r\n");
        serial_comm_send_ack();
        break;

    case SYSTEM_STATUS_WIFI_CONNECTING:
        if (current_state == PROTO_STATE_ESP32_READY) {
            current_state = PROTO_STATE_WIFI_CONNECTING;
            clear_connection_error(); // Clear error while attempting connection
            DEBUG_PRINTF(false, "ESP32 WiFi connecting\r\n");
        }
        break;

    case SYSTEM_STATUS_WIFI_CONNECTED:
        current_state = PROTO_STATE_WIFI_CONNECTED;
        is_wifi_connected = true;
        clear_connection_error();
        DEBUG_PRINTF(false, "ESP32 WiFi connected successfully\r\n");
        ui_needs_status_update = true;
        break;

    case SYSTEM_STATUS_WIFI_DISCONNECTED:
        // Detailed debug info
        DEBUG_PRINTF(false, "WiFi disconnected: reason=%d, message=%s\r\n",
            status->error_code, status->status_message);

        // Unified user error handling
        handle_connection_error("WiFi disconnected", "Lost WiFi Connection");
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
        clear_connection_error();
        DEBUG_PRINTF(false, "ESP32 WebSocket connected successfully\r\n");
        break;

    case SYSTEM_STATUS_WEBSOCKET_DISCONNECTED:
        // Detailed debug info
        DEBUG_PRINTF(false, "WebSocket disconnected: error=%d, message=%s\r\n",
            status->error_code, status->status_message);

        // Unified user error handling - same message as WiFi for simplicity
        handle_connection_error("WebSocket disconnected", "Lost WiFi Connection");
        break;

    case SYSTEM_STATUS_GAME_READY:
        current_state = PROTO_STATE_GAME_READY;
        clear_connection_error();
        DEBUG_PRINTF(false, "ESP32 game session ready\r\n");
        serial_comm_send_status(SYSTEM_STATUS_STM32_GAME_READY, 0, "STM32_GAME_READY");
        break;

    case SYSTEM_STATUS_GAME_ACTIVE:
        if (current_state == PROTO_STATE_GAME_READY) {
            current_state = PROTO_STATE_GAME_ACTIVE;
            DEBUG_PRINTF(false, "Game session active\r\n");
        }
        break;

    case SYSTEM_STATUS_GAME_ENDED:
        if (current_state == PROTO_STATE_GAME_ACTIVE) {
            current_state = PROTO_STATE_WEBSOCKET_CONNECTED;
            DEBUG_PRINTF(false, "Game session ended - back to ready state\r\n");
        }
        break;

    case SYSTEM_STATUS_PLAYER_ASSIGNMENT:
        DEBUG_PRINTF(false, "Player assignment received: %s\r\n", status->status_message);
        if (parse_and_store_player_data(status->status_message, true)) {
            DEBUG_PRINTF(false, "Local player assignment stored successfully\r\n");
        }
        else {
            DEBUG_PRINTF(false, "Failed to parse local player assignment\r\n");
        }
        break;

    case SYSTEM_STATUS_OPPONENT_CONNECTED:
        DEBUG_PRINTF(false, "Opponent connected: %s\r\n", status->status_message);
        if (parse_and_store_player_data(status->status_message, false)) {
            DEBUG_PRINTF(false, "Opponent data stored successfully\r\n");
        }
        else {
            DEBUG_PRINTF(false, "Failed to parse opponent data\r\n");
        }
        break;

    case SYSTEM_STATUS_OPPONENT_DISCONNECTED:
        DEBUG_PRINTF(false, "Opponent disconnected\r\n");
        // Clear opponent data
        memset(&stored_opponent_data, 0, sizeof(stored_opponent_data));
        break;

    case SYSTEM_STATUS_TILE_SIZE_RESPONSE:
        is_game_over =
            !is_tile_size_valid(status->status_message, sizeof(status->status_message) / sizeof(char));
        if (is_game_over) {
            DEBUG_PRINTF(false, "Tile size validation failed: %s\r\n", status->status_message);
        }
        else {
            DEBUG_PRINTF(false, "Tile size validation passed\r\n");
        }
        break;

    case SYSTEM_STATUS_ERROR:
        // Detailed debug info
        DEBUG_PRINTF(false, "ESP32 general error: code=%d, message=%s\r\n",
            status->error_code, status->status_message);

        // General error handling
        handle_connection_error("ESP32 system error", "Connection Error");
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

    case MSG_TYPE_CONNECTION:
        if (msg->length == sizeof(uart_connection_message_t)) {
            handle_connection_message((uart_connection_message_t*)msg->data);
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

    // Reset error state
    network_error_displayed = false;
    last_error_display_time = 0;

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

// Get local player client id
const char* serial_comm_get_client_id(void) {
    return stored_client_id;
}

// Get multiplayer game over state
bool serial_comm_get_mp_game_over(void) {
    return is_game_over;
}

// Get local player assignment data
bool serial_comm_get_player_assignment(int* player_id, char* session_id, size_t session_id_size,
    int* player_count, char* color, size_t color_size) {
    if (!stored_player_assignment.valid) {
        return false;
    }

    if (player_id) {
        *player_id = stored_player_assignment.player_id;
    }

    if (session_id && session_id_size > 0) {
        strncpy(session_id, stored_player_assignment.session_id, session_id_size - 1);
        session_id[session_id_size - 1] = '\0';
    }

    if (player_count) {
        *player_count = stored_player_assignment.player_count;
    }

    if (color && color_size > 0) {
        strncpy(color, stored_player_assignment.color, color_size - 1);
        color[color_size - 1] = '\0';
    }

    return true;
}

// Get opponent data
bool serial_comm_get_opponent_data(int* player_id, char* session_id, size_t session_id_size,
    int* player_count, char* color, size_t color_size) {
    if (!stored_opponent_data.valid) {
        return false;
    }

    if (player_id) {
        *player_id = stored_opponent_data.player_id;
    }

    if (session_id && session_id_size > 0) {
        strncpy(session_id, stored_opponent_data.session_id, session_id_size - 1);
        session_id[session_id_size - 1] = '\0';
    }

    if (player_count) {
        *player_count = stored_opponent_data.player_count;
    }

    if (color && color_size > 0) {
        strncpy(color, stored_opponent_data.color, color_size - 1);
        color[color_size - 1] = '\0';
    }

    return true;
}

// Check if player assignment is available
bool serial_comm_has_player_assignment(void) {
    return stored_player_assignment.valid;
}

// Check if opponent is connected
bool serial_comm_has_opponent_connected(void) {
    return stored_opponent_data.valid;
}

// Get local player ID (convenience function)
int serial_comm_get_local_player_id(void) {
    return stored_player_assignment.valid ? stored_player_assignment.player_id : 0;
}

// Get opponent player ID (convenience function)
int serial_comm_get_opponent_player_id(void) {
    return stored_opponent_data.valid ? stored_opponent_data.player_id : 0;
}

// Enhanced connection status functions
bool serial_comm_has_network_error(void) {
    return network_error_displayed || (current_state == PROTO_STATE_ERROR);
}

void serial_comm_clear_network_error(void) {
    clear_connection_error();
}

const char* serial_comm_get_error_message(void) {
    if (network_error_displayed) {
        return "Lost WiFi Connection";
    }
    return NULL;
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

UART_Status serial_comm_send_connection_message(const char* message, const char* client_id) {
    uart_connection_message_t payload;
    memset(&payload, 0, sizeof(payload));

    strncpy(payload.client_id, client_id, sizeof(payload.client_id) - 1);
    strncpy(payload.message, message, sizeof(payload.message) - 1);
    payload.timestamp = get_current_ms();

    DEBUG_PRINTF(false, "Sending connection data: message='%s', client_id='%s'\r\n",
        message, client_id);

    return serial_comm_send_message(MSG_TYPE_CONNECTION, (const uint8_t*)&payload, sizeof(payload));
}

UART_Status serial_comm_send_tile_size_validation(uint8_t tile_size) {
    uart_tile_size_validation_t payload;
    memset(&payload, 0, sizeof(payload));

    payload.tile_size = tile_size;
    payload.timestamp = get_current_ms();

    DEBUG_PRINTF(false, "Sending tile size validation: size=%d\r\n", tile_size);

    return serial_comm_send_message(MSG_TILE_SIZE_VALIDATION, (const uint8_t*)&payload, sizeof(payload));
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

void serial_comm_register_connection_message_callback(connection_message_callback_t callback) {
    connection_message_callback = callback;
    DEBUG_PRINTF(false, "Connection message callback registered\r\n");
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
