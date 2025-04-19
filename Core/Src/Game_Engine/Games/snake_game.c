
/*
 * snake_game.c
 *
 *  Created on: Jan 15, 2025
 *      Author: rohitimandi
 *  Modified on: Apr 14, 2025
 *      Added dirty rectangle optimization
 */

#include "Game_Engine/Games/snake_game.h"
#include "Utils/debug_conf.h"
#include "Utils/misc_utils.h"
#include <stdlib.h>

static uint32_t last_move_time = 0;

// For dirty rectangle optimization - use separate coordinates instead of Position struct
static coord_t previous_head_x = 0, previous_head_y = 0;
static coord_t previous_tail_x = 0, previous_tail_y = 0;
static coord_t previous_food_x = 0, previous_food_y = 0;
static uint8_t previous_length = 0;
static uint32_t previous_score = 0;
static uint8_t previous_lives = 0;
static bool first_render = true;

// Forward declarations of game engine functions
static void snake_init(void);
static void snake_update_dpad(DPAD_STATUS dpad_status);
static void snake_render(void);
static void snake_cleanup(void);
static void wrap_coordinates(coord_t* x, coord_t* y);
static uint16_t calculate_snake_speed(uint32_t score);
static void spawn_food(SnakeGameData* data);

// Forward declarations for new rendering functions
static void render_status_area(bool force_redraw);
static void clear_and_redraw_border_if_needed(coord_t x, coord_t y);
static void clear_previous_head_position(SnakeGameData* data);
static void clear_previous_tail_position(SnakeGameData* data);
static void clear_previous_food_position(SnakeGameData* data);
static void draw_snake_and_food(SnakeGameData* data);

SnakeGameData snake_data = {
    .head_x = 0,
    .head_y = 0,
    .direction = DPAD_DIR_RIGHT,
    .length = 1,
    .body = {{0}},
    .food = {
        .x = 0,
        .y = 0
    }
};

// Initialize the snake game engine instance
GameEngine snake_game_engine = {
    .init = snake_init,
    .render = snake_render,
    .cleanup = snake_cleanup,
    .update_func = {
        .update_dpad = snake_update_dpad
    },
    .game_data = &snake_data,
    .base_state = {
        .score = 0,
        .lives = 3,
        .paused = false,
        .game_over = false,
        .is_reset = false
    },
    .is_d_pad_game = true  // Snake is a D-pad game
};

static void wrap_coordinates(coord_t* x, coord_t* y) {
    if (*x >= DISPLAY_WIDTH) *x = BORDER_OFFSET;
    if (*x < BORDER_OFFSET) *x = DISPLAY_WIDTH - SPRITE_SIZE - BORDER_OFFSET;
    if (*y >= DISPLAY_HEIGHT) *y = GAME_AREA_TOP;
    if (*y < GAME_AREA_TOP) *y = DISPLAY_HEIGHT - SPRITE_SIZE - BORDER_OFFSET;
}

static void snake_init(void) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;

    // Initialize snake position (center of screen)
    data->head_x = DISPLAY_WIDTH / 2;
    data->head_y = DISPLAY_HEIGHT / 2;
    data->direction = DPAD_DIR_RIGHT;  // Start moving right
    data->length = 1;     // Start with just the head

    // Initialize first body segment
    data->body[0].x = data->head_x - 8;  // 8 pixels is sprite width
    data->body[0].y = data->head_y;

    // Initialize food position
    data->food.x = 16;
    data->food.y = 16;

    // Reset dirty rectangle tracking
    first_render = true;
    previous_head_x = data->head_x;
    previous_head_y = data->head_y;
    previous_tail_x = data->body[0].x;
    previous_tail_y = data->body[0].y;
    previous_food_x = data->food.x;
    previous_food_y = data->food.y;
    previous_length = data->length;
    previous_score = 0;
    previous_lives = 3;

    last_move_time = get_current_ms();
}

static void spawn_food(SnakeGameData* data) {
    // Store previous food position for clearing
    previous_food_x = data->food.x;
    previous_food_y = data->food.y;

    // Generate random position for food
    data->food.x = BORDER_OFFSET +
        (get_random() % (DISPLAY_WIDTH - 2 * BORDER_OFFSET - SPRITE_SIZE));
    data->food.y = GAME_AREA_TOP +
        (get_random() % (DISPLAY_HEIGHT - GAME_AREA_TOP - BORDER_OFFSET - SPRITE_SIZE));

    // Ensure food doesn't spawn on snake
    for (uint8_t i = 0; i < data->length; i++) {
        if (abs(data->food.x - data->body[i].x) < SPRITE_SIZE &&
            abs(data->food.y - data->body[i].y) < SPRITE_SIZE) {
            spawn_food(data);  // Try again
            return;
        }
    }
}

