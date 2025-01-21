/*
 * game_engine.h
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAME_ENGINE_H_
#define INC_GAME_ENGINE_GAME_ENGINE_H_

#include <stdbool.h>
#include <stdio.h>
#include "Console_Peripherals/types.h"
#include "Utils/misc_utils.h"
#include "game_engine_conf.h"

typedef struct {
    void (*init)(void);              // Initialize game state
    void (*update)(JoystickStatus);  // Game logic update
    void (*render)(void);            // Draw game state
    void (*cleanup)(void);           // Cleanup resources

    struct {
        uint32_t score;
        uint8_t lives;
        bool paused;		// To be implemented
        bool game_over;
    } base_state;

    void* game_data;  // Game-specific data
} GameEngine;

// Game Engine core functions
void game_engine_init(GameEngine* engine);
void game_engine_update(GameEngine* engine, JoystickStatus js_status);
void game_engine_render(GameEngine* engine);
void game_engine_cleanup(GameEngine* engine);

#endif /* INC_GAME_ENGINE_GAME_ENGINE_H_ */
