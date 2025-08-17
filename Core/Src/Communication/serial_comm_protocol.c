/*
 * serial_comm_protocol.c
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */

#include "Communication/serial_comm_protocol.h"
#include "Communication/serial_comm_callbacks.h"
#include "Utils/debug_conf.h"

 /* Protocol state */
static ProtocolState current_state = PROTO_STATE_INIT;
static uint32_t last_activity_time = 0;
static uint32_t last_heartbeat_time = 0;

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
} ParsedPlayerData;

static ParsedPlayerData stored_player_assignment = { 0 };
static ParsedPlayerData stored_opponent_data = { 0 };

/* Error handling state */
static bool network_error_displayed = false;
static uint32_t last_error_display_time = 0;
static const uint32_t ERROR_DISPLAY_COOLDOWN_MS = 5000;

/* Private function prototypes */
static void protocol_message_handler(const uart_message_t* msg);
static void handle_esp32_status(const uart_status_t* status);
static void handle_connection_message(uart_connection_message_t* msg);
static void handle_esp32_command(const uart_command_t* command);
static void handle_game_data(const uart_game_data_t* game_data);
static void handle_chat_message(const uart_chat_message_t* chat_message);
static bool is_tile_size_valid(const char* message, uint8_t array_length);
static bool parse_and_store_player_data(const char* data_string, bool is_local_player);
static void handle_connection_error(const char* debug_message, const char* user_message);
static void clear_connection_error(void);
static void show_user_error_message(const char* message);

/* Error handling implementation */
static void handle_connection_error(const char* debug_message, const char* user_message) {
    DEBUG_PRINTF(false, "PROTO: Connection Error: %s\r\n", debug_message);

    uint32_t current_time = get_current_ms();
    if (!network_error_displayed ||
        (current_time - last_error_display_time) > ERROR_DISPLAY_COOLDOWN_MS) {

        show_user_error_message(user_message);
        network_error_displayed = true;
        last_error_display_time = current_time;
        DEBUG_PRINTF(false, "PROTO: Connection lost - will exit game\r\n");
    }

    is_wifi_connected = false;
    is_websocket_connected = false;
    current_state = PROTO_STATE_ERROR;
    ui_needs_status_update = true;
}

static void clear_connection_error(void) {
    if (network_error_displayed) {
        display_manager_clear_screen();
        display_manager_update();
        network_error_displayed = false;
        DEBUG_PRINTF(false, "PROTO: Connection error cleared\r\n");
    }
}

static void show_user_error_message(const char* message) {
    display_manager_show_centered_message((char*)message, DISPLAY_HEIGHT / 2);
    DEBUG_PRINTF(false, "PROTO: User Error Message: %s\r\n", message);
}

/* Multiplayer game communication related functions */
static bool is_tile_size_valid(const char* message, uint8_t array_length) {
    if (!message) {
        return false;
    }
    return strncmp(message, TILE_SIZE_ACCEPTED, array_length) == 0;
}

static bool parse_and_store_player_data(const char* data_string, bool is_local_player) {
    if (!data_string || strlen(data_string) == 0) {
        DEBUG_PRINTF(false, "PROTO: Invalid player data string\r\n");
        return false;
    }

    DEBUG_PRINTF(false, "PROTO: Parsing player data: %s (local=%d)\r\n", data_string, is_local_player);

    ParsedPlayerData temp_data = { 0 };
    temp_data.player_id = 1;
    temp_data.player_count = 2;
    strcpy(temp_data.color, "blue");

    int parsed_items = sscanf(data_string, "%d:%31[^:]:%d:%15s",
        &temp_data.player_id, temp_data.session_id,
        &temp_data.player_count, temp_data.color);

    if (parsed_items >= 3) {
        if (temp_data.player_id != 1 && temp_data.player_id != 2) {
            DEBUG_PRINTF(false, "PROTO: Invalid player ID: %d\r\n", temp_data.player_id);
            return false;
        }

        temp_data.valid = true;

        if (is_local_player) {
            stored_player_assignment = temp_data;
        }
        else {
            stored_opponent_data = temp_data;
        }

        DEBUG_PRINTF(false, "PROTO: Stored %s player data successfully\r\n",
            is_local_player ? "LOCAL" : "OPPONENT");
        return true;
    }
    else {
        DEBUG_PRINTF(false, "PROTO: Failed to parse player data. Parsed %d items\r\n", parsed_items);
        return false;
    }
}

