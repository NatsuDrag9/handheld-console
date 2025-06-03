/*
 * snake_game_helpers.h
 *
 *  Created on: Jun 3, 2025
 *      Author: rohitimandi
 *
 *  Helper functions for snake game logic that can be shared between
 *  single-player and multi-player implementations
 */

#ifndef INC_GAME_ENGINE_GAMES_HELPERS_SNAKE_GAME_HELPERS_H_
#define INC_GAME_ENGINE_GAMES_HELPERS_SNAKE_GAME_HELPERS_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "Game_Engine/Games/game_types.h"
#include "Game_Engine/game_engine.h"
#include "Sprites/snake_sprite.h"

#define SNAKE_SPEED 500 // Movement delay in ms


// Snake state
typedef struct {
    coord_t head_x;
    coord_t head_y;
    uint8_t direction;  // Will use DPAD_DIR  values
    uint8_t length;
    struct {
        coord_t x;
        coord_t y;
    } body[64];  // Maximum snake length
} SnakeState;

// Co-ordinates of food being spawned. This is not inside SnakeState because each snake doesn't have its own food.
// Food is common to both snakes in multi-player game.
Position food;

// Movement and collision functions
void snake_helper_wrap_coordinates(coord_t* x, coord_t* y);
void snake_helper_move_snake(SnakeState* snake);
bool snake_helper_check_self_collision(SnakeState* snake);
bool snake_helper_check_food_collision(SnakeState* snake, const Position* food);
void snake_helper_grow_snake(SnakeState* snake);

// Direction handling
bool snake_helper_is_valid_direction_change(uint8_t current_direction, uint8_t new_direction);
void snake_helper_apply_direction_change(SnakeState* snake, uint8_t new_direction);

// Food spawning
void snake_helper_spawn_food(Position* food, const SnakeState* snake);

// Game speed calculation
uint16_t snake_helper_calculate_speed(uint32_t score);

// Snake initialization
void snake_helper_init_snake(SnakeState* snake, coord_t start_x, coord_t start_y, uint8_t start_direction);

// Rendering helpers
void snake_helper_draw_snake(const SnakeState* snake);
void snake_helper_draw_food(const Position* food);

// Utility functions
void snake_helper_copy_snake_state(SnakeState* dest, const SnakeState* src);
bool snake_helper_positions_overlap(coord_t x1, coord_t y1, coord_t x2, coord_t y2);


#endif /* INC_GAME_ENGINE_GAMES_HELPERS_SNAKE_GAME_HELPERS_H_ */
