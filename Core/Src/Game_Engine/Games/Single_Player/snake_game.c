/*
 * snake_game.c
 *
 *  Created on: Jun 3, 2025
 *      Author: rohitimandi
 */

#include <stdlib.h>
#include "Game_Engine/Games/Single_Player/snake_game.h"
#include "Utils/debug_conf.h"
#include "Utils/misc_utils.h"

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
static void snake_show_game_over_message(void);

// Forward declarations pf rendering functions
static void render_status_area(bool force_redraw);
static void clear_and_redraw_border_if_needed(coord_t x, coord_t y);
static void clear_previous_head_position(SnakeGameData* data);
static void clear_previous_tail_position(SnakeGameData* data);
static void clear_previous_food_position(SnakeGameData* data);
static void draw_snake_and_food(SnakeGameData* data);

SnakeGameData snake_data = {
    .snake = {
        .head_x = 0,
        .head_y = 0,
        .direction = DPAD_DIR_RIGHT,
        .length = DEFAULT_LIVES,
        .body = {{0}}
    },
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
    .show_game_over_message = snake_show_game_over_message,
    .game_data = &snake_data,
    .base_state = {
        .state_data = {
            .single = {
                .score = 0,
                .lives = 1,
            }
        },
        .paused = false,
        .game_over = false,
        .is_reset = false
    },
    .is_d_pad_game = true,  // Snake is a D-pad game
    .is_mp_game = false,
};

static void snake_init(void) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;

    snake_helper_init_snake(&data->snake, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, DPAD_DIR_RIGHT);

    // Initialize food position
    data->food.x = 16;
    data->food.y = 16;

    // Reset dirty rectangle tracking
    first_render = true;
    previous_head_x = data->snake.head_x;
    previous_head_y = data->snake.head_y;
    previous_tail_x = data->snake.body[0].x;
    previous_tail_y = data->snake.body[0].y;
    previous_food_x = data->food.x;
    previous_food_y = data->food.y;
    previous_length = data->snake.length;
    previous_score = 0;
    previous_lives = DEFAULT_LIVES;

    last_move_time = get_current_ms();
}

static void handle_direction_change(SnakeGameData* data, DPAD_STATUS dpad_status) {
    if (!dpad_status.is_new) return;
    snake_helper_apply_direction_change(&data->snake, dpad_status.direction);
}

static void handle_food_collision(SnakeGameData* data) {

    if (snake_helper_check_food_collision(&data->snake, &data->food)) {

        snake_helper_grow_snake(&data->snake);
        snake_game_engine.base_state.state_data.single.score += 10;

        snake_helper_spawn_food(&data->food, &data->snake);
    }
}

static void handle_collision(SnakeGameData* data) {
    if (snake_helper_check_self_collision(&data->snake)) {
        if (snake_game_engine.base_state.state_data.single.lives > 0) {
            snake_game_engine.base_state.state_data.single.lives--;
            if (snake_game_engine.base_state.state_data.single.lives == 0) {
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

static void snake_update_dpad(DPAD_STATUS dpad_status) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;
    uint32_t current_time = get_current_ms();

    handle_direction_change(data, dpad_status);

    // Use helper function to calculate snake speed
    uint16_t current_speed = snake_helper_calculate_speed(snake_game_engine.base_state.state_data.single.score);
    if (current_time - last_move_time >= current_speed) {
        // Store previous positions for rendering optimization
        previous_head_x = data->snake.head_x;
        previous_head_y = data->snake.head_y;
        if (data->snake.length > 0) {
            previous_tail_x = data->snake.body[data->snake.length - 1].x;
            previous_tail_y = data->snake.body[data->snake.length - 1].y;
        }

        snake_helper_move_snake(&data->snake);

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
        previous_score == snake_game_engine.base_state.state_data.single.score &&
        previous_lives == snake_game_engine.base_state.state_data.single.lives) {
        return;
    }

    // Clear the status area at the top without affecting the border
    display_fill_rectangle(2, 2, DISPLAY_WIDTH - 2, STATUS_START_Y - 1, DISPLAY_BLACK);

    // Draw score and lives
    char status_text[32];
    snprintf(status_text, sizeof(status_text), "Score: %lu Lives: %d",
        snake_game_engine.base_state.state_data.single.score,
        snake_game_engine.base_state.state_data.single.lives);
    display_set_cursor(2, 2);
#ifdef DISPLAY_MODULE_LCD
    display_write_string(status_text, Font_11x18, DISPLAY_WHITE);
#else
    display_write_string(status_text, Font_7x10, DISPLAY_WHITE);
#endif

    // Update previous values
    previous_score = snake_game_engine.base_state.state_data.single.score;
    previous_lives = snake_game_engine.base_state.state_data.single.lives;
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
    if (previous_head_x == data->snake.head_x && previous_head_y == data->snake.head_y) {
        return; // Head didn't move
    }

    // Only clear if not occupied by a body segment
    bool is_occupied = false;
    for (uint8_t i = 0; i < data->snake.length; i++) {
        if (previous_head_x == data->snake.body[i].x && previous_head_y == data->snake.body[i].y) {
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
        }
        else {
            // Near border - clear and redraw border
            clear_and_redraw_border_if_needed(previous_head_x, previous_head_y);
        }
    }
}

// Function to clear previous tail position
static void clear_previous_tail_position(SnakeGameData* data) {
    // Only needed when snake didn't grow
    if (previous_length != data->snake.length || (previous_tail_x == 0 && previous_tail_y == 0)) {
        return;
    }

    // Check if previous tail position is part of current snake
    bool is_part_of_snake = false;
    if (previous_tail_x == data->snake.head_x && previous_tail_y == data->snake.head_y) {
        is_part_of_snake = true;
    }
    else {
        for (uint8_t i = 0; i < data->snake.length; i++) {
            if (previous_tail_x == data->snake.body[i].x && previous_tail_y == data->snake.body[i].y) {
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
        }
        else {
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

// Function to draw the snake and food using helper functions
static void draw_snake_and_food(SnakeGameData* data) {
    snake_helper_draw_snake(&data->snake);
    snake_helper_draw_food(&data->food);

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
        //        render_status_area(true);
        first_render = false;
    }

    // Update status area if score or lives changed
    bool status_changed = (previous_score != snake_game_engine.base_state.state_data.single.score) ||
        (previous_lives != snake_game_engine.base_state.state_data.single.lives);
    if (status_changed) {
        //        render_status_area(true);
    }

    // Clear previous positions if needed
    clear_previous_head_position(data);
    clear_previous_tail_position(data);
    clear_previous_food_position(data);

    // Draw current game elements using helper functions
    draw_snake_and_food(data);

    // Update tracking for next frame
    previous_length = data->snake.length;
}

static void snake_show_game_over_message(void) {
    if (snake_game_engine.base_state.state_data.single.score >= 100) {
        display_manager_show_game_over_message("Well Played", snake_game_engine.base_state.state_data.single.score);
    }
    else {
        display_manager_show_game_over_message(NULL, snake_game_engine.base_state.state_data.single.score);
    }
}

static void snake_cleanup(void) {
    // Reset game state using helper structure
    memset(&snake_data.snake, 0, sizeof(SnakeState));
    snake_data.snake.direction = DPAD_DIR_RIGHT;
    snake_data.snake.length = 1;

    memset(&snake_data.food, 0, sizeof(Position));

    // Reset timing
    last_move_time = 0;

    // Reset game engine state
    snake_game_engine.base_state.state_data.single.score = 0;
    snake_game_engine.base_state.state_data.single.lives = DEFAULT_LIVES;
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