/* Handle game data messages */
static void handle_game_data(const uart_game_data_t* game_data) {
    DEBUG_PRINTF(false, "PROTO: GAME DATA: Type=%.16s, Data=%.64s, Meta=%.32s\r\n",
        game_data->data_type, game_data->game_data, game_data->metadata);

    callbacks_handle_game_data(game_data);
    protocol_send_ack();
}

/* Handle connection messages */
static void handle_connection_message(uart_connection_message_t* msg) {
    DEBUG_PRINTF(false, "PROTO: Connection Message: ID=%.6s, Message=%.63s\r\n",
        msg->client_id, msg->message);

    strncpy(stored_client_id, msg->client_id, sizeof(stored_client_id) - 1);
    stored_client_id[sizeof(stored_client_id) - 1] = '\0';

    protocol_send_connection_message(MSG_ACKNOWLEDGE_WEBSOCKET_SERVER, msg->client_id);
    protocol_send_tile_size_validation(MP_DEVICE_TILE_SIZE);

    callbacks_handle_connection_message(msg);
    protocol_send_ack();
}

/* Handle chat messages */
static void handle_chat_message(const uart_chat_message_t* chat_message) {
    DEBUG_PRINTF(false, "PROTO: CHAT MESSAGE: From=%.32s, Type=%.16s, Message=%.96s\r\n",
        chat_message->sender, chat_message->chat_type, chat_message->message);

    callbacks_handle_chat_message(chat_message);
    protocol_send_ack();
}

