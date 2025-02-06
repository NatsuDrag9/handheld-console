/*
 * snake_game.c
 *
 *  Created on: Jan 15, 2025
 *      Author: rohitimandi
 */

#include "Game_Engine/Games/snake_game.h"
#include "Utils/debug_conf.h"
#include "Utils/misc_utils.h"
#include <stdlib.h>

static uint32_t last_move_time = 0;

static SnakeGameData snake_data = {
    .head_x = 0,           // Will be initialized properly in snake_init()
    .head_y = 0,           // Will be initialized properly in snake_init()
    .direction = JS_DIR_RIGHT,
    .length = 1,
    .body = {{0}},        // Initialize all body segments to 0
    .food = {
        .x = 0,
        .y = 0
    }
};

// Forward declarations of game engine functions
static void snake_init(void);
static void snake_update(JoystickStatus js_status);
static void snake_render(void);
static void snake_cleanup(void);
static void wrap_coordinates(uint8_t* x, uint8_t* y);
static uint16_t calculate_snake_speed(uint32_t score);
static void spawn_food(SnakeGameData* data);

// Initialize the snake game engine instance
GameEngine snake_game_engine = {
    .init = snake_init,
    .update = snake_update,
    .render = snake_render,
    .cleanup = snake_cleanup,
    .game_data = &snake_data,
    .base_state = {
        .score = 0,
        .lives = 3,
        .paused = false,
        .game_over = false
    }
};

static void wrap_coordinates(uint8_t* x, uint8_t* y) {
    if (*x >= DISPLAY_WIDTH) *x = BORDER_OFFSET;
    if (*x < BORDER_OFFSET) *x = DISPLAY_WIDTH - SNAKE_SPRITE_SIZE - BORDER_OFFSET;
    if (*y >= DISPLAY_HEIGHT) *y = GAME_AREA_TOP;
    if (*y < GAME_AREA_TOP) *y = DISPLAY_HEIGHT - SNAKE_SPRITE_SIZE - BORDER_OFFSET;
}

static void snake_init(void) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;

    // Initialize snake position (center of screen)
    data->head_x = DISPLAY_WIDTH / 2;
    data->head_y = DISPLAY_HEIGHT / 2;
    data->direction = JS_DIR_RIGHT;  // Start moving right
    data->length = 1;     // Start with just the head

    // Initialize first body segment
    data->body[0].x = data->head_x - 8;  // 8 pixels is sprite width
    data->body[0].y = data->head_y;

    // Initialize food position (random position will be implemented later)
    data->food.x = 16;
    data->food.y = 16;

    //    DEBUG_PRINTF(false, "Snake initialized: x=%d, y=%d, direction=%d\n",
    //                  data->head_x, data->head_y, data->direction);

    last_move_time = get_current_ms();
}

static void spawn_food(SnakeGameData* data) {
    // Generate random position for food
    data->food.x = BORDER_OFFSET +
        (get_random() % (DISPLAY_WIDTH - 2 * BORDER_OFFSET - SNAKE_SPRITE_SIZE));
    data->food.y = GAME_AREA_TOP +
        (get_random() % (DISPLAY_HEIGHT - GAME_AREA_TOP - BORDER_OFFSET - SNAKE_SPRITE_SIZE));

    // Ensure food doesn't spawn on snake
    for (uint8_t i = 0; i < data->length; i++) {
        if (abs(data->food.x - data->body[i].x) < SNAKE_SPRITE_SIZE &&
            abs(data->food.y - data->body[i].y) < SNAKE_SPRITE_SIZE) {
            spawn_food(data);  // Try again
            return;
        }
    }
}

static bool check_collision(SnakeGameData* data) {
    for (uint8_t i = 1; i < data->length; i++) {
        uint8_t body_x = data->body[i].x;
        uint8_t body_y = data->body[i].y;
        wrap_coordinates(&body_x, &body_y);

        if (abs(data->head_x - body_x) < SNAKE_SPRITE_SIZE &&
            abs(data->head_y - body_y) < SNAKE_SPRITE_SIZE) {
            return true;
        }
    }
    return false;
}