static bool check_collision(SnakeGameData* data) {
    for (uint8_t i = 1; i < data->length; i++) {
        coord_t body_x = data->body[i].x;
        coord_t body_y = data->body[i].y;
        wrap_coordinates(&body_x, &body_y);

        if (abs(data->head_x - body_x) < SPRITE_SIZE &&
            abs(data->head_y - body_y) < SPRITE_SIZE) {
            return true;
        }
    }
    return false;
}

static void handle_direction_change(SnakeGameData* data, DPAD_STATUS dpad_status) {
    if (!dpad_status.is_new) return;

    coord_t new_direction = data->direction;
    switch (dpad_status.direction) {
    case DPAD_DIR_RIGHT:
        if (data->direction != DPAD_DIR_LEFT) new_direction = DPAD_DIR_RIGHT;
        break;
    case DPAD_DIR_LEFT:
        if (data->direction != DPAD_DIR_RIGHT) new_direction = DPAD_DIR_LEFT;
        break;
    case DPAD_DIR_UP:
        if (data->direction != DPAD_DIR_DOWN) new_direction = DPAD_DIR_UP;
        break;
    case DPAD_DIR_DOWN:
        if (data->direction != DPAD_DIR_UP) new_direction = DPAD_DIR_DOWN;
        break;
    }
    data->direction = new_direction;
}

static void move_snake(SnakeGameData* data) {
    // Store the previous head position
    previous_head_x = data->head_x;
    previous_head_y = data->head_y;

    // Store the tail position before moving (for clearing)
    if (data->length > 0) {
        previous_tail_x = data->body[data->length - 1].x;
        previous_tail_y = data->body[data->length - 1].y;
    }

    coord_t prev_x = data->head_x;
    coord_t prev_y = data->head_y;

    switch (data->direction) {
    case DPAD_DIR_RIGHT: data->head_x += SPRITE_SIZE; break;
    case DPAD_DIR_LEFT:  data->head_x -= SPRITE_SIZE; break;
    case DPAD_DIR_UP:    data->head_y -= SPRITE_SIZE; break;
    case DPAD_DIR_DOWN:  data->head_y += SPRITE_SIZE; break;
    }

    // Wrap around screen borders
    wrap_coordinates(&data->head_x, &data->head_y);

    for (uint8_t i = 0; i < data->length; i++) {
        coord_t temp_x = data->body[i].x;
        coord_t temp_y = data->body[i].y;
        data->body[i].x = prev_x;
        data->body[i].y = prev_y;
        prev_x = temp_x;
        prev_y = temp_y;
    }
}

static void handle_food_collision(SnakeGameData* data) {
    coord_t food_x = data->food.x;
    coord_t food_y = data->food.y;
    wrap_coordinates(&food_x, &food_y);

    if (abs(data->head_x - food_x) < SPRITE_SIZE &&
        abs(data->head_y - food_y) < SPRITE_SIZE) {
        data->length++;
        data->body[data->length - 1].x = data->body[data->length - 2].x;
        data->body[data->length - 1].y = data->body[data->length - 2].y;
        snake_game_engine.base_state.score += 10;
        spawn_food(data);
    }
}

static void handle_collision(SnakeGameData* data) {
    if (check_collision(data)) {
        if (snake_game_engine.base_state.lives > 0) {
            snake_game_engine.base_state.lives--;
            if (snake_game_engine.base_state.lives == 0) {
                snake_game_engine.base_state.game_over = true;
            }
            else {
            	 // Clear the game area before reinitializing the snake
            	display_fill_rectangle(
            			BORDER_OFFSET,
						GAME_AREA_TOP,
						DISPLAY_WIDTH - BORDER_OFFSET,
						DISPLAY_HEIGHT - BORDER_OFFSET,
						DISPLAY_BLACK
            	);
                snake_init();
            }
        }
    }
}