/* Handle ESP32 status messages */
static void handle_esp32_status(const uart_status_t* status) {
    DEBUG_PRINTF(false, "PROTO: ESP32 Status: %d, Error: %d, Msg: %.32s\r\n",
        status->system_status, status->error_code, status->status_message);

    switch (status->system_status) {
    case SYSTEM_STATUS_ESP32_STARTED:
        if (current_state == PROTO_STATE_INIT) {
            current_state = PROTO_STATE_ESP32_READY;
            is_esp32_ready = true;
            clear_connection_error();
            DEBUG_PRINTF(false, "PROTO: ESP32 started and ready\r\n");
            ui_needs_status_update = true;
        }
        break;

    case SYSTEM_STATUS_ESP32_READY:
        current_state = PROTO_STATE_ESP32_READY;
        is_esp32_ready = true;
        clear_connection_error();
        DEBUG_PRINTF(false, "PROTO: ESP32 initialization complete - sending ack\r\n");
        protocol_send_ack();
        ui_needs_status_update = true;
        break;

    case SYSTEM_STATUS_WIFI_CONNECTING:
        if (current_state == PROTO_STATE_ESP32_READY) {
            current_state = PROTO_STATE_WIFI_CONNECTING;
            clear_connection_error();
            DEBUG_PRINTF(false, "PROTO: ESP32 WiFi connecting\r\n");
            ui_needs_status_update = true;
        }
        break;

    case SYSTEM_STATUS_WIFI_CONNECTED:
        current_state = PROTO_STATE_WIFI_CONNECTED;
        is_wifi_connected = true;
        clear_connection_error();
        DEBUG_PRINTF(false, "PROTO: ESP32 WiFi connected successfully\r\n");
        ui_needs_status_update = true;
        break;

    case SYSTEM_STATUS_WIFI_DISCONNECTED:
        DEBUG_PRINTF(false, "PROTO: WiFi disconnected: reason=%d, message=%s\r\n",
            status->error_code, status->status_message);
        handle_connection_error("WiFi disconnected", "Lost WiFi Connection");
        // ui_needs_status_update = true; is set inside handle_connection_error
        break;

    case SYSTEM_STATUS_WEBSOCKET_CONNECTING:
        if (current_state == PROTO_STATE_WIFI_CONNECTED) {
            current_state = PROTO_STATE_WEBSOCKET_CONNECTING;
            DEBUG_PRINTF(false, "PROTO: ESP32 WebSocket connecting\r\n");
            ui_needs_status_update = true;
        }
        break;

    case SYSTEM_STATUS_WEBSOCKET_CONNECTED:
        current_state = PROTO_STATE_WEBSOCKET_CONNECTED;
        is_websocket_connected = true;
        clear_connection_error();
        ui_needs_status_update = true;
        DEBUG_PRINTF(false, "PROTO: ESP32 WebSocket connected successfully\r\n");

        break;

    case SYSTEM_STATUS_WEBSOCKET_DISCONNECTED:
        DEBUG_PRINTF(false, "PROTO: WebSocket disconnected: error=%d, message=%s\r\n",
            status->error_code, status->status_message);
        handle_connection_error("WebSocket disconnected", "Lost WiFi Connection");
        // ui_needs_status_update = true; is set inside handle_connection_error
        break;

    case SYSTEM_STATUS_GAME_READY:
        current_state = PROTO_STATE_GAME_READY;
        clear_connection_error();
        DEBUG_PRINTF(false, "PROTO: ESP32 game session ready\r\n");
        protocol_send_status(SYSTEM_STATUS_STM32_GAME_READY, 0, "STM32_GAME_READY");
        break;

    case SYSTEM_STATUS_GAME_ACTIVE:
        if (current_state == PROTO_STATE_GAME_READY) {
            current_state = PROTO_STATE_GAME_ACTIVE;
            DEBUG_PRINTF(false, "PROTO: Game session active\r\n");
        }
        break;

    case SYSTEM_STATUS_GAME_ENDED:
        if (current_state == PROTO_STATE_GAME_ACTIVE) {
            current_state = PROTO_STATE_WEBSOCKET_CONNECTED;
            DEBUG_PRINTF(false, "PROTO: Game session ended - back to ready state\r\n");
        }
        break;

    case SYSTEM_STATUS_PLAYER_ASSIGNMENT:
        DEBUG_PRINTF(false, "PROTO: Player assignment received: %s\r\n", status->status_message);
        if (parse_and_store_player_data(status->status_message, true)) {
            DEBUG_PRINTF(false, "PROTO: Local player assignment stored successfully\r\n");
        }
        else {
            DEBUG_PRINTF(false, "PROTO: Failed to parse local player assignment\r\n");
        }
        break;

    case SYSTEM_STATUS_OPPONENT_CONNECTED:
        DEBUG_PRINTF(false, "PROTO: Opponent connected: %s\r\n", status->status_message);
        if (parse_and_store_player_data(status->status_message, false)) {
            DEBUG_PRINTF(false, "PROTO: Opponent data stored successfully\r\n");
        }
        else {
            DEBUG_PRINTF(false, "PROTO: Failed to parse opponent data\r\n");
        }
        break;

    case SYSTEM_STATUS_OPPONENT_DISCONNECTED:
        DEBUG_PRINTF(false, "PROTO: Opponent disconnected\r\n");
        memset(&stored_opponent_data, 0, sizeof(stored_opponent_data));
        break;

    case SYSTEM_STATUS_TILE_SIZE_RESPONSE:
        is_game_over = !is_tile_size_valid(status->status_message,
            sizeof(status->status_message) / sizeof(char));
        if (is_game_over) {
            DEBUG_PRINTF(false, "PROTO: Tile size validation failed: %s\r\n", status->status_message);
        }
        else {
            DEBUG_PRINTF(false, "PROTO: Tile size validation passed\r\n");
        }
        break;

    case SYSTEM_STATUS_ERROR:
        DEBUG_PRINTF(false, "PROTO: ESP32 general error: code=%d, message=%s\r\n",
            status->error_code, status->status_message);
        handle_connection_error("ESP32 system error", "Connection Error");
        break;

    default:
        DEBUG_PRINTF(false, "PROTO: Unknown ESP32 status: %d\r\n", status->system_status);
        break;
    }

    callbacks_handle_status(status);
}

