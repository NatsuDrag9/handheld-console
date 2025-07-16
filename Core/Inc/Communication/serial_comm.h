/*
 * serial_comm.h
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_COMMUNICATION_SERIAL_COMM_H_
#define INC_COMMUNICATION_SERIAL_COMM_H_

/* Include all communication layers */
#include "Console_Peripherals/Hardware/serial_comm_core.h"
#include "Communication/serial_comm_protocol.h"
#include "Communication/serial_comm_callbacks.h"

/* Main communication API - these are the functions applications should use */

/* Core communication system */
UART_Status serial_comm_init(void);
UART_Status serial_comm_deinit(void);
void serial_comm_process_messages(void);
UART_Status serial_comm_reset(void);

/* Connection status (maps to protocol layer) */
bool serial_comm_is_esp32_ready(void);
bool serial_comm_is_wifi_connected(void);
bool serial_comm_is_websocket_connected(void);
ProtocolState serial_comm_get_state(void);
bool serial_comm_needs_ui_update(void);
void serial_comm_clear_ui_update_flag(void);

/* Error handling (maps to protocol layer) */
bool serial_comm_has_network_error(void);
void serial_comm_clear_network_error(void);
const char* serial_comm_get_error_message(void);

/* Basic message sending (maps to protocol layer) */
UART_Status serial_comm_send_game_data(const char* data_type, const char* game_data, const char* metadata);
UART_Status serial_comm_send_chat_message(const char* message, const char* sender, const char* chat_type);
UART_Status serial_comm_send_command(const char* command, const char* parameters);
UART_Status serial_comm_send_status(uint8_t system_status, uint8_t error_code, const char* message);
UART_Status serial_comm_send_ack(void);
UART_Status serial_comm_send_nack(void);
UART_Status serial_comm_send_heartbeat(void);

/* Multiplayer data (maps to protocol layer) */
const char* serial_comm_get_client_id(void);
bool serial_comm_get_mp_game_over(void);
bool serial_comm_get_player_assignment(int* player_id, char* session_id, size_t session_id_size,
                                      int* player_count, char* color, size_t color_size);
bool serial_comm_get_opponent_data(int* player_id, char* session_id, size_t session_id_size,
                                  int* player_count, char* color, size_t color_size);
bool serial_comm_has_player_assignment(void);
bool serial_comm_has_opponent_connected(void);
int serial_comm_get_local_player_id(void);
int serial_comm_get_opponent_player_id(void);

/* Callback registration (maps to callbacks layer) */
void serial_comm_register_game_data_callback(game_data_received_callback_t callback);
void serial_comm_register_chat_message_callback(chat_message_received_callback_t callback);
void serial_comm_register_command_callback(command_received_callback_t callback);
void serial_comm_register_status_callback(status_received_callback_t callback);
void serial_comm_register_connection_message_callback(connection_message_callback_t callback);

/* Utility functions */
void serial_comm_print_stats(void);
void serial_comm_send_debug(const char* message, uint32_t timeout);

#endif /* INC_COMMUNICATION_SERIAL_COMM_H_ */
