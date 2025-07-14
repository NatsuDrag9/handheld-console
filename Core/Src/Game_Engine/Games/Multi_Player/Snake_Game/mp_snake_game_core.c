/*
 * mp_snake_game_core.c
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *  Core game logic and state management for multiplayer snake
 *  Similar to TypeScript MultiplayerSnakeCore class
 */

#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_core.h"
#include "Utils/misc_utils.h"
#include "Utils/debug_conf.h"


 // Static game state (similar to private properties in TS class)
MultiplayerSnakeGameData mp_snake_data = { 0 };

// Movement simulation state
static uint32_t last_movement_time = 0;
static bool movement_simulation_active = false;
static const uint32_t MOVEMENT_INTERVAL_MS = 100;

// Server reconciliation tracking
static uint32_t last_processed_sequence = 0;
static uint32_t last_server_reconciliation = 0;

// Utility helpers
static void mp_snake_apply_server_state(const TempServerState* server_state);

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

    // Player 1 starts on left side, Player 2 on right side (like TS)
    snake_helper_init_snake(&mp_snake_data.player1,
        start_x - 32, start_y, DPAD_DIR_RIGHT);
    snake_helper_init_snake(&mp_snake_data.player2,
        start_x + 32, start_y, DPAD_DIR_LEFT);

    // Initialize food position
    mp_snake_data.server_food.x = start_x;
    mp_snake_data.server_food.y = start_y - 32;

    // Set both players as alive initially
    mp_snake_data.players_alive[0] = true;
    mp_snake_data.players_alive[1] = true;

    // Reset timing
    last_movement_time = get_current_ms();
    last_processed_sequence = 0;
    last_server_reconciliation = 0;
    mp_snake_data.game_start_time = get_current_ms();
    movement_simulation_active = false;

    DEBUG_PRINTF(false, "Core: Multiplayer snake initialized for Player %d\r\n", player_id);
}

void mp_snake_core_cleanup(void) {
    // Stop movement simulation first
    mp_snake_stop_movement_simulation();

    // Reset all state (similar to TS cleanup)
    memset(&mp_snake_data, 0, sizeof(MultiplayerSnakeGameData));

    // Reset timing
    last_movement_time = 0;
    last_processed_sequence = 0;
    last_server_reconciliation = 0;
    movement_simulation_active = false;

    DEBUG_PRINTF(false, "Core: Multiplayer snake cleanup completed\r\n");
}

// Movement simulation
void mp_snake_start_movement_simulation(void) {
    movement_simulation_active = true;
    last_movement_time = get_current_ms();
    DEBUG_PRINTF(false, "Core: Movement simulation started\r\n");
}

void mp_snake_stop_movement_simulation(void) {
    movement_simulation_active = false;
    DEBUG_PRINTF(false, "Core: Movement simulation stopped\r\n");
}

void mp_snake_move_all_players_locally(void) {
    // Moves both players based on server state
    if (!movement_simulation_active || mp_snake_data.phase != MP_PHASE_PLAYING) {
        return;
    }

    uint32_t current_time = get_current_ms();
    if (current_time - last_movement_time < MOVEMENT_INTERVAL_MS) {
        return; // Not time to move yet
    }

    // Move all players
    if (mp_snake_data.players_alive[0]) {
        snake_helper_move_snake(&mp_snake_data.player1);
        snake_helper_wrap_coordinates(&mp_snake_data.player1.head_x,
            &mp_snake_data.player1.head_y);
    }

    if (mp_snake_data.players_alive[1]) {
        snake_helper_move_snake(&mp_snake_data.player2);
        snake_helper_wrap_coordinates(&mp_snake_data.player2.head_x,
            &mp_snake_data.player2.head_y);
    }

    last_movement_time = current_time;
}

// Game state utility functions
MultiplayerPlayerId mp_snake_get_opponent_id(void) {
    return (mp_snake_data.local_player_id == MP_PLAYER_1) ? MP_PLAYER_2 : MP_PLAYER_1;
}

