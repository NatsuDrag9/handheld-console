/*
 * mp_snake_game_core.c
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *  Core game logic and state management for multiplayer snake
 */

#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_core.h"
#include "Game_Engine/Games/Helpers/snake_game_helpers.h"
#include "Utils/misc_utils.h"
#include "Utils/debug_conf.h"
#include <string.h>
#include <stdio.h>

// Static game state
MultiplayerSnakeGameData mp_snake_data = {0};
uint32_t last_move_time = 0;
uint32_t last_prediction_move_time = 0;

// Core game logic functions
void mp_snake_core_init(MultiplayerPlayerId player_id, uint32_t target_score) {
    // Reset all state
    memset(&mp_snake_data, 0, sizeof(MultiplayerSnakeGameData));

    mp_snake_data.local_player_id = player_id;
    mp_snake_data.target_score = target_score;
    mp_snake_data.phase = MP_PHASE_WAITING;
    mp_snake_data.winner = MP_RESULT_ONGOING;

    // Initialize both players using helper functions
    coord_t start_x = DISPLAY_WIDTH / 2;
    coord_t start_y = DISPLAY_HEIGHT / 2;

    // Player 1 starts on left side, Player 2 on right side
    snake_helper_init_snake(&mp_snake_data.server_player1, start_x - 32, start_y, DPAD_DIR_RIGHT);
    snake_helper_init_snake(&mp_snake_data.server_player2, start_x + 32, start_y, DPAD_DIR_LEFT);

    // Initialize food position
    mp_snake_data.server_food.x = start_x;
    mp_snake_data.server_food.y = start_y - 32;

    // Set both players as alive initially
    mp_snake_data.players_alive[0] = true;
    mp_snake_data.players_alive[1] = true;

    // Copy server state to prediction state
    snake_helper_copy_snake_state(&mp_snake_data.predicted_local_player, mp_snake_get_local_player_state());
    snake_helper_copy_snake_state(&mp_snake_data.display_opponent, mp_snake_get_opponent_state());

    // Reset timing
    last_move_time = get_current_ms();
    last_prediction_move_time = get_current_ms();
    mp_snake_data.game_start_time = get_current_ms();

    DEBUG_PRINTF(false, "Core: Multiplayer snake initialized for Player %d\r\n", player_id);
}

void mp_snake_core_cleanup(void) {
    // Reset all state
    memset(&mp_snake_data, 0, sizeof(MultiplayerSnakeGameData));

    // Reset timing
    last_move_time = 0;
    last_prediction_move_time = 0;

    DEBUG_PRINTF(false, "Core: Multiplayer snake cleanup completed\r\n");
}

void mp_snake_process_local_prediction(DPAD_STATUS dpad_status) {
    // Apply direction change immediately for responsive controls
    if (dpad_status.is_new) {
        snake_helper_apply_direction_change(&mp_snake_data.predicted_local_player, dpad_status.direction);
    }

    // Move predicted snake at appropriate intervals
    if (mp_snake_should_move_prediction()) {
        snake_helper_move_snake(&mp_snake_data.predicted_local_player);
        last_prediction_move_time = get_current_ms();
    }
}

void mp_snake_reconcile_prediction_with_server(void) {
    // Simple reconciliation: replace prediction with server state
    // More sophisticated implementations could interpolate or validate
    snake_helper_copy_snake_state(&mp_snake_data.predicted_local_player, mp_snake_get_local_player_state());
}

void mp_snake_update_display_state(void) {
    // Update opponent display state from server
    snake_helper_copy_snake_state(&mp_snake_data.display_opponent, mp_snake_get_opponent_state());
}

// Game state utility functions
MultiplayerPlayerId mp_snake_get_opponent_id(void) {
    return (mp_snake_data.local_player_id == MP_PLAYER_1) ? MP_PLAYER_2 : MP_PLAYER_1;
}

SnakeState* mp_snake_get_local_player_state(void) {
    return (mp_snake_data.local_player_id == MP_PLAYER_1) ?
           &mp_snake_data.server_player1 : &mp_snake_data.server_player2;
}

SnakeState* mp_snake_get_opponent_state(void) {
    // Simple reconciliation: replace prediction with server state
    // More sophisticated implementations could interpolate or validate
    return (mp_snake_data.local_player_id == MP_PLAYER_1) ?
           &mp_snake_data.server_player2 : &mp_snake_data.server_player1;
}

bool mp_snake_should_move_prediction(void) {
    uint32_t current_time = get_current_ms();
    uint32_t prediction_speed = snake_helper_calculate_speed(mp_snake_data.server_scores[mp_snake_data.local_player_id - 1]);
    return (current_time - last_prediction_move_time >= prediction_speed);
}

// Game event handlers
void mp_snake_handle_game_start(void) {
    mp_snake_data.phase = MP_PHASE_PLAYING;
    mp_snake_data.opponent_connected = true;
    DEBUG_PRINTF(false, "Core: Multiplayer game started!\r\n");
}

void mp_snake_handle_game_end(MultiplayerGameResult result) {
    mp_snake_data.phase = MP_PHASE_ENDED;
    mp_snake_data.winner = result;
    DEBUG_PRINTF(false, "Core: Game ended with result: %d\r\n", result);
}

void mp_snake_handle_player_collision(MultiplayerPlayerId player_id) {
    if (player_id == MP_PLAYER_1) {
        mp_snake_data.players_alive[0] = false;
    } else if (player_id == MP_PLAYER_2) {
        mp_snake_data.players_alive[1] = false;
    }
    DEBUG_PRINTF(false, "Core: Player %d collided\r\n", player_id);
}

void mp_snake_handle_food_eaten(MultiplayerPlayerId player_id) {
    DEBUG_PRINTF(false, "Core: Player %d ate food\r\n", player_id);
}
