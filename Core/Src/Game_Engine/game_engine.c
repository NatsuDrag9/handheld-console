/*
 * game_engine.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/oled.h"
#include "Game_Engine/game_engine.h"

static uint32_t game_over_start_time = 0;

// Common game engine functions
void game_engine_init(GameEngine* engine) {
    if (engine && engine->init) {
        // Initialize base state
        engine->base_state.score = 0;
        engine->base_state.lives = 3;
        engine->base_state.paused = false;
        engine->base_state.game_over = false;
        engine->countdown_over = false;

        // Reset game over timer
        game_over_start_time = 0;

        // Call game-specific initialization
        engine->init();
    }
}

void game_engine_update(GameEngine* engine, JoystickStatus js_status) {
    if (engine && engine->update && !engine->base_state.game_over) {
        if (!engine->base_state.paused) {
            engine->update(js_status);
        }
    }
}

void game_engine_render_countdown(GameEngine* engine) {
    if (engine && engine->base_state.game_over) {
        uint32_t current_time = get_current_ms();

        // Initialize timer if not started
        if (game_over_start_time == 0) {
            game_over_start_time = current_time;
        }

        uint32_t elapsed_time = current_time - game_over_start_time;

        // Only show countdown message between 5-10 seconds
        if (elapsed_time >= RETURN_MESSAGE_START_TIME
            && elapsed_time < GAME_OVER_MESSAGE_TIME) {
            uint32_t remaining_secs = (GAME_OVER_MESSAGE_TIME - elapsed_time) / 1000;
            char countdown_msg[32];
            snprintf(countdown_msg, sizeof(countdown_msg),
                "Returning to main\n menu in: %lu", remaining_secs + 1);
            display_write_string_centered(countdown_msg, Font_7x10, 45, DISPLAY_WHITE);
            DEBUG_PRINTF(false, "Remaining sec: %lu\n", remaining_secs);
        }

        if (elapsed_time >= GAME_OVER_MESSAGE_TIME) {
            engine->countdown_over = true;  // Set true when countdown is complete
            return;
        }
    }
}

void game_engine_render(GameEngine* engine) {
    if (engine && engine->render) {
        // Clear screen before rendering
        display_clear();

        // Call game-specific render function
        engine->render();

        // Render countdown if in game over state
        if (engine->base_state.game_over) {
            game_engine_render_countdown(engine);
        }

        // Update display
        display_update();
    }
}

void game_engine_cleanup(GameEngine* engine) {
    if (engine && engine->cleanup) {
        engine->cleanup();
        engine->countdown_over = false;
        game_over_start_time = 0;  // Reset timer on cleanup
    }
}
