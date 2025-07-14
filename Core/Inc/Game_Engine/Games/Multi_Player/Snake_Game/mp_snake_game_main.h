/*
 * mp_snake_main.h
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *  Main public interface for multiplayer snake game
 */

#ifndef INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_MAIN_H_
#define INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_MAIN_H_

#include "mp_snake_game_core.h"
#include "mp_snake_game_network.h"
#include "mp_snake_game_render.h"
#include "Game_Engine/game_engine.h"
#include "Game_Engine/Games/Helpers/snake_game_helpers.h"
#include "Console_Peripherals/Hardware/serial_comm.h"
#include <stdint.h>
#include <stdbool.h>

 // Forward declarations for internal modules
#include "mp_snake_game_core.h"
#include "mp_snake_game_network.h"
#include "mp_snake_game_render.h"

// Public interface functions
void mp_snake_update_dpad(DPAD_STATUS dpad_status);
void mp_snake_render(void);
void mp_snake_cleanup(void);
void mp_snake_process_communication(void);

// Communication related functions exposed
//bool mp_snake_is_communication_ready(void);
//bool mp_snake_is_websocket_connected(void);
//ProtocolState mp_snake_get_connection_state(void);

// Game state queries
MultiplayerGamePhase mp_snake_get_game_phase(void);
MultiplayerGameResult mp_snake_get_game_result(void);
uint32_t mp_snake_get_player_score(MultiplayerPlayerId player_id);
bool mp_snake_is_player_alive(MultiplayerPlayerId player_id);

// Multiplayer snake game engine instance
extern GameEngine mp_snake_game_engine;

#endif /* INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_MAIN_H_ */
