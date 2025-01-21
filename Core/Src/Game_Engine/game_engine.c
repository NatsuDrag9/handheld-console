/*
 * game_engine.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/oled.h"
#include "Game_Engine/game_engine.h"

// Common game engine functions
void game_engine_init(GameEngine* engine) {
    if (engine && engine->init) {
        // Initialize base state
        engine->base_state.score = 0;
        engine->base_state.lives = 3;
        engine->base_state.paused = false;
        engine->base_state.game_over = false;

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

void game_engine_render(GameEngine* engine) {
    if (engine && engine->render) {
        // Clear screen before rendering
        display_clear();

        // Call game-specific render function
        engine->render();

        // Update display
        display_update();
    }
}

void game_engine_cleanup(GameEngine* engine) {
    if (engine && engine->cleanup) {
        engine->cleanup();
    }
}
