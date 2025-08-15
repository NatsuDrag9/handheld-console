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

 // Rendering optimization tracking (similar to TS private properties)
static coord_t previous_local_head_x = 0, previous_local_head_y = 0;
static coord_t previous_opponent_head_x = 0, previous_opponent_head_y = 0;
static coord_t previous_food_x = 0, previous_food_y = 0;
static uint8_t previous_local_length = 0;
static uint8_t previous_opponent_length = 0;
static uint32_t previous_local_score = 0;
static uint32_t previous_opponent_score = 0;
static bool first_render = true;

// Private function prototypes (similar to TS private methods)
static void render_waiting_screen(ProtocolState connection_status, uint8_t local_player_id);
static void render_gameplay(SnakeState* players, uint8_t player_count, Position* food,
    GameStats* game_stats, uint8_t local_player_id);
static void render_game_area(SnakeState* players, uint8_t player_count, Position* food,
    GameStats* game_stats, uint8_t local_player_id);
// NOTE: With GameStats to render scores from here
// static void render_multiplayer_ui(GameStats* game_stats, uint8_t local_player_id);

// NOTE: Without GameStats to render scores from display_manager
static void render_multiplayer_ui(uint8_t local_player_id);
static void clear_previous_positions(SnakeState* players, uint8_t player_count, Position* food);
static void reset_render_tracking(SnakeState* players, uint8_t player_count, Position* food);

// Rendering initialization and cleanup (similar to TS constructor/destructor)
void mp_snake_render_init(void) {
    first_render = true;
    DEBUG_PRINTF(false, "Render: Multiplayer snake rendering initialized\r\n");
}

void mp_snake_render_cleanup(void) {
    first_render = true;
    DEBUG_PRINTF(false, "Render: Multiplayer snake rendering cleanup completed\r\n");
}

// Main rendering function
void mp_snake_render_game(
    MultiplayerGamePhase game_phase,
    SnakeState* players,
    uint8_t player_count,
    Position* food,
    GameStats* game_stats,
    ProtocolState connection_status,
    uint8_t local_player_id,
    bool is_spectator
) {
    // Route to appropriate phase renderer (like TS switch statement)
    switch (game_phase) {
    case MP_PHASE_WAITING:
        render_waiting_screen(connection_status, local_player_id);
        break;
    case MP_PHASE_PLAYING:
        render_gameplay(players, player_count, food, game_stats, local_player_id);
        break;
    case MP_PHASE_ENDED:
        render_gameplay(players, player_count, food, game_stats, local_player_id);
        break;
    }
}

// Phase-specific rendering functions (similar to TS screen rendering methods)
static void render_waiting_screen(ProtocolState connection_status, uint8_t local_player_id) {
	if (first_render || serial_comm_needs_ui_update()) {
		display_manager_clear_main_area();
		if (serial_comm_needs_ui_update()) {
			serial_comm_clear_ui_update_flag();
		}
		first_render = false;
	}


    // Show connection status (similar to TS renderWaitingScreen)
    const char* status_text = "";

    switch (connection_status) {
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
    default:
        status_text = "Loading...";
        break;
    }

    display_manager_show_centered_message((char*)status_text, DISPLAY_HEIGHT / 2 - 10);

    if((connection_status == PROTO_STATE_WEBSOCKET_CONNECTED) || (connection_status == PROTO_STATE_GAME_READY) || (connection_status == PROTO_STATE_GAME_ACTIVE)) {
        char player_text[32];
        snprintf(player_text, sizeof(player_text), "You are Player %d", local_player_id);
        display_manager_show_centered_message(player_text, DISPLAY_HEIGHT / 2 + 10);
    }

    // Show debug info if in error state
    if (connection_status == PROTO_STATE_ERROR) {
        display_manager_show_centered_message("Check ESP32 connection", DISPLAY_HEIGHT / 2 + 20);
    }

    display_manager_update();
}

static void render_gameplay(SnakeState* players, uint8_t player_count, Position* food,
    GameStats* game_stats, uint8_t local_player_id) {

    render_game_area(players, player_count, food, game_stats, local_player_id);
    render_multiplayer_ui(local_player_id);
}

static void render_game_area(SnakeState* players, uint8_t player_count, Position* food, GameStats* game_stats, uint8_t local_player_id) {
    if (first_render) {
        display_draw_border_at(1, STATUS_START_Y, 3, 3);
        reset_render_tracking(players, player_count, food);
        first_render = false;
    }

    // Clear previous positions (like TS dirty rectangle optimization)
    clear_previous_positions(players, player_count, food);

    // Draw all players (like TS renderPlayers - no prediction, just server state)
    for (uint8_t i = 0; i < player_count; i++) {
        MultiplayerPlayerId player_id = (i == 0) ? MP_PLAYER_1 : MP_PLAYER_2;
        bool is_alive = (player_id == MP_PLAYER_1) ?
            (game_stats->p1_lives > 0) : (game_stats->p2_lives > 0);

        if (is_alive) {
            // Use different colors to distinguish players (when available)
            snake_helper_draw_snake(&players[i]);

            // TODO: Implement color differentiation for local vs opponent
            // if (player_id == local_player_id) { /* local player color */ }
            // else { /* opponent player color */ }
        }
    }

    // Draw shared food (like TS renderFood)
    snake_helper_draw_food(food);

    // Periodically redraw border to ensure it's intact
    if (get_current_ms() % 1000 == 0) {
        display_draw_border_at(1, STATUS_START_Y, 3, 3);
    }
}

