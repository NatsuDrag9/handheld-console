/*
 * mp_snake_game_render.c
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *  Rendering and UI functions for multiplayer snake
 */

#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_render.h"
#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_core.h"
#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_network.h"
#include "Game_Engine/Games/Helpers/snake_game_helpers.h"
#include "Console_Peripherals/Hardware/display_manager.h"
#include "Utils/debug_conf.h"
#include <string.h>
#include <stdio.h>

// Rendering optimization tracking
coord_t previous_local_head_x = 0, previous_local_head_y = 0;
coord_t previous_opponent_head_x = 0, previous_opponent_head_y = 0;
coord_t previous_food_x = 0, previous_food_y = 0;
uint8_t previous_local_length = 0;
uint8_t previous_opponent_length = 0;
uint32_t previous_local_score = 0;
uint32_t previous_opponent_score = 0;
bool first_render = true;

// Main rendering function
void mp_snake_render_game(void) {
    switch (mp_snake_data.phase) {
        case MP_PHASE_WAITING:
            mp_snake_render_waiting_screen();
            break;
        case MP_PHASE_PLAYING:
            mp_snake_render_game_area();
            mp_snake_render_multiplayer_ui();
            break;
        case MP_PHASE_ENDED:
            mp_snake_render_game_area();
            mp_snake_render_game_over_screen();
            break;
    }
}

// Rendering initialization and cleanup
void mp_snake_render_init(void) {
    first_render = true;
    mp_snake_reset_render_tracking();
    DEBUG_PRINTF(false, "Render: Multiplayer snake rendering initialized\r\n");
}

void mp_snake_render_cleanup(void) {
    first_render = true;
    DEBUG_PRINTF(false, "Render: Multiplayer snake rendering cleanup completed\r\n");
}

void mp_snake_reset_render_tracking(void) {
    previous_local_head_x = mp_snake_data.predicted_local_player.head_x;
    previous_local_head_y = mp_snake_data.predicted_local_player.head_y;
    previous_opponent_head_x = mp_snake_data.display_opponent.head_x;
    previous_opponent_head_y = mp_snake_data.display_opponent.head_y;
    previous_food_x = mp_snake_data.server_food.x;
    previous_food_y = mp_snake_data.server_food.y;
    previous_local_length = mp_snake_data.predicted_local_player.length;
    previous_opponent_length = mp_snake_data.display_opponent.length;
    previous_local_score = 0;
    previous_opponent_score = 0;
}

// Phase-specific rendering functions
void mp_snake_render_waiting_screen(void) {
    if (first_render) {
        display_clear();
        first_render = false;
    }

    // Show connection status
    ProtocolState state = mp_snake_get_connection_state();
    const char* status_text = "";

    switch (state) {
        case PROTO_STATE_INIT:
            status_text = "Initializing...";
            break;
        case PROTO_STATE_ESP32_READY:
            status_text = "ESP32 Ready";
            break;
        case PROTO_STATE_WIFI_CONNECTING:
            status_text = "Connecting to WiFi...";
            break;
        case PROTO_STATE_WIFI_CONNECTED:
            status_text = "WiFi Connected";
            break;
        case PROTO_STATE_WEBSOCKET_CONNECTING:
            status_text = "Connecting to Server...";
            break;
        case PROTO_STATE_WEBSOCKET_CONNECTED:
            status_text = "Waiting for opponent...";
            break;
        case PROTO_STATE_GAME_READY:
                status_text = "Game ready...";
                break;
        case PROTO_STATE_GAME_ACTIVE:
            status_text = "Starting game...";
            break;
        case PROTO_STATE_ERROR:
            status_text = "Connection Error";
            break;
    }

    display_manager_show_centered_message((char*)status_text, DISPLAY_HEIGHT/2 - 10);

    char player_text[32];
    snprintf(player_text, sizeof(player_text), "You are Player %d", mp_snake_data.local_player_id);
    display_manager_show_centered_message(player_text, DISPLAY_HEIGHT/2 + 5);

    // Show debug info if in error state
    if (state == PROTO_STATE_ERROR) {
    	display_manager_show_centered_message("Check ESP32 connection", DISPLAY_HEIGHT/2 + 20);
    }
}

