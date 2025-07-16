/*
 * serial_comm.c
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */


#include "Communication/serial_comm.h"
#include "Utils/debug_conf.h"

/* Core communication system */
UART_Status serial_comm_init(void) {
    DEBUG_PRINTF(false, "MAIN: Initializing serial communication system\r\n");

    /* Initialize protocol layer (which initializes hardware and callbacks) */
    protocol_init();

    /* Initialize snake multiplayer layer */
    snake_multiplayer_init();

    DEBUG_PRINTF(false, "MAIN: Serial communication system initialized\r\n");
    return UART_OK;
}

UART_Status serial_comm_deinit(void) {
    DEBUG_PRINTF(false, "MAIN: Deinitializing serial communication system\r\n");

    /* Deinitialize in reverse order */
    snake_multiplayer_deinit();
    protocol_deinit();

    DEBUG_PRINTF(false, "MAIN: Serial communication system deinitialized\r\n");
    return UART_OK;
}

void serial_comm_process_messages(void) {
    /* Process messages at protocol level */
    protocol_process();
}

UART_Status serial_comm_reset(void) {
    DEBUG_PRINTF(false, "MAIN: Resetting serial communication system\r\n");
    protocol_reset();
    return UART_OK;
}

/* Connection status functions - map directly to protocol layer */
bool serial_comm_is_esp32_ready(void) {
    return protocol_is_esp32_ready();
}

bool serial_comm_is_wifi_connected(void) {
    return protocol_is_wifi_connected();
}

bool serial_comm_is_websocket_connected(void) {
    return protocol_is_websocket_connected();
}

ProtocolState serial_comm_get_state(void) {
    return protocol_get_state();
}

bool serial_comm_needs_ui_update(void) {
    return protocol_needs_ui_update();
}

void serial_comm_clear_ui_update_flag(void) {
    protocol_clear_ui_update_flag();
}

/* Error handling functions - map directly to protocol layer */
bool serial_comm_has_network_error(void) {
    return protocol_has_network_error();
}

void serial_comm_clear_network_error(void) {
    protocol_clear_network_error();
}

const char* serial_comm_get_error_message(void) {
    return protocol_get_error_message();
}

/* Basic message sending functions - map directly to protocol layer */
UART_Status serial_comm_send_game_data(const char* data_type, const char* game_data, const char* metadata) {
    return protocol_send_game_data(data_type, game_data, metadata);
}

UART_Status serial_comm_send_chat_message(const char* message, const char* sender, const char* chat_type) {
    return protocol_send_chat_message(message, sender, chat_type);
}

UART_Status serial_comm_send_command(const char* command, const char* parameters) {
    return protocol_send_command(command, parameters);
}

UART_Status serial_comm_send_status(uint8_t system_status, uint8_t error_code, const char* message) {
    return protocol_send_status(system_status, error_code, message);
}

UART_Status serial_comm_send_ack(void) {
    return protocol_send_ack();
}

UART_Status serial_comm_send_nack(void) {
    return protocol_send_nack();
}

UART_Status serial_comm_send_heartbeat(void) {
    return protocol_send_heartbeat();
}

/* Multiplayer data functions - map directly to protocol layer */
const char* serial_comm_get_client_id(void) {
    return protocol_get_client_id();
}

bool serial_comm_get_mp_game_over(void) {
    return protocol_get_mp_game_over();
}

bool serial_comm_get_player_assignment(int* player_id, char* session_id, size_t session_id_size,
                                      int* player_count, char* color, size_t color_size) {
    return protocol_get_player_assignment(player_id, session_id, session_id_size, player_count, color, color_size);
}

bool serial_comm