SnakeState* mp_snake_get_local_player_state(void) {
    return (mp_snake_data.local_player_id == MP_PLAYER_1) ?
        &mp_snake_data.player1 : &mp_snake_data.player2;
}

SnakeState* mp_snake_get_opponent_state(void) {
    return (mp_snake_data.local_player_id == MP_PLAYER_1) ?
        &mp_snake_data.player2 : &mp_snake_data.player1;
}

Position* mp_snake_get_food(void) {
    return &mp_snake_data.server_food;
}

// Get all players for renderer
SnakeState* mp_snake_get_all_players(uint8_t* player_count) {
    static SnakeState players[2];

    players[0] = mp_snake_data.player1;
    players[1] = mp_snake_data.player2;

    if (player_count) {
        *player_count = 2;
    }

    return players;
}

// Input validation (similar to TS canChangeDirection)
bool mp_snake_can_change_direction(uint8_t new_direction) {
    SnakeState* local_player = mp_snake_get_local_player_state();
    if (!local_player) {
        DEBUG_PRINTF(false, "canChangeDirection: No local player found\r\n");
        return false;
    }

    if (!mp_snake_data.players_alive[mp_snake_data.local_player_id - 1]) {
        DEBUG_PRINTF(false, "canChangeDirection: Local player is dead\r\n");
        return false;
    }

    // Validate direction using helper
    return snake_helper_is_valid_direction_change(local_player->direction, new_direction);
}

// Reconcile with server
void mp_snake_reconcile_with_server(const TempServerState* server_state) {
    if (!server_state || !server_state->valid) {
        DEBUG_PRINTF(false, "Core: Invalid server state for reconciliation\r\n");
        return;
    }

    DEBUG_PRINTF(false, "Core: Performing server reconciliation with comparison\r\n");

    bool discrepancies_found = false;

    // Player 1 reconciliation
    if (mp_snake_data.player1.length != server_state->player1_length) {
        DEBUG_PRINTF(false, "[RECONCILIATION] P1 length mismatch: local=%d, server=%d\r\n",
            mp_snake_data.player1.length, server_state->player1_length);
        discrepancies_found = true;
    }

    if (mp_snake_data.players_alive[0] != server_state->player1_alive) {
        DEBUG_PRINTF(false, "[RECONCILIATION] P1 alive status mismatch: local=%d, server=%d\r\n",
            mp_snake_data.players_alive[0], server_state->player1_alive);
        discrepancies_found = true;

        // Special handling for death state changes
        if (mp_snake_data.players_alive[0] && !server_state->player1_alive) {
            DEBUG_PRINTF(false, "[RECONCILIATION] P1 died on server\r\n");
        }
        else if (!mp_snake_data.players_alive[0] && server_state->player1_alive) {
            DEBUG_PRINTF(false, "[RECONCILIATION] P1 revived on server (unusual)\r\n");
        }
    }

    if (mp_snake_data.server_scores[0] != server_state->player1_score) {
        DEBUG_PRINTF(false, "[RECONCILIATION] P1 score mismatch: local=%lu, server=%lu\r\n",
            mp_snake_data.server_scores[0], server_state->player1_score);
        discrepancies_found = true;
    }

    // Player 2 reconciliation
    if (mp_snake_data.player2.length != server_state->player2_length) {
        DEBUG_PRINTF(false, "[RECONCILIATION] P2 length mismatch: local=%d, server=%d\r\n",
            mp_snake_data.player2.length, server_state->player2_length);
        discrepancies_found = true;
    }

    if (mp_snake_data.players_alive[1] != server_state->player2_alive) {
        DEBUG_PRINTF(false, "[RECONCILIATION] P2 alive status mismatch: local=%d, server=%d\r\n",
            mp_snake_data.players_alive[1], server_state->player2_alive);
        discrepancies_found = true;

        // Special handling for death state changes
        if (mp_snake_data.players_alive[1] && !server_state->player2_alive) {
            DEBUG_PRINTF(false, "[RECONCILIATION] P2 died on server\r\n");
        }
        else if (!mp_snake_data.players_alive[1] && server_state->player2_alive) {
            DEBUG_PRINTF(false, "[RECONCILIATION] P2 revived on server (unusual)\r\n");
        }
    }

    if (mp_snake_data.server_scores[1] != server_state->player2_score) {
        DEBUG_PRINTF(false, "[RECONCILIATION] P2 score mismatch: local=%lu, server=%lu\r\n",
            mp_snake_data.server_scores[1], server_state->player2_score);
        discrepancies_found = true;
    }

    // Food position reconciliation
    if (mp_snake_data.server_food.x != server_state->food_position.x ||
        mp_snake_data.server_food.y != server_state->food_position.y) {
        DEBUG_PRINTF(false, "[RECONCILIATION] Food position mismatch: local(%d,%d), server(%d,%d)\r\n",
            mp_snake_data.server_food.x, mp_snake_data.server_food.y,
            server_state->food_position.x, server_state->food_position.y);
        discrepancies_found = true;
    }

    if (discrepancies_found) {
        DEBUG_PRINTF(false, "[RECONCILIATION] Discrepancies found - server state will be applied\r\n");
    }
    else {
        DEBUG_PRINTF(false, "[RECONCILIATION] Local state matches server state\r\n");
    }

    // Apply server state after reconciliation
    mp_snake_apply_server_state(server_state);


    // Update reconciliation timestamp
    mp_snake_data.last_server_update_time = get_current_ms();
}

