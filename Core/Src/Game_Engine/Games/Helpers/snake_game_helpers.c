/*
 * snake_game_helpers.c
 *
 *  Created on: Jun 3, 2025
 *      Author: rohitimandi
 *
 * Helper functions for snake game logic that can be shared between
 * single-player and multi-player implementations
 */


#include "Game_Engine/Games/Helpers/snake_game_helpers.h"

void snake_helper_wrap_coordinates(coord_t* x, coord_t* y) {
    // Handle X wrapping with proper underflow check
    if (*x >= DISPLAY_WIDTH - BORDER_OFFSET) {
        *x = BORDER_OFFSET;
    }
    else if (*x <= BORDER_OFFSET || *x > DISPLAY_WIDTH) {  // Catches underflow
        *x = DISPLAY_WIDTH - SPRITE_SIZE - BORDER_OFFSET;
    }

    // Handle Y wrapping
    if (*y >= DISPLAY_HEIGHT - BORDER_OFFSET) {
        *y = GAME_AREA_TOP;
    }
    else if (*y <= GAME_AREA_TOP || *y > DISPLAY_HEIGHT) {  // Catches underflow
        *y = DISPLAY_HEIGHT - SPRITE_SIZE - BORDER_OFFSET;
    }
}

void snake_helper_move_snake(SnakeState* snake) {
	coord_t prev_x = snake->head_x;
	coord_t prev_y = snake->head_y;

	switch(snake->direction) {
		case DPAD_DIR_RIGHT: snake->head_x += SPRITE_SIZE; break;
		case DPAD_DIR_LEFT:  snake->head_x -= SPRITE_SIZE; break;
		case DPAD_DIR_UP:    snake->head_y -= SPRITE_SIZE; break;
		case DPAD_DIR_DOWN:  snake->head_y += SPRITE_SIZE; break;
	}

	// Wrap around screen borders
	snake_helper_wrap_coordinates(&snake->head_x, &snake->head_y);

	// Update body segments
	for(uint8_t i = 0; i < snake->length; i++) {
		coord_t temp_x = snake->body[i].x;
		coord_t temp_y = snake->body[i].y;

		snake->body[i].x = prev_x;
		snake->body[i].y = prev_y;

		prev_x = temp_x;
		prev_y = temp_y;
	}
}

// Checks if snake collided with itself
bool snake_helper_check_self_collision(SnakeState* snake) {
	for (uint8_t i = 1; i < snake->length; i++) {
		coord_t body_x = snake->body[i].x;
		coord_t body_y = snake->body[i].y;
		snake_helper_wrap_coordinates(&body_x, &body_y);

		if (abs(snake->head_x - body_x) < SPRITE_SIZE &&
				abs(snake->head_y - body_y) < SPRITE_SIZE) {
			return true;
		}
	}
	return false;
}


// Checks if snake head collides with food
bool snake_helper_check_food_collision(SnakeState* snake, const Position* food) {
	coord_t food_x = food->x;
	coord_t food_y = food->y;
	snake_helper_wrap_coordinates(&food_x, &food_y);

	return (abs(snake->head_x - food_x) < SPRITE_SIZE && abs(snake->head_y - food_y) < SPRITE_SIZE);
}

// Grow snake by one segment
void snake_helper_grow_snake(SnakeState* snake) {
    if (snake->length < 63) {  // Prevent overflow
        snake->length++;
        // New segment takes position of previous tail
        snake->body[snake->length - 1].x = snake->body[snake->length - 2].x;
        snake->body[snake->length - 1].y = snake->body[snake->length - 2].y;
    }
}

// Check if direction change is valid (can't reverse directly)
bool snake_helper_is_valid_direction_change(uint8_t current_direction, uint8_t new_direction) {
    switch (new_direction) {
    case DPAD_DIR_RIGHT:
        return (current_direction != DPAD_DIR_LEFT);
    case DPAD_DIR_LEFT:
        return (current_direction != DPAD_DIR_RIGHT);
    case DPAD_DIR_UP:
        return (current_direction != DPAD_DIR_DOWN);
    case DPAD_DIR_DOWN:
        return (current_direction != DPAD_DIR_UP);
    default:
        return false;
    }
}