static uint16_t calculate_snake_speed(uint32_t score) {
    uint16_t speed_reduction = (score / 20) * 20;  // Every 20 points, reduce by 20ms
    return (SNAKE_SPEED > speed_reduction) ? SNAKE_SPEED - speed_reduction : 200;  // Minimum time difference of 200ms -> maximum speed
}

static void snake_update_dpad(DPAD_STATUS dpad_status) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;
    uint32_t current_time = get_current_ms();

    handle_direction_change(data, dpad_status);

    uint16_t current_speed = calculate_snake_speed(snake_game_engine.base_state.score);
    if (current_time - last_move_time >= current_speed) {
        move_snake(data);
        handle_food_collision(data);
        handle_collision(data);
        last_move_time = current_time;
    }

    animated_sprite_update(&snake_head_animated);
}


// Function to render the score and lives in the status area
static void render_status_area(bool force_redraw) {
    // Check if there's a reason to redraw
    if (!force_redraw &&
        previous_score == snake_game_engine.base_state.score &&
        previous_lives == snake_game_engine.base_state.lives) {
        return;
    }

    // Clear the status area at the top without affecting the border
    display_fill_rectangle(2, 2, DISPLAY_WIDTH - 2, STATUS_START_Y - 1, DISPLAY_BLACK);

    // Draw score and lives
    char status_text[32];
    snprintf(status_text, sizeof(status_text), "Score: %lu Lives: %d",
        snake_game_engine.base_state.score,
        snake_game_engine.base_state.lives);
    display_set_cursor(2, 2);
#ifdef DISPLAY_MODULE_LCD
    display_write_string(status_text, Font_11x18, DISPLAY_WHITE);
#else
    display_write_string(status_text, Font_7x10, DISPLAY_WHITE);
#endif

    // Update previous values
    previous_score = snake_game_engine.base_state.score;
    previous_lives = snake_game_engine.base_state.lives;
}

// Function to clear a region and redraw border if needed
static void clear_and_redraw_border_if_needed(coord_t x, coord_t y) {
    // Check if position is near any border
    bool near_border = (x <= BORDER_OFFSET + SPRITE_SIZE ||
                       x >= DISPLAY_WIDTH - BORDER_OFFSET - SPRITE_SIZE - 1 ||
                       y <= GAME_AREA_TOP + SPRITE_SIZE ||
                       y >= DISPLAY_HEIGHT - BORDER_OFFSET - SPRITE_SIZE - 1);

    // Clear the region
    display_clear_region(x, y, SPRITE_SIZE, SPRITE_SIZE);

    // Redraw border if needed
    if (near_border) {
        display_draw_border_at(1, STATUS_START_Y, 3, 3);
    }
}

// Function to clear previous head position
static void clear_previous_head_position(SnakeGameData* data) {
    if (previous_head_x == data->head_x && previous_head_y == data->head_y) {
        return; // Head didn't move
    }

    // Only clear if not occupied by a body segment
    bool is_occupied = false;
    for (uint8_t i = 0; i < data->length; i++) {
        if (previous_head_x == data->body[i].x && previous_head_y == data->body[i].y) {
            is_occupied = true;
            break;
        }
    }

    if (!is_occupied) {
        // Check if we can safely clear with buffer
        if (previous_head_x > BORDER_OFFSET &&
            previous_head_x < DISPLAY_WIDTH - BORDER_OFFSET - SPRITE_SIZE &&
            previous_head_y > GAME_AREA_TOP &&
            previous_head_y < DISPLAY_HEIGHT - BORDER_OFFSET - SPRITE_SIZE) {
            display_clear_region(previous_head_x, previous_head_y, SPRITE_SIZE, SPRITE_SIZE);
        } else {
            // Near border - clear and redraw border
            clear_and_redraw_border_if_needed(previous_head_x, previous_head_y);
        }
    }
}

