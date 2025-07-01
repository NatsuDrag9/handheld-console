/*
 * game_engine_network.c
 *
 *  Created on: Jul 1, 2025
 *      Author: rohitimandi
 *
 * Network error handling for game engine - handles connection loss and exit logic
 */

#include "Game_Engine/game_engine_network.h"
#include "Utils/debug_conf.h"
#include "Utils/misc_utils.h"

// Network error state
static bool network_error_displayed = false;
static uint32_t network_error_start_time = 0;
static uint32_t last_network_error_check = 0;

// Configuration constants
static const uint32_t NETWORK_ERROR_CHECK_INTERVAL_MS = 1000; // Check every second
static const uint32_t NETWORK_ERROR_EXIT_DELAY_MS = 3000; // Exit after 3 seconds

// Private function prototypes
static void clear_network_error_display(void);

// Initialize network error handling
void game_engine_network_init(void) {
    network_error_displayed = false;
    network_error_start_time = 0;
    last_network_error_check = 0;

    DEBUG_PRINTF(false, "Game Engine Network: Initialized\r\n");
}

// Cleanup network error handling
void game_engine_network_cleanup(void) {
    network_error_displayed = false;
    network_error_start_time = 0;
    last_network_error_check = 0;

    DEBUG_PRINTF(false, "Game Engine Network: Cleanup completed\r\n");
}

// Check for network errors and handle exit logic
void game_engine_network_check_errors(GameEngine* engine) {
    if (!engine) return;

    uint32_t current_time = get_current_ms();

    // Don't check too frequently
    if (current_time - last_network_error_check < NETWORK_ERROR_CHECK_INTERVAL_MS) {
        return;
    }
    last_network_error_check = current_time;

    // Check if serial_comm has detected a network error
    if (serial_comm_has_network_error()) {
        if (!network_error_displayed) {
            // Start error display timer
            network_error_displayed = true;
            network_error_start_time = current_time;
            DEBUG_PRINTF(false, "Game Engine Network: Error detected - starting exit timer\r\n");
        } else {
            // Check if we should exit the game
            uint32_t error_duration = current_time - network_error_start_time;
            if (error_duration >= NETWORK_ERROR_EXIT_DELAY_MS) {
                DEBUG_PRINTF(false, "Game Engine Network: Error timeout - exiting game\r\n");
                engine->return_to_main_menu = true;
            }
        }
    } else {
        // Network error cleared
        if (network_error_displayed) {
            clear_network_error_display();
        }
    }
}

// Render network error messages
void game_engine_network_render_error(void) {
    if (!network_error_displayed) return;

    uint32_t current_time = get_current_ms();
    uint32_t error_duration = current_time - network_error_start_time;

    if (error_duration < NETWORK_ERROR_EXIT_DELAY_MS) {
        // Show connection lost message for first 2 seconds
    	const char* error_message = serial_comm_get_error_message();
        if (error_duration < 2000) {
        	if(error_message) {
        		display_manager_show_centered_message(error_message, DISPLAY_HEIGHT/2 - 20);
        	}
        } else {
            // Show exit countdown for remaining time
            uint32_t remaining_ms = NETWORK_ERROR_EXIT_DELAY_MS - error_duration;
            uint32_t remaining_seconds = (remaining_ms / 1000) + 1;

            char exit_msg[64];
            snprintf(exit_msg, sizeof(exit_msg), "Returning to menu in %lu...", remaining_seconds);
            if(error_message) {
            	display_manager_show_centered_message(error_message, DISPLAY_HEIGHT/2 - 20);
            }
            display_manager_show_centered_message(exit_msg, DISPLAY_HEIGHT/2);
        }
    }
}

// Query network error state
bool game_engine_network_has_error(void) {
    return network_error_displayed;
}


// Private helper functions
static void clear_network_error_display(void) {
    network_error_displayed = false;
    network_error_start_time = 0;
    DEBUG_PRINTF(false, "Game Engine Network: Error cleared\r\n");
}
