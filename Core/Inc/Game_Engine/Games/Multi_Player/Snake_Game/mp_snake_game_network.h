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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Communication/serial_comm.h"
#include "mp_snake_game_core.h"
#include "Utils/misc_utils.h"

 // Message sending functions
void mp_snake_send_input_to_server(uint8_t direction);
void mp_snake_send_player_ready(void);

// Message parsing functions
bool mp_snake_parse_player_update(const char* game_data);
bool mp_snake_parse_game_event(const char* game_data);
bool mp_snake_parse_game_state(const char* game_data);

// Callback handler for serial_comm
void mp_snake_on_game_data_received(const uart_game_data_t* game_data);
void mp_snake_on_connection_received(const uart_connection_message_t* communication_message);

#endif /* INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_NETWORK_H_ */
