/*
 * game_engine.h
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAME_ENGINE_H_
#define INC_GAME_ENGINE_GAME_ENGINE_H_

#include "Console_Peripherals/Hardware/Drivers/display_driver.h"
#include "Console_Peripherals/types.h"
#include <stdbool.h>
#include <stdio.h>
#include "Utils/misc_utils.h"
#include "game_engine_conf.h"

typedef void (*UpdateWithJoystick)(JoystickStatus);
typedef void (*UpdateWithDPad)(DPAD_STATUS);

// Single player state structure
typedef struct {
    uint32_t score;
    uint8_t lives;
} SinglePlayerState;

// Multiplayer state structure
typedef struct {
    uint32_t p1_score;
    uint32_t p2_score;
    uint32_t target_score;
    uint8_t lives;
} MultiPlayerState;

typedef struct {
    void (*init)(void);              // Initialize game state
    void (*render)(void);            // Draw game state
    void (*cleanup)(void);           // Cleanup resources
    void (*show_game_over_message)(void); // Displays a custom game over message

    // Union for different update function signatures
    union {
        UpdateWithJoystick update_joystick;
        UpdateWithDPad update_dpad;
    } update_func;  // Game logic update

    struct {
        // Common state fields that all games need
        bool paused;
        bool game_over;
        bool is_reset;

        // Union for different state types
        union {
            SinglePlayerState single;
            MultiPlayerState multi;
        } state_data;
    } base_state;

    void* game_data;  // Game-specific data
    bool countdown_over;
    bool return_to_main_menu;
    bool is_d_pad_game;  // Flag to determine input type
    bool is_mp_game;     // Flag to determine state type

} GameEngine;

// Game Engine core functions
void game_engine_init(GameEngine* engine);
//void game_engine_update(GameEngine* engine, JoystickStatus js_status);
void game_engine_update(GameEngine* engine, void* input_data);
void game_engine_render(GameEngine* engine);
void game_engine_cleanup(GameEngine* engine);
void game_engine_render_countdown(GameEngine* engine);

#endif /* INC_GAME_ENGINE_GAME_ENGINE_H_ */