// NOTE: This version of the function renders scores. For consistency across the app, I moved score rendering to display_manager where single player scores are also displayed
// static void render_multiplayer_ui(GameStats* game_stats, uint8_t local_player_id) {
//     // Clear status area
//     display_fill_rectangle(2, 2, DISPLAY_WIDTH - 2, STATUS_START_Y - 1, DISPLAY_BLACK);

//     // Draw scores for both players (like TS renderStatus)
//     char score_text[64];
//     snprintf(score_text, sizeof(score_text), "P1:%lu P2:%lu Target:%lu",
//              game_stats->p1_score, game_stats->p2_score, game_stats->target_score);

//     display_set_cursor(2, 2);
//     display_write_string(score_text, Font_7x10, DISPLAY_WHITE);

//     // Draw player identification (like TS renderPlayerId)
//     char status_text[64];
//     if (local_player_id == MP_PLAYER_1) {
//         strcpy(status_text, "You: P1 (Green)");
//     } else {
//         strcpy(status_text, "You: P2 (Blue)");
//     }

//     display_set_cursor(2, 12);
//     display_write_string(status_text, Font_7x10, DISPLAY_WHITE);
// }

static void render_multiplayer_ui(uint8_t local_player_id) {
    // Draw player identification (like TS renderPlayerId)
    char status_text[64];
    if (local_player_id == MP_PLAYER_1) {
        strcpy(status_text, "You: P1 (Green)");
    }
    else {
        strcpy(status_text, "You: P2 (Blue)");
    }

    display_set_cursor(2, 12);
    display_write_string(status_text, Font_7x10, DISPLAY_WHITE);
}


// Optimization functions (like TS rendering optimization)
static void clear_previous_positions(SnakeState* players, uint8_t player_count, Position* food) {
    // Optimized dirty rectangle clearing for multiplayer (like TS optimization)

    // Clear previous player 1 head if it moved
    if (player_count > 0 &&
        (previous_local_head_x != players[0].head_x || previous_local_head_y != players[0].head_y)) {

        // Check if previous position is not occupied by any snake
        bool is_occupied = false;

        for (uint8_t p = 0; p < player_count; p++) {
            for (uint8_t i = 0; i < players[p].length; i++) {
                if (previous_local_head_x == players[p].body[i].x &&
                    previous_local_head_y == players[p].body[i].y) {
                    is_occupied = true;
                    break;
                }
            }
            if (is_occupied) break;
        }

        if (!is_occupied) {
            display_clear_region(previous_local_head_x, previous_local_head_y, SPRITE_SIZE, SPRITE_SIZE);
        }
    }

    // Clear previous player 2 head if it moved
    if (player_count > 1 &&
        (previous_opponent_head_x != players[1].head_x || previous_opponent_head_y != players[1].head_y)) {

        // Check if previous position is not occupied by any snake
        bool is_occupied = false;

        for (uint8_t p = 0; p < player_count; p++) {
            for (uint8_t i = 0; i < players[p].length; i++) {
                if (previous_opponent_head_x == players[p].body[i].x &&
                    previous_opponent_head_y == players[p].body[i].y) {
                    is_occupied = true;
                    break;
                }
            }
            if (is_occupied) break;
        }

        if (!is_occupied) {
            display_clear_region(previous_opponent_head_x, previous_opponent_head_y, SPRITE_SIZE, SPRITE_SIZE);
        }
    }

    // Clear previous food position if changed
    if (previous_food_x != food->x || previous_food_y != food->y) {
        if (previous_food_x != 0 && previous_food_y != 0) {
            display_clear_region(previous_food_x, previous_food_y, SPRITE_SIZE, SPRITE_SIZE);
        }
    }

    // Update previous positions for next frame
    if (player_count > 0) {
        previous_local_head_x = players[0].head_x;
        previous_local_head_y = players[0].head_y;
        previous_local_length = players[0].length;
    }
    if (player_count > 1) {
        previous_opponent_head_x = players[1].head_x;
        previous_opponent_head_y = players[1].head_y;
        previous_opponent_length = players[1].length;
    }
    previous_food_x = food->x;
    previous_food_y = food->y;
}

static void reset_render_tracking(SnakeState* players, uint8_t player_count, Position* food) {
    if (player_count > 0) {
        previous_local_head_x = players[0].head_x;
        previous_local_head_y = players[0].head_y;
        previous_local_length = players[0].length;
    }
    if (player_count > 1) {
        previous_opponent_head_x = players[1].head_x;
        previous_opponent_head_y = players[1].head_y;
        previous_opponent_length = players[1].length;
    }
    previous_food_x = food->x;
    previous_food_y = food->y;
    previous_local_score = 0;
    previous_opponent_score = 0;
}