// Function to clear previous tail position
static void clear_previous_tail_position(SnakeGameData* data) {
    // Only needed when snake didn't grow
    if (previous_length != data->length || (previous_tail_x == 0 && previous_tail_y == 0)) {
        return;
    }

    // Check if previous tail position is part of current snake
    bool is_part_of_snake = false;
    if (previous_tail_x == data->head_x && previous_tail_y == data->head_y) {
        is_part_of_snake = true;
    } else {
        for (uint8_t i = 0; i < data->length; i++) {
            if (previous_tail_x == data->body[i].x && previous_tail_y == data->body[i].y) {
                is_part_of_snake = true;
                break;
            }
        }
    }

    if (!is_part_of_snake) {
        // Check if we can safely clear with buffer
        if (previous_tail_x > BORDER_OFFSET + 1 &&
            previous_tail_x < DISPLAY_WIDTH - BORDER_OFFSET - SPRITE_SIZE - 1 &&
            previous_tail_y > GAME_AREA_TOP + 1 &&
            previous_tail_y < DISPLAY_HEIGHT - BORDER_OFFSET - SPRITE_SIZE - 1) {
            // Safe to clear with buffer
            display_clear_region(
                previous_tail_x - 1,
                previous_tail_y - 1,
                SPRITE_SIZE + 2,
                SPRITE_SIZE + 2
            );
        } else {
            // Near border - clear and redraw border
            clear_and_redraw_border_if_needed(previous_tail_x, previous_tail_y);
        }
    }
}

// Function to clear previous food position
static void clear_previous_food_position(SnakeGameData* data) {
    if ((previous_food_x == data->food.x && previous_food_y == data->food.y) ||
        previous_food_x == 0 || previous_food_y == 0) {
        return; // Food didn't move or is not initialized
    }

    clear_and_redraw_border_if_needed(previous_food_x, previous_food_y);
}

// Function to draw the snake and food
static void draw_snake_and_food(SnakeGameData* data) {
    // Draw snake head with rotation based on direction
    uint16_t rotation = 0;
    switch (data->direction) {
    case DPAD_DIR_RIGHT: rotation = 0;   break;
    case DPAD_DIR_DOWN:  rotation = 90;  break;
    case DPAD_DIR_LEFT:  rotation = 180; break;
    case DPAD_DIR_UP:    rotation = 270; break;
    }

    sprite_draw_rotated(&snake_head_animated.frames[snake_head_animated.current_frame],
        data->head_x, data->head_y, rotation, DISPLAY_WHITE);

    // Draw snake body segments
    for (uint8_t i = 0; i < data->length; i++) {
        sprite_draw(&snake_body_sprite, data->body[i].x, data->body[i].y, DISPLAY_WHITE);
    }

    // Draw food
    sprite_draw(&food_sprite, data->food.x, data->food.y, DISPLAY_WHITE);

    // Periodically redraw border to ensure it's intact
    if (get_current_ms() % 500 == 0) {
        display_draw_border_at(1, STATUS_START_Y, 3, 3);
    }
}

static void snake_render(void) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;

    // Initialize display on first render
    if (first_render) {
        // Draw border
        display_draw_border_at(1, STATUS_START_Y, 3, 3);
        render_status_area(true);
        first_render = false;
    }

    // Update status area if score or lives changed
    bool status_changed = (previous_score != snake_game_engine.base_state.score) ||
                         (previous_lives != snake_game_engine.base_state.lives);
    if (status_changed) {
        render_status_area(true);
    }

    // Clear previous positions if needed
    clear_previous_head_position(data);
    clear_previous_tail_position(data);
    clear_previous_food_position(data);

    // Draw current game elements
    draw_snake_and_food(data);

    // Game over message
    if (snake_game_engine.base_state.game_over) {
        display_write_string_centered("GAME OVER", Font_7x10, 30, DISPLAY_WHITE);
    }

    // Update tracking for next frame
    previous_length = data->length;
}


static void snake_cleanup(void) {
    // Reset game state
    snake_data.head_x = 0;
    snake_data.head_y = 0;
    snake_data.direction = DPAD_DIR_RIGHT;  // Using DPAD direction constant
    snake_data.length = 1;
    memset(snake_data.body, 0, sizeof(snake_data.body));
    snake_data.food.x = 0;
    snake_data.food.y = 0;

    // Reset timing
    last_move_time = 0;

    // Reset game engine state
    snake_game_engine.base_state.score = 0;
    snake_game_engine.base_state.lives = 3;
    snake_game_engine.base_state.paused = false;
    snake_game_engine.base_state.game_over = false;

    // Reset dirty rectangle tracking
    first_render = true;
    previous_head_x = 0;
    previous_head_y = 0;
    previous_tail_x = 0;
    previous_tail_y = 0;
    previous_food_x = 0;
    previous_food_y = 0;
    previous_length = 0;
    previous_score = 0;
    previous_lives = 0;
}
