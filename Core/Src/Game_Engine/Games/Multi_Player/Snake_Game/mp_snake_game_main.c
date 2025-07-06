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

// Network event callbacks - similar to TS NetworkEventCallbacks
static void on_connected(void);
static void on_disconnected(void);
static void on_game_data_received(const uart_game_data_t* game_data);
static void on_status_received(const uart_status_t* status);
static void on_command_received(const uart_command_t* command);

// Initialize the multiplayer snake game engine instance
GameEngine multiplayer_snake_game_engine = {
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
void mp_snake_set_game_params(MultiplayerPlayerId player_id, uint32_t target_score) {
    // Store parameters for when game engine initializes
    mp_snake_data.local_player_id = player_id;
    mp_snake_data.target_score = target_score;
    DEBUG_PRINTF(false, "Game params set: Player %d, Target Score: %lu\r\n", player_id, target_score);
}

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
    multiplayer_snake_game_engine.base_state.score = 0;
    multiplayer_snake_game_engine.base_state.lives = 1;
    multiplayer_snake_game_engine.base_state.paused = false;
    multiplayer_snake_game_engine.base_state.game_over = false;

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

    // Initialize modules in order (like TS constructor)

    // 1. Initialize core game logic
    mp_snake_core_init(mp_snake_data.local_player_id, mp_snake_data.target_score);

    // 2. Initialize rendering
    mp_snake_render_init();

    // 3. Register network callback (serial_comm already initialized by game_controller)
    serial_comm_register_game_data_callback(mp_snake_on_game_data_received);

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
        } else {
            strcpy(result_message, "PLAYER 1 WINS");
            final_score = game_stats.p1_score;
        }
    }
    else if (game_stats.p2_score > game_stats.p1_score) {
        if (mp_snake_data.local_player_id == MP_PLAYER_2) {
            strcpy(result_message, "YOU WIN!");
            final_score = game_stats.p2_score;
        } else {
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

// Network event callbacks (similar to TS NetworkEventCallbacks)
static void on_connected(void) {
    DEBUG_PRINTF(false, "Network connected, sending player ready signal\r\n");
    // Send player ready when connected
    mp_snake_send_player_ready();
}

static void on_disconnected(void) {
    DEBUG_PRINTF(false, "Network disconnected\r\n");
    // Game continues to render last known state
}

static void on_game_data_received(const uart_game_data_t* game_data) {
    if (!game_data) return;

    DEBUG_PRINTF(false, "Game data received: %s\r\n", game_data->data_type);

    // Route to appropriate handlers based on data type (like TS message routing)
    if (strcmp(game_data->data_type, "game_event") == 0) {
        // Handle real-time events (direction_changed, food_eaten, collision)
        mp_snake_handle_game_event(game_data->game_data);
    }
    else if (strcmp(game_data->data_type, "game_state") == 0) {
        // Handle state updates (length, alive, score, food)
        mp_snake_parse_game_state(game_data->game_data);
    }
    else if (strcmp(game_data->data_type, "player_update") == 0) {
        // Parse game state data
        mp_snake_parse_player_update(game_data->game_data);
    }
}

static void on_status_received(const uart_status_t* status) {
    DEBUG_PRINTF(false, "Status message received: %d\r\n", status->system_status);

    switch (status->system_status) {
        case SYSTEM_STATUS_PLAYER_ASSIGNMENT:
        	// Update mp_snake_data:
        	// 1. local_player_id
        	// 2. session id
            DEBUG_PRINTF(false, "Player assignment received\r\n");
            break;

        case SYSTEM_STATUS_OPPONENT_CONNECTED:
            mp_snake_data.opponent_connected = true;
            // Update mp_snake_data:
            // 1. opponent player id
            DEBUG_PRINTF(false, "Opponent connected\r\n");
            break;

        case SYSTEM_STATUS_OPPONENT_DISCONNECTED:
            mp_snake_data.opponent_connected = false;
            DEBUG_PRINTF(false, "Opponent disconnected\r\n");
            break;

        case SYSTEM_STATUS_WEBSOCKET_CONNECTED:
            mp_snake_data.connected_to_server = true;
            on_connected(); // Send ready signal
            break;

        case SYSTEM_STATUS_WEBSOCKET_DISCONNECTED:
        case SYSTEM_STATUS_WIFI_DISCONNECTED:
        case SYSTEM_STATUS_ERROR:
            mp_snake_data.connected_to_server = false;
            mp_snake_data.opponent_connected = false;
            on_disconnected();
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
        multiplayer_snake_game_engine.base_state.game_over = true;
    }
    else if (strcmp(command->command, "restart") == 0) {
        // Reset game state
        mp_snake_core_init(mp_snake_data.local_player_id, mp_snake_data.target_score);
        multiplayer_snake_game_engine.base_state.game_over = false;
    }
    else {
        DEBUG_PRINTF(false, "Unknown command: %s\r\n", command->command);
    }
}