void mp_snake_render_game_area(void) {
    if (first_render) {
        display_draw_border_at(1, STATUS_START_Y, 3, 3);
        first_render = false;
    }

    // Clear previous positions
    mp_snake_clear_previous_positions();

    // Draw local player (predicted state for responsiveness)
    // Use different colors to distinguish players
    if (mp_snake_data.players_alive[mp_snake_data.local_player_id - 1]) {
        // TODO: Implement color differentiation when available
        snake_helper_draw_snake(&mp_snake_data.predicted_local_player);
    }

    // Draw opponent (server state)
    MultiplayerPlayerId opponent_id = mp_snake_get_opponent_id();
    if (mp_snake_data.players_alive[opponent_id - 1]) {
        // TODO: Draw opponent in different color
        snake_helper_draw_snake(&mp_snake_data.display_opponent);
    }

    // Draw shared food
    snake_helper_draw_food(&mp_snake_data.server_food);

    // Periodically redraw border to ensure it's intact
    if (get_current_ms() % 1000 == 0) {
        display_draw_border_at(1, STATUS_START_Y, 3, 3);
    }
}

void mp_snake_render_multiplayer_ui(void) {
    // Clear status area
    display_fill_rectangle(2, 2, DISPLAY_WIDTH - 2, STATUS_START_Y - 1, DISPLAY_BLACK);

    // Draw scores for both players
    char score_text[64];
    snprintf(score_text, sizeof(score_text), "P1:%lu P2:%lu Target:%lu",
             mp_snake_data.server_scores[0],
             mp_snake_data.server_scores[1],
             mp_snake_data.target_score);

    display_set_cursor(2, 2);
    display_write_string(score_text, Font_7x10, DISPLAY_WHITE);

    // Draw connection status on second line
    char status_text[64] = "";
    if (!mp_snake_data.opponent_connected) {
        strcpy(status_text, "Opponent Disconnected");
    } else if (!mp_snake_is_websocket_connected()) {
        strcpy(status_text, "Connection Lost");
    } else {
        // Show which player is you
        if (mp_snake_data.local_player_id == MP_PLAYER_1) {
            strcpy(status_text, "You: P1 (Green)");
        } else {
            strcpy(status_text, "You: P2 (Blue)");
        }
    }

    display_set_cursor(2, 12);
    display_write_string(status_text, Font_7x10, DISPLAY_WHITE);
}

void mp_snake_render_game_over_screen(void) {
    char result_text[32];
    char details_text[32] = "";

    switch (mp_snake_data.winner) {
        case MP_RESULT_PLAYER1_WINS:
            if (mp_snake_data.local_player_id == MP_PLAYER_1) {
                strcpy(result_text, "YOU WIN!");
            } else {
                strcpy(result_text, "PLAYER 1 WINS");
            }
            snprintf(details_text, sizeof(details_text), "Score: %lu", mp_snake_data.server_scores[0]);
            break;
        case MP_RESULT_PLAYER2_WINS:
            if (mp_snake_data.local_player_id == MP_PLAYER_2) {
                strcpy(result_text, "YOU WIN!");
            } else {
                strcpy(result_text, "PLAYER 2 WINS");
            }
            snprintf(details_text, sizeof(details_text), "Score: %lu", mp_snake_data.server_scores[1]);
            break;
        case MP_RESULT_DRAW:
            strcpy(result_text, "DRAW!");
            snprintf(details_text, sizeof(details_text), "Both: %lu", mp_snake_data.server_scores[0]);
            break;
        default:
            strcpy(result_text, "GAME OVER");
            break;
    }

    // Draw semi-transparent overlay
    display_fill_rectangle(10, DISPLAY_HEIGHT/2 - 20, DISPLAY_WIDTH - 10, DISPLAY_HEIGHT/2 + 20, DISPLAY_BLACK);

    display_manager_show_centered_message(result_text, DISPLAY_HEIGHT/2 - 10);
       if (strlen(details_text) > 0) {
       	display_manager_show_centered_message(details_text, DISPLAY_HEIGHT/2 + 15);
       }
}

