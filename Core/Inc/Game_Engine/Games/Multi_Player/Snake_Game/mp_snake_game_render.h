/*
 * mp_snake_game_render.h
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *  Rendering and UI functions for multiplayer snake
 */

#ifndef INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_RENDER_H_
#define INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_RENDER_H_

#include "mp_snake_game_core.h"
#include "mp_snake_game_network.h"
#include <stdint.h>
#include <stdbool.h>

// Rendering initialization and cleanup (similar to TS constructor/destructor)
void mp_snake_render_init(void);
void mp_snake_render_cleanup(void);

// Main rendering function
void mp_snake_render_game(
    MultiplayerGamePhase game_phase,
    SnakeState* players,
    uint8_t player_count,
    Position* food,
    GameStats* game_stats,
    ProtocolState connection_status,
    uint8_t local_player_id,
    bool is_spectator
);


#endif /* INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_RENDER_H_ */
