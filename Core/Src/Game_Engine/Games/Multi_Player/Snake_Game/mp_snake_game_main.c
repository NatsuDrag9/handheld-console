/*
 * mp_snake_game_main.c
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *  Main public interface for multiplayer snake game
 */

#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_main.h"
#include "Utils/debug_conf.h"

// Internal game engine interface functions
static void mp_snake_init_game_engine(void);
static void mp_snake_update_dpad_internal(DPAD_STATUS dpad_status);
static void mp_snake_render_internal(void);
static void mp_snake_cleanup_internal(void);

// Initialize the multiplayer snake game engine instance
GameEngine multiplayer_snake_game_engine = {
    .init = mp_snake_init_game_engine,
    .render = mp_snake_render_internal,
    .cleanup = mp_snake_cleanup_internal,
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
}


void mp_snake_update_dpad(DPAD_STATUS dpad_status) {
    if (mp_snake_data.phase != MP_PHASE_PLAYING) return;
    if (!mp_snake_data.players_alive[mp_snake_data.local_player_id - 1]) return;

    // Process local prediction for responsive controls
    mp_snake_process_local_prediction(dpad_status);

    // Send input to server if direction changed
    if (dpad_status.is_new) {
        mp_snake_send_input_to_server(dpad_status.direction);
    }

    // Update animated sprites
    animated_sprite_update(&snake_head_animated);
}

void mp_snake_render(void) {
    // First process any pending communication
    mp_snake_process_communication();

    // Render the game
    mp_snake_render_game();
}

void mp_snake_cleanup(void) {
    DEBUG_PRINTF(false, "Multiplayer snake cleanup starting\r\n");

    // Shutdown communication
    mp_snake_shutdown_communication();

    // Cleanup core game state
    mp_snake_core_cleanup();

    // Cleanup rendering
    mp_snake_render_cleanup();

    // Reset game engine state
    multiplayer_snake_game_engine.base_state.score = 0;
    multiplayer_snake_game_engine.base_state.lives = 1;
    multiplayer_snake_game_engine.base_state.paused = false;
    multiplayer_snake_game_engine.base_state.game_over = false;

    DEBUG_PRINTF(false, "Multiplayer snake cleanup completed\r\n");
}

void mp_snake_process_communication(void) {
    // Process any pending messages from ESP32
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

    // Initialize core game logic
    mp_snake_core_init(mp_snake_data.local_player_id, mp_snake_data.target_score);

    // Initialize rendering
    mp_snake_render_init();

    // Initialize communication
    mp_snake_initialize_communication();

    DEBUG_PRINTF(false, "Multiplayer snake initialization complete\r\n");
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
