/*
 * game_engine.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/oled.h"
#include "Game_Engine/game_engine.h"
#include "Console_Peripherals/push_button.h"

static uint32_t game_over_start_time = 0;
static uint32_t button2_press_start_time = 0;
static bool button2_being_held = false;

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
	}
}

void game_engine_handle_buttons(GameEngine* engine) {
	uint8_t pb1_state = pb1_get_state();
	uint32_t current_time = get_current_ms();

	// Handle Button 1 (Play/Pause) - toggle on press
	if (pb1_state && !engine->base_state.game_over) {
		engine->base_state.paused = !engine->base_state.paused;

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
				}
			}
			else if (hold_duration >= BUTTON_MENU_MIN_DURATION) {  // Long press 3 seconds

				engine->return_to_main_menu = true;
			}

			// Reset button tracking
			button2_being_held = false;
		}
	}
}

void game_engine_update(GameEngine* engine, JoystickStatus js_status) {
	if (engine) {
		// Always handle button input regardless of game state
		game_engine_handle_buttons(engine);

		// Only update game logic if not paused and not game over
		if (engine->update && !engine->base_state.game_over && !engine->base_state.paused) {
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

		// Render paused message if game is paused
		if (engine->base_state.paused) {
			display_write_string_centered("PAUSED", Font_7x10, 30, DISPLAY_WHITE);
		}

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
		button2_press_start_time = 0;
		button2_being_held = false;
	}
}