/* Handle ESP32 commands */
static void handle_esp32_command(const uart_command_t* command) {
    DEBUG_PRINTF(false, "PROTO: ESP32 Command: %.32s %.64s\r\n",
        command->command, command->parameters);

    if (strncmp(command->command, "game_ready", 10) == 0) {
        current_state = PROTO_STATE_GAME_READY;
        DEBUG_PRINTF(false, "PROTO: Game session ready\r\n");
        protocol_send_status(SYSTEM_STATUS_STM32_GAME_READY, 0, "STM32_GAME_READY");
        protocol_send_ack();
    }
    else if (strncmp(command->command, "game_start", 10) == 0) {
        current_state = PROTO_STATE_GAME_ACTIVE;
        DEBUG_PRINTF(false, "PROTO: Game session started\r\n");
        protocol_send_ack();
    }
    else if (strncmp(command->command, "game_end", 8) == 0) {
        current_state = PROTO_STATE_WEBSOCKET_CONNECTED;
        DEBUG_PRINTF(false, "PROTO: Game session ended\r\n");
        protocol_send_ack();
    }
    else if (strncmp(command->command, "ping", 4) == 0) {
        DEBUG_PRINTF(false, "PROTO: Ping received, sending ACK\r\n");
        protocol_send_ack();
    }
    else if (strncmp(command->command, "pong", 4) == 0) {
        DEBUG_PRINTF(false, "PROTO: Pong received from ESP32\r\n");
    }
    else if (strncmp(command->command, "reset", 5) == 0) {
        DEBUG_PRINTF(false, "PROTO: Reset command received\r\n");
        protocol_reset();
        protocol_send_ack();
    }

    callbacks_handle_command(command);
}

/* Protocol message handler - processes decoded messages */
static void protocol_message_handler(const uart_message_t* msg) {
    DEBUG_PRINTF(false, "PROTO: Processing message type: 0x%02X, length: %d\r\n",
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
        DEBUG_PRINTF(false, "PROTO: Received ACK from ESP32\r\n");
        break;

    case MSG_TYPE_NACK:
        DEBUG_PRINTF(false, "PROTO: Received NACK from ESP32\r\n");
        break;

    case MSG_TYPE_HEARTBEAT:
        DEBUG_PRINTF(false, "PROTO: Received heartbeat from ESP32\r\n");
        protocol_send_ack();
        break;

    default:
        DEBUG_PRINTF(false, "PROTO: Unknown message type: 0x%02X\r\n", msg->msg_type);
        break;
    }
}

/* Public Function Implementations */
void protocol_init(void) {
    /* Initialize protocol state */
    current_state = PROTO_STATE_INIT;
    is_esp32_ready = false;
    is_wifi_connected = false;
    is_websocket_connected = false;
    ui_needs_status_update = false;

    /* Reset error state */
    network_error_displayed = false;
    last_error_display_time = 0;

    /* Clear stored data */
    memset(stored_client_id, 0, sizeof(stored_client_id));
    memset(&stored_player_assignment, 0, sizeof(stored_player_assignment));
    memset(&stored_opponent_data, 0, sizeof(stored_opponent_data));
    is_game_over = false;

    /* Initialize hardware layer */
    hardware_serial_init();

    /* Register our message handler with hardware layer */
    hardware_serial_register_callback(protocol_message_handler);

    /* Initialize callbacks layer */
    callbacks_init();

    /* Record start time */
    last_activity_time = get_current_ms();
    last_heartbeat_time = get_current_ms();

    DEBUG_PRINTF(false, "PROTO: Protocol Layer Initialized\r\n");

    /* Send initial status to ESP32 */
    protocol_send_status(SYSTEM_STATUS_STM32_READY, 0, "STM32_READY");
}

void protocol_deinit(void) {
    /* Reset protocol state */
    current_state = PROTO_STATE_INIT;
    is_esp32_ready = false;
    is_wifi_connected = false;
    is_websocket_connected = false;
    ui_needs_status_update = false;

    /* Deinitialize layers */
    callbacks_deinit();
    // To Do: Send a message to ESP32 that communication has been de-initialized to put it to sleep mode
    hardware_serial_deinit();

    DEBUG_PRINTF(false, "PROTO: Protocol Layer Deinitialized\r\n");
}

void protocol_process(void) {
    /* Check for incoming messages and process them */
    if (hardware_serial_is_message_ready()) {
        hardware_serial_process_incoming();
        last_activity_time = get_current_ms();
    }
}