// Game event handlers (similar to TS event handlers)
void mp_snake_handle_game_start(void) {
    mp_snake_data.phase = MP_PHASE_PLAYING;
    mp_snake_data.opponent_connected = true;
    mp_snake_start_movement_simulation(); // Start local movement simulation
    DEBUG_PRINTF(false, "Core: Multiplayer game started!\r\n");
}

void mp_snake_handle_game_end(MultiplayerGameResult result) {
    mp_snake_data.phase = MP_PHASE_ENDED;
    mp_snake_data.winner = result;
    mp_snake_stop_movement_simulation(); // Stop movement simulation
    DEBUG_PRINTF(false, "Core: Game ended with result: %d\r\n", result);
}

void mp_snake_handle_player_collision(MultiplayerPlayerId player_id) {
    if (player_id == MP_PLAYER_1) {
        mp_snake_data.players_alive[0] = false;
    }
    else if (player_id == MP_PLAYER_2) {
        mp_snake_data.players_alive[1] = false;
    }
    DEBUG_PRINTF(false, "Core: Player %d collided\r\n", player_id);
}

void mp_snake_handle_food_eaten(MultiplayerPlayerId player_id) {
    if (player_id == MP_PLAYER_1) {
        mp_snake_data.server_scores[0]++;
        mp_snake_data.player1.length++;
    }
    else if (player_id == MP_PLAYER_2) {
        mp_snake_data.server_scores[1]++;
        mp_snake_data.player2.length++;
    }
    DEBUG_PRINTF(false, "Core: Player %d ate food, new score: %lu\r\n",
        player_id, mp_snake_data.server_scores[player_id - 1]);
}

