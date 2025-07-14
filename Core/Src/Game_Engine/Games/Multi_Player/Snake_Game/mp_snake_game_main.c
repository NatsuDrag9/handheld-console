/*
 * mp_snake_game_main.c
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *  Main public interface for multiplayer snake game
 *  Orchestrates core, network, and renderer modules like TypeScript version
 */

#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_main.h"
#include "Utils/debug_conf.h"

 // Internal game state
static bool is_initialized = false;


// Game engine callbacks
static void mp_snake_init_game_engine(void);
static void mp_snake_update_dpad_internal(DPAD_STATUS dpad_status);
static void mp_snake_render_internal(void);
static void mp_snake_cleanup_internal(void);
static void mp_snake_show_game_over_message_internal(void);

/*
 * The status and command callbacks are not placed in mp_snake_game_network because:
 * 1.) status_messages and command_messages update the game engine variable.
 * 2.) Importing the mp_snake_game_core header file in mp_snake_game_network would lead
 * to circular import issue and be inconsistent with the web-app design
 */
static void on_status_received_in_game(const uart_status_t* status);
static void on_command_received(const uart_command_t* command);

// Miscellaneous functions
static void mp_snake_load_local_player_data(void);
static void mp_snake_load_opponent_data(void);

// Initialize the multiplayer snake game engine instance
GameEngine mp_snake_game_engine = {
    .init = mp_snake_init_game_engine,
    .render = mp_snake_render_internal,
    .cleanup = mp_snake_cleanup_internal,
    .show_game_over_message = mp_snake_show_game_over_message_internal,
    .update_func = {
        .update_dpad = mp_snake_update_dpad_internal
    },
    .game_data = &mp_snake_data,
    .base_state = {
        .score = 0,
        .lives = 1,        // Multiplayer typically has single life
        .paused = false,
        .game_over = false,
        .is_reset = false
    },
    .is_d_pad_game = true
};

// Public interface implementation
void mp_snake_update_dpad(DPAD_STATUS dpad_status) {
    if (!is_initialized || mp_snake_data.phase != MP_PHASE_PLAYING) return;
    if (!mp_snake_data.players_alive[mp_snake_data.local_player_id - 1]) return;

    // Just send to server, no local prediction (like TS)
    if (dpad_status.is_new && mp_snake_can_change_direction(dpad_status.direction)) {
        mp_snake_send_input_to_server(dpad_status.direction);
        DEBUG_PRINTF(false, "Input sent to server: direction=%d\r\n", dpad_status.direction);
    }

    // Update movement simulation (like TS moveAllPlayersLocally)
    mp_snake_move_all_players_locally();
}

void mp_snake_render(void) {
    if (!is_initialized) return;

    // Process any pending communication first (like TS version)
    mp_snake_process_communication();

    // Gather all data for renderer (like TS render method parameters)
    MultiplayerGamePhase game_phase = mp_snake_data.phase;
    uint8_t player_count;
    SnakeState* players = mp_snake_get_all_players(&player_count);
    Position* food = mp_snake_get_food();
    GameStats game_stats = mp_snake_get_game_stats();
    ProtocolState connection_status = serial_comm_get_state();
    bool is_spectator = false; // Could be made configurable

    // Call renderer with all data (like TS render method)
    mp_snake_render_game(
        game_phase,
        players,
        player_count,
        food,
        &game_stats,
        connection_status,
        mp_snake_data.local_player_id,
        is_spectator
    );
}

void mp_snake_cleanup(void) {
    DEBUG_PRINTF(false, "Multiplayer snake cleanup starting\r\n");

    if (is_initialized) {
        // Cleanup rendering
        mp_snake_render_cleanup();

        // Cleanup core game state
        mp_snake_core_cleanup();

        is_initialized = false;
    }

    // Reset game engine state
    mp_snake_game_engine.base_state.score = 0;
    mp_snake_game_engine.base_state.lives = 1;
    mp_snake_game_engine.base_state.paused = false;
    mp_snake_game_engine.base_state.game_over = false;

    DEBUG_PRINTF(false, "Multiplayer snake cleanup completed\r\n");
}

void mp_snake_process_communication(void) {
    // Process any pending messages from ESP32 (similar to TS network processing)
    if (serial_comm_is_message_ready()) {
        serial_comm_process_messages();
    }
}

// Game state query functions
MultiplayerGamePhase mp_snake_get_game_phase(void) {
    return mp_snake_data.phase;
}

MultiplayerGameResult mp_snake_get_game_result(void) {
    return mp_snake_data.winner;
}

uint32_t mp_snake_get_player_score(MultiplayerPlayerId player_id) {
    if (player_id == MP_PLAYER_1) return mp_snake_data.server_scores[0];
    if (player_id == MP_PLAYER_2) return mp_snake_data.server_scores[1];
    return 0;
}

bool mp_snake_is_player_alive(MultiplayerPlayerId player_id) {
    if (player_id == MP_PLAYER_1) return mp_snake_data.players_alive[0];
    if (player_id == MP_PLAYER_2) return mp_snake_data.players_alive[1];
    return false;
}