void protocol_reset(void) {
    /* Reset protocol state */
    current_state = PROTO_STATE_INIT;
    is_esp32_ready = false;
    is_wifi_connected = false;
    is_websocket_connected = false;
    ui_needs_status_update = false;

    /* Reset hardware buffers */
    hardware_serial_reset_buffers();

    DEBUG_PRINTF(false, "PROTO: Protocol reset initiated\r\n");
    last_activity_time = get_current_ms();
}

/* Connection status functions */
bool protocol_is_esp32_ready(void) {
    return is_esp32_ready;
}

bool protocol_is_wifi_connected(void) {
    return is_wifi_connected;
}

bool protocol_is_websocket_connected(void) {
    return is_websocket_connected;
}

bool protocol_needs_ui_update(void) {
    return ui_needs_status_update;
}

void protocol_clear_ui_update_flag(void) {
    ui_needs_status_update = false;
}

ProtocolState protocol_get_state(void) {
    return current_state;
}

/* Enhanced error handling functions */
bool protocol_has_network_error(void) {
    return network_error_displayed || (current_state == PROTO_STATE_ERROR);
}

void protocol_clear_network_error(void) {
    clear_connection_error();
}

const char* protocol_get_error_message(void) {
    if (network_error_displayed) {
        return "Lost WiFi Connection";
    }
    return NULL;
}

/* Multiplayer communication getter functions */
const char* protocol_get_client_id(void) {
    return stored_client_id;
}

bool protocol_get_mp_game_over(void) {
    return is_game_over;
}

