/*
 * game_engine.h
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAME_ENGINE_H_
#define INC_GAME_ENGINE_GAME_ENGINE_H_

#include <Console_Peripherals/Hardware/Drivers/display_driver.h>
#include <Console_Peripherals/types.h>
#include <stdbool.h>
#include <stdio.h>
#include "Utils/misc_utils.h"
#include "game_engine_conf.h"

typedef void (*UpdateWithJoystick)(JoystickStatus);
typedef void (*UpdateWithDPad)(DPAD_STATUS);

typedef struct {
    void (*init)(void);              // Initialize game state
//    void (*update)(JoystickStatus);  // Game logic update
    void (*render)(void);            // Draw game state
    void (*cleanup)(void);           // Cleanup resources

    // Union for different update function signatures
       union {
           UpdateWithJoystick update_joystick;
           UpdateWithDPad update_dpad;
       } update_func;  // Game logic update

    struct {
        uint32_t score;
        uint8_t lives;
        bool paused;
        bool game_over;
        bool is_reset;
    } base_state;

    void* game_data;  // Game-specific data
    bool countdown_over;
    bool return_to_main_menu;
    bool is_d_pad_game;  // Flag to determine input type

} GameEngine;

// Game Engine core functions
void game_engine_init(GameEngine* engine);
//void game_engine_update(GameEngine* engine, JoystickStatus js_status);
void game_engine_update(GameEngine* engine, void* input_data);
void game_engine_render(GameEngine* engine);
void game_engine_cleanup(GameEngine* engine);
void game_engine_render_countdown(GameEngine* engine);

#endif /* INC_GAME_ENGINE_GAME_ENGINE_H_ */
