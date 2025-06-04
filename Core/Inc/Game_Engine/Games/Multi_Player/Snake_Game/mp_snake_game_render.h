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

// Rendering optimization tracking
extern coord_t previous_local_head_x, previous_local_head_y;
extern coord_t previous_opponent_head_x, previous_opponent_head_y;
extern coord_t previous_food_x, previous_food_y;
extern uint8_t previous_local_length;
extern uint8_t previous_opponent_length;
extern uint32_t previous_local_score;
extern uint32_t previous_opponent_score;
extern bool first_render;

// Main rendering function
void mp_snake_render_game(void);

// Rendering initialization and cleanup
void mp_snake_render_init(void);
void mp_snake_render_cleanup(void);

// Phase-specific rendering functions
void mp_snake_render_waiting_screen(void);
void mp_snake_render_game_area(void);
void mp_snake_render_multiplayer_ui(void);
void mp_snake_render_game_over_screen(void);

// Optimization functions
void mp_snake_clear_previous_positions(void);
void mp_snake_reset_render_tracking(void);


#endif /* INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_RENDER_H_ */