// Spawn food at random location not occupied by snake
void snake_helper_spawn_food(Position* food, const SnakeState* snake) {
    bool valid_position = false;
    int attempts = 0;
    const int max_attempts = 100;  // Prevent infinite loop

    while (!valid_position && attempts < max_attempts) {
        // Generate random position for food
        food->x = BORDER_OFFSET +
            (get_random() % (DISPLAY_WIDTH - 2 * BORDER_OFFSET - SPRITE_SIZE));
        food->y = GAME_AREA_TOP +
            (get_random() % (DISPLAY_HEIGHT - GAME_AREA_TOP - BORDER_OFFSET - SPRITE_SIZE));

        // Check if food spawns on snake head
        if (snake_helper_positions_overlap(food->x, food->y, snake->head_x, snake->head_y)) {
            attempts++;
            continue;
        }

        // Check if food spawns on snake body
        valid_position = true;
        for (uint8_t i = 0; i < snake->length; i++) {
            if (snake_helper_positions_overlap(food->x, food->y, snake->body[i].x, snake->body[i].y)) {
                valid_position = false;
                break;
            }
        }
        attempts++;
    }

    // If we couldn't find a valid position, use a default safe location
    if (!valid_position) {
        food->x = BORDER_OFFSET + SPRITE_SIZE;
        food->y = GAME_AREA_TOP + SPRITE_SIZE;
    }
}

// Calculate game speed based on score
uint16_t snake_helper_calculate_speed(uint32_t score) {
    uint16_t speed_reduction = (score / 20) * 20;  // Every 20 points, reduce by 20ms
    return (SNAKE_SPEED > speed_reduction) ? SNAKE_SPEED - speed_reduction : 200;  // Minimum time difference of 200ms
}

// Initialize snake at starting position
void snake_helper_init_snake(SnakeState* snake, coord_t start_x, coord_t start_y, uint8_t start_direction) {
    snake->head_x = start_x;
    snake->head_y = start_y;
    snake->direction = start_direction;
    snake->length = 1;

    // Initialize first body segment behind the head
    switch (start_direction) {
    case DPAD_DIR_RIGHT:
        snake->body[0].x = start_x - SPRITE_SIZE;
        snake->body[0].y = start_y;
        break;
    case DPAD_DIR_LEFT:
        snake->body[0].x = start_x + SPRITE_SIZE;
        snake->body[0].y = start_y;
        break;
    case DPAD_DIR_UP:
        snake->body[0].x = start_x;
        snake->body[0].y = start_y + SPRITE_SIZE;
        break;
    case DPAD_DIR_DOWN:
        snake->body[0].x = start_x;
        snake->body[0].y = start_y - SPRITE_SIZE;
        break;
    default:
        snake->body[0].x = start_x - SPRITE_SIZE;
        snake->body[0].y = start_y;
        break;
    }

    // Clear remaining body segments
    for (uint8_t i = 1; i < 64; i++) {
        snake->body[i].x = 0;
        snake->body[i].y = 0;
    }
}

// Draw snake using sprites
void snake_helper_draw_snake(const SnakeState* snake) {
    // Draw snake head with rotation based on direction
    uint16_t rotation = 0;
    switch (snake->direction) {
    case DPAD_DIR_RIGHT: rotation = 0;   break;
    case DPAD_DIR_DOWN:  rotation = 90;  break;
    case DPAD_DIR_LEFT:  rotation = 180; break;
    case DPAD_DIR_UP:    rotation = 270; break;
    }

    sprite_draw_rotated(&snake_head_animated.frames[snake_head_animated.current_frame],
        snake->head_x, snake->head_y, rotation, DISPLAY_WHITE);

    // Draw snake body segments
    for (uint8_t i = 0; i < snake->length; i++) {
        sprite_draw(&snake_body_sprite, snake->body[i].x, snake->body[i].y, DISPLAY_WHITE);
    }
}


// Apply direction change if valid
void snake_helper_apply_direction_change(SnakeState* snake, uint8_t new_direction) {
	if(snake_helper_is_valid_direction_change(snake->direction, new_direction)){
		snake->direction = new_direction;
	}
}

// Draw food using sprite
void snake_helper_draw_food(const Position* food) {
    sprite_draw(&food_sprite, food->x, food->y, DISPLAY_WHITE);
}

// Copy snake state from source to destination
void snake_helper_copy_snake_state(SnakeState* dest, const SnakeState* src) {
    memcpy(dest, src, sizeof(SnakeState));
}

// Check if two positions overlap (within sprite size)
bool snake_helper_positions_overlap(coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
    return (abs(x1 - x2) < SPRITE_SIZE && abs(y1 - y2) < SPRITE_SIZE);
}

// Convert server co-ordinates to device grid co-ordindates
coord_t mp_snake_server_to_device_coord(coord_t server_coord) {
    // Server uses 8-pixel units, device uses actual pixel coordinates
    return server_coord * MP_DEVICE_TILE_SIZE;
}

// For parsing values from event strings
uint8_t mp_snake_parse_single_value(const char* str, const char* key) {
    if (!str || !key) return 0;

    char search_key[16];
    snprintf(search_key, sizeof(search_key), "%s:", key);

    const char* pos = strstr(str, search_key);
    if (pos) {
        return (uint8_t)atoi(pos + strlen(search_key));
    }
    return 0;
}