// Optimization functions
void mp_snake_clear_previous_positions(void) {
    // Optimized dirty rectangle clearing for multiplayer
    // Similar to single-player but handles two snakes

    // Clear previous local player head if it moved
    if (previous_local_head_x != mp_snake_data.predicted_local_player.head_x ||
        previous_local_head_y != mp_snake_data.predicted_local_player.head_y) {

        // Check if previous position is not occupied by any snake
        bool is_occupied = false;

        // Check local player body
        for (uint8_t i = 0; i < mp_snake_data.predicted_local_player.length; i++) {
            if (previous_local_head_x == mp_snake_data.predicted_local_player.body[i].x &&
                previous_local_head_y == mp_snake_data.predicted_local_player.body[i].y) {
                is_occupied = true;
                break;
            }
        }

        // Check opponent
        if (!is_occupied) {
            for (uint8_t i = 0; i < mp_snake_data.display_opponent.length; i++) {
                if (previous_local_head_x == mp_snake_data.display_opponent.body[i].x &&
                    previous_local_head_y == mp_snake_data.display_opponent.body[i].y) {
                    is_occupied = true;
                    break;
                }
            }
        }

        if (!is_occupied) {
            display_clear_region(previous_local_head_x, previous_local_head_y, SPRITE_SIZE, SPRITE_SIZE);
        }
    }

    // Similar logic for opponent head
    if (previous_opponent_head_x != mp_snake_data.display_opponent.head_x ||
        previous_opponent_head_y != mp_snake_data.display_opponent.head_y) {

        // Check if previous position is not occupied by any snake
        bool is_occupied = false;

        // Check both snakes
        for (uint8_t i = 0; i < mp_snake_data.predicted_local_player.length; i++) {
            if (previous_opponent_head_x == mp_snake_data.predicted_local_player.body[i].x &&
                previous_opponent_head_y == mp_snake_data.predicted_local_player.body[i].y) {
                is_occupied = true;
                break;
            }
        }

        if (!is_occupied) {
            for (uint8_t i = 0; i < mp_snake_data.display_opponent.length; i++) {
                if (previous_opponent_head_x == mp_snake_data.display_opponent.body[i].x &&
                    previous_opponent_head_y == mp_snake_data.display_opponent.body[i].y) {
                    is_occupied = true;
                    break;
                }
            }
        }

        if (!is_occupied) {
            display_clear_region(previous_opponent_head_x, previous_opponent_head_y, SPRITE_SIZE, SPRITE_SIZE);
        }
    }

    // Clear previous food position if changed
    if (previous_food_x != mp_snake_data.server_food.x || previous_food_y != mp_snake_data.server_food.y) {
        if (previous_food_x != 0 && previous_food_y != 0) {
            display_clear_region(previous_food_x, previous_food_y, SPRITE_SIZE, SPRITE_SIZE);
        }
    }

    // Update previous positions for next frame
    previous_local_head_x = mp_snake_data.predicted_local_player.head_x;
    previous_local_head_y = mp_snake_data.predicted_local_player.head_y;
    previous_opponent_head_x = mp_snake_data.display_opponent.head_x;
    previous_opponent_head_y = mp_snake_data.display_opponent.head_y;
    previous_food_x = mp_snake_data.server_food.x;
    previous_food_y = mp_snake_data.server_food.y;
    previous_local_length = mp_snake_data.predicted_local_player.length;
    previous_opponent_length = mp_snake_data.display_opponent.length;
    previous_local_score = mp_snake_data.server_scores[mp_snake_data.local_player_id - 1];
    previous_opponent_score = mp_snake_data.server_scores[mp_snake_get_opponent_id() - 1];
}
