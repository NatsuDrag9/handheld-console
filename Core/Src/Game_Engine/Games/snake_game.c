/*
 * snake_game.c
 *
 *  Created on: Jan 15, 2025
 *      Author: rohitimandi
 */

#include "Game_Engine/Games/snake_game.h"
#include "Utils/debug_conf.h"

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
}

static void snake_update(JoystickStatus js_status) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;

    // Update snake head animation
    animated_sprite_update(&snake_head_animated);

    // Handle direction changes based on joystick
    if (js_status.is_new) {
        switch (js_status.direction) {
            case JS_DIR_RIGHT:
                if (data->direction != JS_DIR_LEFT) {
                    data->direction = JS_DIR_RIGHT;
                }
                break;

            case JS_DIR_LEFT:
                if (data->direction != JS_DIR_RIGHT) {
                    data->direction = JS_DIR_LEFT;
                }
                break;

            case JS_DIR_UP:
                if (data->direction != JS_DIR_DOWN) {
                    data->direction = JS_DIR_UP;
                }
                break;

            case JS_DIR_DOWN:
                if (data->direction != JS_DIR_UP) {
                    data->direction = JS_DIR_DOWN;
                }
                break;

            // Handle diagonal movements if needed
            case JS_DIR_LEFT_UP:
                if (data->direction != JS_DIR_DOWN && data->direction != JS_DIR_RIGHT) {
                    data->direction = JS_DIR_UP;  // Prioritize vertical movement
                }
                break;

            case JS_DIR_LEFT_DOWN:
                if (data->direction != JS_DIR_UP && data->direction != JS_DIR_RIGHT) {
                    data->direction = JS_DIR_DOWN;
                }
                break;

            case JS_DIR_RIGHT_UP:
                if (data->direction != JS_DIR_DOWN && data->direction != JS_DIR_LEFT) {
                    data->direction = JS_DIR_UP;
                }
                break;

            case JS_DIR_RIGHT_DOWN:
                if (data->direction != JS_DIR_UP && data->direction != JS_DIR_LEFT) {
                    data->direction = JS_DIR_DOWN;
                }
                break;

            default:
                break;
        }
    }

    // TODO: Implement snake movement
    // TODO: Implement collision detection
    // TODO: Implement food collection
    // TODO: Implement game over conditions
}

static void snake_render(void) {
    SnakeGameData* data = (SnakeGameData*)snake_game_engine.game_data;

    // Calculate rotation based on direction
    uint8_t rotation;
    switch(data->direction) {
        case JS_DIR_RIGHT:
            rotation = 0;
            break;
        case JS_DIR_DOWN:
            rotation = 90;
            break;
        case JS_DIR_LEFT:
            rotation = 180;
            break;
        case JS_DIR_UP:
            rotation = (uint8_t)270;
            break;
        default:
            rotation = 0;
    }

    // Draw game border
    display_draw_border();

    // Draw snake head with appropriate rotation
    sprite_draw_rotated(&snake_head_animated.frames[snake_head_animated.current_frame],
                       data->head_x, data->head_y, rotation, DISPLAY_WHITE);

    // Draw body segments
    for (uint8_t i = 0; i < data->length; i++) {
        sprite_draw(&snake_body_sprite, data->body[i].x, data->body[i].y, DISPLAY_WHITE);
    }

    // Draw score on top
    char score_text[16];
    snprintf(score_text, sizeof(score_text), "Score: %lu", snake_game_engine.base_state.score);
    display_set_cursor(2, 2);
    display_write_string(score_text, Font_7x10, DISPLAY_WHITE);
}

static void snake_cleanup(void) {
    // Nothing to clean up for now
}