// Game event parsing (similar to TS handleGameEvent)
void mp_snake_handle_game_event(const char* event_data) {
    if (!event_data) return;

    DEBUG_PRINTF(false, "Core: Handling game event: %s\r\n", event_data);

    if (strstr(event_data, "\"event\":\"direction_changed\"")) {
        const char* player_pos = strstr(event_data, "\"playerId\":");
        uint8_t player = 0;
        if (player_pos) {
            player = atoi(player_pos + 11);
        }

        const char* dir_pos = strstr(event_data, "\"direction\":");
        uint8_t direction = 0;
        if (dir_pos) {
            direction = atoi(dir_pos + 12);
        }

        const char* seq_pos = strstr(event_data, "\"sequence\":");
        uint32_t sequence = 0;
        if (seq_pos) {
            sequence = strtoul(seq_pos + 11, NULL, 10);
        }

        if (sequence <= last_processed_sequence) {
            DEBUG_PRINTF(false, "Ignoring old sequence %lu (last: %lu)\r\n",
                sequence, last_processed_sequence);
            return;
        }
        last_processed_sequence = sequence;

        if (player == MP_PLAYER_1) {
            mp_snake_data.player1.direction = direction;
        }
        else if (player == MP_PLAYER_2) {
            mp_snake_data.player2.direction = direction;
        }

        DEBUG_PRINTF(false, "Player %d direction changed to %d\r\n", player, direction);
    }
    else if (strstr(event_data, "\"event\":\"food_eaten\"")) {
        const char* player_pos = strstr(event_data, "\"playerId\":");
        uint8_t player = 0;
        if (player_pos) {
            player = atoi(player_pos + 11);
        }

        mp_snake_handle_food_eaten((MultiplayerPlayerId)player);

        // Parse new food position with coordinate conversion
        const char* food_x_pos = strstr(event_data, "\"newFoodX\":");
        const char* food_y_pos = strstr(event_data, "\"newFoodY\":");
        if (food_x_pos && food_y_pos) {
            coord_t server_x = atoi(food_x_pos + 11);
            coord_t server_y = atoi(food_y_pos + 11);

            // Convert server coordinates to device coordinates
            mp_snake_data.server_food.x = mp_snake_server_to_device_coord(server_x);
            mp_snake_data.server_food.y = mp_snake_server_to_device_coord(server_y);

            DEBUG_PRINTF(false, "New food: server(%d,%d) -> device(%d,%d)\r\n",
                server_x, server_y, mp_snake_data.server_food.x, mp_snake_data.server_food.y);
        }
    }
    else if (strstr(event_data, "\"event\":\"collision\"")) {
        const char* player_pos = strstr(event_data, "\"playerId\":");
        uint8_t player = 0;
        if (player_pos) {
            player = atoi(player_pos + 11);
        }

        mp_snake_handle_player_collision((MultiplayerPlayerId)player);
    }
    else {
        DEBUG_PRINTF(false, "Unknown event type in: %s\r\n", event_data);
    }
}
// Game stats (similar to TS getGameStats)
GameStats mp_snake_get_game_stats(void) {
    GameStats stats = {
        .p1_score = mp_snake_data.server_scores[0],
        .p1_lives = mp_snake_data.players_alive[0] ? 1 : 0,
        .p2_score = mp_snake_data.server_scores[1],
        .p2_lives = mp_snake_data.players_alive[1] ? 1 : 0,
        .target_score = mp_snake_data.target_score
    };
    return stats;
}

// Utility functions
void mp_snake_apply_server_state(const TempServerState* server_state) {
    if (!server_state || !server_state->valid) {
        DEBUG_PRINTF(false, "Core: Invalid server state - not applying\r\n");
        return;
    }

    DEBUG_PRINTF(false, "Core: Applying authoritative server state\r\n");

    // Apply player 1 state
    mp_snake_data.player1.length = server_state->player1_length;
    mp_snake_data.players_alive[0] = server_state->player1_alive;
    mp_snake_data.server_scores[0] = server_state->player1_score;

    // Apply player 2 state
    mp_snake_data.player2.length = server_state->player2_length;
    mp_snake_data.players_alive[1] = server_state->player2_alive;
    mp_snake_data.server_scores[1] = server_state->player2_score;

    // Apply food position
    mp_snake_data.server_food.x = server_state->food_position.x;
    mp_snake_data.server_food.y = server_state->food_position.y;

    DEBUG_PRINTF(false, "Core: Server state applied - P1(len=%d,alive=%d,score=%lu) P2(len=%d,alive=%d,score=%lu)\r\n",
        mp_snake_data.player1.length, mp_snake_data.players_alive[0], mp_snake_data.server_scores[0],
        mp_snake_data.player2.length, mp_snake_data.players_alive[1], mp_snake_data.server_scores[1]);
}




