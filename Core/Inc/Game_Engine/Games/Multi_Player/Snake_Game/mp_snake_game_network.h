/*
 * mp_snake_game_network.h
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *  Network communication and message parsing for multiplayer snake
 */

#ifndef INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_NETWORK_H_
#define INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_NETWORK_H_

#include "Console_Peripherals/Hardware/serial_comm.h"
#include "mp_snake_game_core.h"
#include <stdint.h>
#include <stdbool.h>

// Network initialization and shutdown
bool mp_snake_initialize_communication(void);
void mp_snake_shutdown_communication(void);
void mp_snake_process_communication(void);

// Message sending functions
void mp_snake_send_input_to_server(uint8_t direction);
void mp_snake_send_player_ready(void);

// Message handling callbacks (called by serial communication)
void mp_snake_on_game_data_received(const uart_game_data_t* game_data);
void mp_snake_on_status_received(const uart_status_t* status);
void mp_snake_on_command_received(const uart_command_t* command);

// Legacy compatibility
void mp_snake_handle_server_message(const uart_game_data_t* server_msg);

// Connection status queries
bool mp_snake_is_communication_ready(void);
bool mp_snake_is_websocket_connected(void);
ProtocolState mp_snake_get_connection_state(void);

// Message parsing functions (internal)
bool mp_snake_parse_player_update(const char* game_data);
bool mp_snake_parse_game_event(const char* game_data);
bool mp_snake_parse_game_state(const char* game_data);
void mp_snake_parse_coordinate_pair(const char* str, coord_t* x, coord_t* y);
uint8_t mp_snake_parse_single_value(const char* str, const char* key);


#endif /* INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_NETWORK_H_ */
