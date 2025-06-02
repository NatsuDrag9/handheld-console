/*
 * game_engine.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 *  Modified on: Apr 13, 2025
 *      Added dirty rectangle optimization
 */

#include <Console_Peripherals/Hardware/push_button.h>
#include "Console_Peripherals/oled.h"
#include "Game_Engine/game_engine.h"

static uint32_t game_over_start_time = 0;
static uint32_t button2_press_start_time = 0;
static bool button2_being_held = false;

// Track if a full screen refresh is needed
static bool require_full_refresh = true;

// Common game engine functions
void game_engine_init(GameEngine* engine) {
    if (engine && engine->init) {
        // Initialize base state
        engine->base_state.score = 0;
        engine->base_state.lives = 3;
        engine->base_state.paused = false;
        engine->base_state.is_reset = false;
        engine->base_state.game_over = false;
        engine->countdown_over = false;
        engine->return_to_main_menu = false;

        // Reset game over timer
        game_over_start_time = 0;

        // Reset button state tracking
        button2_press_start_time = 0;
        button2_being_held = false;

        // Call game-specific initialization
        engine->init();

        // Force a full screen refresh on first render after initialization
        require_full_refresh = true;
    }
}

void game_engine_handle_buttons(GameEngine* engine) {
    uint8_t pb1_state = pb1_get_state();
    uint32_t current_time = get_current_ms();

    // Handle Button 1 (Play/Pause) - toggle on press
    if (pb1_state && !engine->base_state.game_over) {
        engine->base_state.paused = !engine->base_state.paused;
        // Force full refresh when game state changes
        require_full_refresh = true;
    }

    // Handle Button 2 (Restart/Main Menu)
    // Using raw pin value instead of debounced state (pb2_get_state()) as we need
    // to track how long the button is held down, which requires continuous monitoring
    // of the raw pin state.
    if (PB_Driver_ReadPin2()) {
        if (!button2_being_held) {
            // Start tracking button press duration
            button2_being_held = true;
            button2_press_start_time = current_time;
        }
        else {
            // Calculate how long button has been held
            //            uint32_t hold_duration = current_time - button2_press_start_time;

            // For UI feedback, could add code here to show hold progress
            // (e.g., drawing a progress bar when holding for main menu)
        }
    }
    else {
        // Button was released or is not pressed
        if (button2_being_held) {
            // Button was released after being held
            uint32_t hold_duration = current_time - button2_press_start_time;

            if (hold_duration <= BUTTON_RESTART_MAX_DURATION) {  // Short press ≤ 1.2 seconds
                if (!engine->base_state.game_over) {
                    engine->base_state.is_reset = true;
                    // Force full refresh on reset
                    require_full_refresh = true;
                }
            }
            else if (hold_duration >= BUTTON_MENU_MIN_DURATION) {  // Long press 3 seconds
                engine->return_to_main_menu = true;
                // Force full refresh when returning to menu
                require_full_refresh = true;
            }

            // Reset button tracking
            button2_being_held = false;
        }
    }
}

void game_engine_update(GameEngine* engine, void* input_data) {
    if (engine) {
        // Always handle button input regardless of game state
        game_engine_handle_buttons(engine);

        // Only update game logic if not paused and not game over
        if (!engine->base_state.game_over && !engine->base_state.paused) {
            if (engine->is_d_pad_game) {
                // Cast input to DPAD_STATUS for D-pad games
                DPAD_STATUS* dpad_status = (DPAD_STATUS*)input_data;
                // Check if the function pointer is valid before calling
                if (engine->update_func.update_dpad != NULL) {
                    engine->update_func.update_dpad(*dpad_status);
                }
            }
            else {
                // Cast input to JoystickStatus for joystick games
                JoystickStatus* js_status = (JoystickStatus*)input_data;
                // Check if the function pointer is valid before calling
                if (engine->update_func.update_joystick != NULL) {
                    engine->update_func.update_joystick(*js_status);
                }
            }
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
        static uint32_t last_second = 0;
        uint32_t current_second = elapsed_time / 1000;

        // Only update the countdown text when the second changes
        // This avoids redrawing the same text every frame
        if (current_second != last_second) {
            // Only show countdown message between 5-10 seconds
            if (elapsed_time >= RETURN_MESSAGE_START_TIME
                && elapsed_time < GAME_OVER_MESSAGE_TIME) {
                uint32_t remaining_secs = (GAME_OVER_MESSAGE_TIME - elapsed_time) / 1000;
                char countdown_msg[32];
                snprintf(countdown_msg, sizeof(countdown_msg),
                    "Returning to main\n menu in: %lu", remaining_secs + 1);

#ifdef DISPLAY_MODULE_LCD
                display_write_string_centered(countdown_msg, Font_11x18, 45, DISPLAY_WHITE);
#else
                display_write_string_centered(countdown_msg, Font_7x10, 45, DISPLAY_WHITE);
#endif

            }

            last_second = current_second;
        }

        if (elapsed_time >= GAME_OVER_MESSAGE_TIME) {
            engine->countdown_over = true;  // Set true when countdown is complete
            return;
        }
    }
}

void game_engine_render(GameEngine* engine) {
    if (engine && engine->render) {
        // Check for state changes that require full screen refresh
        static bool was_paused = false;
        static bool was_game_over = false;

        bool state_changed = (was_paused != engine->base_state.paused) ||
                            (was_game_over != engine->base_state.game_over);

        // Clear screen only when needed
        if (state_changed || require_full_refresh) {
            display_clear();
            require_full_refresh = false;
        }

        // Call game-specific render function - this will update dirty regions
        engine->render();

        // Render paused message if game is paused
        if (engine->base_state.paused) {
#ifdef DISPLAY_MODULE_LCD
        	display_write_string_centered("PAUSED", Font_11x18, 30, DISPLAY_WHITE);
#else
            display_write_string_centered("PAUSED", Font_7x10, 30, DISPLAY_WHITE);
#endif
        }

        // Render countdown if in game over state
        if (engine->base_state.game_over) {
            game_engine_render_countdown(engine);
        }

        // Update state tracking
        was_paused = engine->base_state.paused;
        was_game_over = engine->base_state.game_over;

        // Send updates to display
        display_update();
    }
}

void game_engine_cleanup(GameEngine* engine) {
    if (engine && engine->cleanup) {
        engine->cleanup();
        engine->countdown_over = false;
        game_over_start_time = 0;  // Reset timer on cleanup
        button2_press_start_time = 0;
        button2_being_held = false;

        // Force full refresh after cleanup
        require_full_refresh = true;
    }
}