static void handle_direction_change(SnakeGameData* data, JoystickStatus js_status) {
    if (!js_status.is_new) return;

    uint8_t new_direction = data->direction;
    switch (js_status.direction) {
    case JS_DIR_RIGHT:
        if (data->direction != JS_DIR_LEFT) new_direction = JS_DIR_RIGHT;
        break;
    case JS_DIR_LEFT:
        if (data->direction != JS_DIR_RIGHT) new_direction = JS_DIR_LEFT;
        break;
    case JS_DIR_UP:
        if (data->direction != JS_DIR_DOWN) new_direction = JS_DIR_UP;
        break;
    case JS_DIR_DOWN:
        if (data->direction != JS_DIR_UP) new_direction = JS_DIR_DOWN;
        break;
    }
    data->direction = new_direction;
}

static void move_snake(SnakeGameData* data) {
    uint8_t prev_x = data->head_x;
    uint8_t prev_y = data->head_y;

    switch (data->direction) {
    case JS_DIR_RIGHT: data->head_x += SNAKE_SPRITE_SIZE; break;
    case JS_DIR_LEFT:  data->head_x -= SNAKE_SPRITE_SIZE; break;
    case JS_DIR_UP:    data->head_y -= SNAKE_SPRITE_SIZE; break;
    case JS_DIR_DOWN:  data->head_y += SNAKE_SPRITE_SIZE; break;
    }

    // Wrap around screen borders
    wrap_coordinates(&data->head_x, &data->head_y);

    for (uint8_t i = 0; i < data->length; i++) {
        uint8_t temp_x = data->body[i].x;
        uint8_t temp_y = data->body[i].y;
        data->body[i].x = prev_x;
        data->body[i].y = prev_y;
        prev_x = temp_x;
        prev_y = temp_y;
    }
}


static void handle_food_collision(SnakeGameData* data) {
    uint8_t food_x = data->food.x;
    uint8_t food_y = data->food.y;
    wrap_coordinates(&food_x, &food_y);

    if (abs(data->head_x - food_x) < SNAKE_SPRITE_SIZE &&
        abs(data->head_y - food_y) < SNAKE_SPRITE_SIZE) {
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
                snake_init();
            }
        }
    }
}

static uint16_t calculate_snake_speed(uint32_t score) {
    uint16_t speed_reduction = (score / 20) * 20;  // Every 20 points, reduce by 20ms
    return (SNAKE_SPEED > speed_reduction) ? SNAKE_SPEED - speed_reduction : 200;  // Minimum time difference of 200ms -> maximum speed
}

static void snake_update(JoystickStatus js_status) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;
    uint32_t current_time = get_current_ms();

    handle_direction_change(data, js_status);

    uint16_t current_speed = calculate_snake_speed(snake_game_engine.base_state.score);
    if (current_time - last_move_time >= current_speed) {
        move_snake(data);
        handle_food_collision(data);
        handle_collision(data);
        last_move_time = current_time;
    }

    animated_sprite_update(&snake_head_animated);
}

static void snake_render(void) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;

    // Draw border
    display_draw_border_at(1, 12, 3, 3);

    // Draw score and lives
    char status_text[32];
    snprintf(status_text, sizeof(status_text), "Score: %lu Lives: %d",
        snake_game_engine.base_state.score,
        snake_game_engine.base_state.lives);
    display_set_cursor(2, 2);
    display_write_string(status_text, Font_7x10, DISPLAY_WHITE);

    // Draw snake head with rotation
    uint16_t rotation = 0;
    switch (data->direction) {
    case JS_DIR_RIGHT: rotation = 0;   break;
    case JS_DIR_DOWN:  rotation = 90;  break;
    case JS_DIR_LEFT:  rotation = 180; break;
    case JS_DIR_UP:    rotation = 270; break;
    }
    sprite_draw_rotated(&snake_head_animated.frames[snake_head_animated.current_frame],
        data->head_x, data->head_y, rotation, DISPLAY_WHITE);

    // Draw body segments
    for (uint8_t i = 0; i < data->length; i++) {
        sprite_draw(&snake_body_sprite, data->body[i].x, data->body[i].y, DISPLAY_WHITE);
    }

    // Draw food
    sprite_draw(&food_sprite, data->food.x, data->food.y, DISPLAY_WHITE);

    // Draw game over text if needed
    if (snake_game_engine.base_state.game_over) {
        display_write_string_centered("GAME OVER", Font_7x10, 30, DISPLAY_WHITE);
    }
}

static void snake_cleanup(void) {
    // Reset game state
    snake_data.head_x = 0;
    snake_data.head_y = 0;
    snake_data.direction = JS_DIR_RIGHT;
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
}