// Internal game engine functions
static void mp_snake_init_game_engine(void) {
    DEBUG_PRINTF(false, "Multiplayer snake initializing for Player %d\r\n", mp_snake_data.local_player_id);
    mp_snake_game_engine.base_state.game_over = serial_comm_get_mp_game_over();

    // Initialize modules in order (like TS constructor)
    // 1. Load local player data and send ready message
    mp_snake_load_local_player_data();

    // 2. Initialize core game logic with loaded player ID
    mp_snake_core_init(mp_snake_data.local_player_id, mp_snake_data.target_score);

    // 3. Try to load opponent data (may not be available yet)
    mp_snake_load_opponent_data();

    // 4. Initialize rendering
    mp_snake_render_init();

    // 5. Register network callback (serial_comm already initialized by game_controller)
    serial_comm_register_game_data_callback(mp_snake_on_game_data_received);
    serial_comm_register_connection_message_callback(mp_snake_on_connection_received);
    serial_comm_register_status_callback(on_status_received_in_game);
    serial_comm_register_command_callback(on_command_received);

    is_initialized = true;
    DEBUG_PRINTF(false, "Multiplayer snake initialization complete\r\n");
}

// Game over message function implementation
static void mp_snake_show_game_over_message_internal(void) {
    GameStats game_stats = mp_snake_get_game_stats();
    char result_message[64];
    uint32_t final_score = 0;

    // Determine winner and construct message (similar to render_game_over_screen logic)
    if (game_stats.p1_score > game_stats.p2_score) {
        if (mp_snake_data.local_player_id == MP_PLAYER_1) {
            strcpy(result_message, "YOU WIN!");
            final_score = game_stats.p1_score;
        }
        else {
            strcpy(result_message, "PLAYER 1 WINS");
            final_score = game_stats.p1_score;
        }
    }
    else if (game_stats.p2_score > game_stats.p1_score) {
        if (mp_snake_data.local_player_id == MP_PLAYER_2) {
            strcpy(result_message, "YOU WIN!");
            final_score = game_stats.p2_score;
        }
        else {
            strcpy(result_message, "PLAYER 2 WINS");
            final_score = game_stats.p2_score;
        }
    }
    else {
        strcpy(result_message, "DRAW!");
        final_score = game_stats.p1_score; // Both scores are equal
    }

    // Use display manager's semantic function
    display_manager_show_game_over_message(result_message, final_score);

    DEBUG_PRINTF(false, "Game over message displayed: %s (Score: %lu)\r\n",
        result_message, final_score);
}

static void mp_snake_update_dpad_internal(DPAD_STATUS dpad_status) {
    mp_snake_update_dpad(dpad_status);
}

static void mp_snake_render_internal(void) {
    mp_snake_render();
}

static void mp_snake_cleanup_internal(void) {
    mp_snake_cleanup();
}

static void on_status_received_in_game(const uart_status_t* status) {
	// These status messages may be received during game play
    DEBUG_PRINTF(false, "Status message received: %d\r\n", status->system_status);

    switch (status->system_status) {
    case SYSTEM_STATUS_OPPONENT_DISCONNECTED:
    	mp_snake_data.opponent_connected = false;
    	DEBUG_PRINTF(false, "Opponent disconnected\r\n");
    	break;

    case SYSTEM_STATUS_WEBSOCKET_CONNECTED:
    	mp_snake_data.connected_to_server = true;
    	break;

    case SYSTEM_STATUS_SESSION_TIMEOUT:
    case SYSTEM_STATUS_WEBSOCKET_DISCONNECTED:
    case SYSTEM_STATUS_WIFI_DISCONNECTED:
    case SYSTEM_STATUS_ERROR:
    	mp_snake_game_engine.base_state.game_over = true;
    	mp_snake_data.connected_to_server = false;
    	mp_snake_data.opponent_connected = false;
    	break;

    default:
        DEBUG_PRINTF(false, "Unknown status: %d\r\n", status->system_status);
    }
}

static void on_command_received(const uart_command_t* command) {
    if (!command) return;

    DEBUG_PRINTF(false, "Command received: %s\r\n", command->command);

    if (strcmp(command->command, "game_start") == 0) {
        mp_snake_handle_game_start();
        DEBUG_PRINTF(false, "Game started!\r\n");
    }
    else if (strcmp(command->command, "game_end") == 0) {
        mp_snake_handle_game_end(MP_RESULT_ONGOING); // Could parse winner from parameters
        DEBUG_PRINTF(false, "Game ended\r\n");

        // Set game over state for engine
        mp_snake_game_engine.base_state.game_over = true;
    }
    else if (strcmp(command->command, "restart") == 0) {
        // Reset game state
        mp_snake_core_init(mp_snake_data.local_player_id, mp_snake_data.target_score);
        mp_snake_game_engine.base_state.game_over = false;
    }
    else {
        DEBUG_PRINTF(false, "Unknown command: %s\r\n", command->command);
    }
}

// Helper function that gets local data
static void mp_snake_load_local_player_data(void) {
    int player_id;
    char session_id[32];
    if (serial_comm_get_player_assignment(&player_id, session_id, sizeof(session_id), NULL, NULL, 0)) {
        mp_snake_data.local_player_id = (MultiplayerPlayerId)player_id;
        strncpy(mp_snake_data.session_id, session_id, sizeof(mp_snake_data.session_id) - 1);
        mp_snake_data.session_id[sizeof(mp_snake_data.session_id) - 1] = '\0';

        DEBUG_PRINTF(false, "Loaded local player data: ID=%d, Session=%s\r\n", player_id, session_id);

        // Send player ready immediately after loading local data
        mp_snake_send_player_ready();
    } else {
        DEBUG_PRINTF(false, "Warning: No local player assignment data available\r\n");
    }
}

// Gets opponent data
static void mp_snake_load_opponent_data(void) {
    int opponent_id;
    if (serial_comm_get_opponent_data(&opponent_id, NULL, 0, NULL, NULL, 0)) {
        mp_snake_data.opponent_player_id = (MultiplayerPlayerId)opponent_id;
        mp_snake_data.opponent_connected = true;
        DEBUG_PRINTF(false, "Loaded opponent data: ID=%d\r\n", opponent_id);
    }
}
