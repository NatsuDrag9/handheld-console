/*
 * serial_comm_protocol.h
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_COMMUNICATION_SERIAL_COMM_PROTOCOL_H_
#define INC_COMMUNICATION_SERIAL_COMM_PROTOCOL_H_

#include "serial_comm_types.h"
#include "System/system_conf.h"
#include "Utils/comm_utils.h"

/* Protocol initialization and control */
void protocol_init(void);
void protocol_deinit(void);
void protocol_process(void);
void protocol_reset(void);

/* Connection status functions */
bool protocol_is_esp32_ready(void);
bool protocol_is_wifi_connected(void);
bool protocol_is_websocket_connected(void);
ProtocolState protocol_get_state(void);
bool protocol_needs_ui_update(void);
void protocol_clear_ui_update_flag(void);

/* Enhanced error handling functions */
bool protocol_has_network_error(void);
void protocol_clear_network_error(void);
const char* protocol_get_error_message(void);

/* Message sending functions */
UART_Status protocol_send_message(MessageType type, const uint8_t* data, uint8_t length);
UART_Status protocol_send_game_data(const char* data_type, const char* game_data, const char* metadata);
UART_Status protocol_send_chat_message(const char* message, const char* sender, const char* chat_type);
UART_Status protocol_send_command(const char* command, const char* parameters);
UART_Status protocol_send_status(uint8_t system_status, uint8_t error_code, const char* message);
UART_Status protocol_send_connection_message(const char* message, const char* client_id);
UART_Status protocol_send_tile_size_validation(uint8_t tile_size);
UART_Status protocol_send_ack(void);
UART_Status protocol_send_nack(void);
UART_Status protocol_send_heartbeat(void);

/* Multiplayer communication getter Functions */
const char* protocol_get_client_id(void);
bool protocol_get_mp_game_over(void);
bool protocol_get_player_assignment(int* player_id, char* session_id, size_t session_id_size,
                                   int* player_count, char* color, size_t color_size);
bool protocol_get_opponent_data(int* player_id, char* session_id, size_t session_id_size,
                               int* player_count, char* color, size_t color_size);
bool protocol_has_player_assignment(void);
bool protocol_has_opponent_connected(void);
int protocol_get_local_player_id(void);
int protocol_get_opponent_player_id(void);

/* Utility functions */
void protocol_send_debug(const char* message, uint32_t timeout);
void protocol_print_stats(void);



#endif /* INC_COMMUNICATION_SERIAL_COMM_PROTOCOL_H_ */