bool protocol_get_player_assignment(int* player_id, char* session_id, size_t session_id_size,
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

bool protocol_get_opponent_data(int* player_id, char* session_id, size_t session_id_size,
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

bool protocol_has_player_assignment(void) {
    return stored_player_assignment.valid;
}

bool protocol_has_opponent_connected(void) {
    return stored_opponent_data.valid;
}

int protocol_get_local_player_id(void) {
    return stored_player_assignment.valid ? stored_player_assignment.player_id : 0;
}

int protocol_get_opponent_player_id(void) {
    return stored_opponent_data.valid ? stored_opponent_data.player_id : 0;
}

/* Message sending functions */
UART_Status protocol_send_message(MessageType type, const uint8_t* data, uint8_t length) {
    if (length > MAX_PAYLOAD_SIZE) {
        DEBUG_PRINTF(false, "PROTO: Message data too long: %d bytes (max %d)\r\n",
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

    msg.checksum = hardware_serial_calculate_checksum(&msg);
    msg.end_byte = MSG_END_BYTE;

    DEBUG_PRINTF(false, "PROTO: Sending message: type=0x%02X, length=%d, checksum=0x%02X\r\n",
        type, length, msg.checksum);

    return hardware_serial_send_message(&msg);
}

UART_Status protocol_send_game_data(const char* data_type, const char* game_data, const char* metadata) {
    uart_game_data_t payload;
    memset(&payload, 0, sizeof(payload));

    strncpy(payload.data_type, data_type, sizeof(payload.data_type) - 1);
    strncpy(payload.game_data, game_data, sizeof(payload.game_data) - 1);
    if (metadata) {
        strncpy(payload.metadata, metadata, sizeof(payload.metadata) - 1);
    }

    uint32_t bytes_sent = 0;
    hardware_serial_get_stats(&bytes_sent, NULL, NULL, NULL);
    payload.sequence_num = bytes_sent;

    DEBUG_PRINTF(false, "PROTO: Sending game data: type='%s', data='%s', meta='%s'\r\n",
        data_type, game_data, metadata ? metadata : "");

    return protocol_send_message(MSG_TYPE_DATA, (const uint8_t*)&payload, sizeof(payload));
}

UART_Status protocol_send_connection_message(const char* message, const char* client_id) {
    uart_connection_message_t payload;
    memset(&payload, 0, sizeof(payload));

    strncpy(payload.client_id, client_id, sizeof(payload.client_id) - 1);
    strncpy(payload.message, message, sizeof(payload.message) - 1);
    payload.timestamp = get_current_ms();

    DEBUG_PRINTF(false, "PROTO: Sending connection data: message='%s', client_id='%s'\r\n",
        message, client_id);

    return protocol_send_message(MSG_TYPE_CONNECTION, (const uint8_t*)&payload, sizeof(payload));
}

UART_Status protocol_send_tile_size_validation(uint8_t tile_size) {
    uart_tile_size_validation_t payload;
    memset(&payload, 0, sizeof(payload));

    payload.tile_size = tile_size;
    payload.timestamp = get_current_ms();

    DEBUG_PRINTF(false, "PROTO: Sending tile size validation: size=%d\r\n", tile_size);

    return protocol_send_message(MSG_TILE_SIZE_VALIDATION, (const uint8_t*)&payload, sizeof(payload));
}

UART_Status protocol_send_chat_message(const char* message, const char* sender, const char* chat_type) {
    uart_chat_message_t payload;
    memset(&payload, 0, sizeof(payload));

    strncpy(payload.message, message, sizeof(payload.message) - 1);
    strncpy(payload.sender, sender ? sender : "STM32", sizeof(payload.sender) - 1);
    strncpy(payload.chat_type, chat_type ? chat_type : "general", sizeof(payload.chat_type) - 1);
    payload.timestamp = get_current_ms();

    DEBUG_PRINTF(false, "PROTO: Sending chat message: from='%s', type='%s', message='%s'\r\n",
        payload.sender, payload.chat_type, payload.message);

    return protocol_send_message(MSG_TYPE_CHAT, (const uint8_t*)&payload, sizeof(payload));
}

UART_Status protocol_send_command(const char* command, const char* parameters) {
    uart_command_t cmd;
    memset(&cmd, 0, sizeof(cmd));

    strncpy(cmd.command, command, sizeof(cmd.command) - 1);
    if (parameters) {
        strncpy(cmd.parameters, parameters, sizeof(cmd.parameters) - 1);
    }

    return protocol_send_message(MSG_TYPE_COMMAND, (const uint8_t*)&cmd, sizeof(cmd));
}

UART_Status protocol_send_status(uint8_t system_status, uint8_t error_code, const char* message) {
    uart_status_t status;
    memset(&status, 0, sizeof(status));

    status.system_status = system_status;
    status.error_code = error_code;
    if (message) {
        strncpy(status.status_message, message, sizeof(status.status_message) - 1);
    }

    DEBUG_PRINTF(false, "PROTO: Sending status: system=%d, error=%d, message='%s'\r\n",
        system_status, error_code, message ? message : "");

    return protocol_send_message(MSG_TYPE_STATUS, (const uint8_t*)&status, sizeof(status));
}

UART_Status protocol_send_ack(void) {
    return protocol_send_message(MSG_TYPE_ACK, NULL, 0);
}

UART_Status protocol_send_nack(void) {
    return protocol_send_message(MSG_TYPE_NACK, NULL, 0);
}

UART_Status protocol_send_heartbeat(void) {
    DEBUG_PRINTF(false, "PROTO: Sending heartbeat to ESP32\r\n");
    return protocol_send_message(MSG_TYPE_HEARTBEAT, NULL, 0);
}

/* Utility functions */
void protocol_send_debug(const char* message, uint32_t timeout) {
    DEBUG_PRINTF(false, "PROTO: %s", message);
}

void protocol_print_stats(void) {
    uint32_t bytes_sent = 0, bytes_received = 0, messages_parsed = 0, parse_errors = 0;
    hardware_serial_get_stats(&bytes_sent, &bytes_received, &messages_parsed, &parse_errors);

    DEBUG_PRINTF(false, "=== Protocol Communication Statistics ===\r\n");
    DEBUG_PRINTF(false, "Messages parsed: %lu\r\n", messages_parsed);
    DEBUG_PRINTF(false, "Parse errors: %lu\r\n", parse_errors);
    DEBUG_PRINTF(false, "Bytes received: %lu\r\n", bytes_received);
    DEBUG_PRINTF(false, "Bytes sent: %lu\r\n", bytes_sent);
    DEBUG_PRINTF(false, "Current state: %d\r\n", current_state);
    DEBUG_PRINTF(false, "ESP32 ready: %s\r\n", is_esp32_ready ? "Yes" : "No");
    DEBUG_PRINTF(false, "WiFi connected: %s\r\n", is_wifi_connected ? "Yes" : "No");
    DEBUG_PRINTF(false, "WebSocket connected: %s\r\n", is_websocket_connected ? "Yes" : "No");
    DEBUG_PRINTF(false, "========================================\r\n");
}
