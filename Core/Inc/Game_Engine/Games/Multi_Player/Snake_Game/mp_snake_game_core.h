/*
 * mp_snake_core.h
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *  Core game logic and state management for multiplayer snake
 */

#ifndef INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_CORE_H_
#define INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_CORE_H_

#include "Game_Engine/Games/Helpers/snake_game_helpers.h"
#include <stdint.h>
#include <stdbool.h>

// Game phases
typedef enum {
    MP_PHASE_WAITING,       // Waiting for second player
    MP_PHASE_PLAYING,       // Game in progress
    MP_PHASE_ENDED          // Game finished
} MultiplayerGamePhase;

// Player identifiers
typedef enum {
    MP_PLAYER_1 = 1,
    MP_PLAYER_2 = 2
} MultiplayerPlayerId;

// Game result
typedef enum {
    MP_RESULT_ONGOING = 0,
    MP_RESULT_PLAYER1_WINS = 1,
    MP_RESULT_PLAYER2_WINS = 2,
    MP_RESULT_DRAW = 3
} MultiplayerGameResult;

// Multiplayer Snake Game Data Structure (simplified - no prediction)
typedef struct {
    // Server-authoritative state (received from server)
	uint16_t session_id;
    SnakeState server_player1;
    SnakeState server_player2;
    Position server_food;
    uint32_t server_scores[2];          // [player1_score, player2_score]
    bool players_alive[2];              // [player1_alive, player2_alive]

    // Game configuration (like TS MultiplayerGameConfig)
    MultiplayerPlayerId local_player_id;    // 1 or 2
    MultiplayerPlayerId opponent_player_id; // 1 or 2
    uint32_t target_score;                  // Score needed to win
    MultiplayerGamePhase phase;
    MultiplayerGameResult winner;

    // Timing (simplified - no prediction timing)
    uint32_t last_server_update_time;
    uint32_t last_input_send_time;
    uint32_t game_start_time;

    // Connection status
    bool connected_to_server;
    bool opponent_connected;

} MultiplayerSnakeGameData;

// Core game state access
extern MultiplayerSnakeGameData mp_snake_data;

// Core game logic functions (similar to TS MultiplayerSnakeCore methods)
void mp_snake_core_init(MultiplayerPlayerId player_id, uint32_t target_score);
void mp_snake_core_cleanup(void);

// Movement simulation (like TS moveAllPlayersLocally)
void mp_snake_start_movement_simulation(void);
void mp_snake_stop_movement_simulation(void);
void mp_snake_move_all_players_locally(void);

// Game state queries (similar to TS getters)
MultiplayerPlayerId mp_snake_get_opponent_id(void);
SnakeState* mp_snake_get_local_player_state(void);
SnakeState* mp_snake_get_opponent_state(void);
Position* mp_snake_get_food(void);
SnakeState* mp_snake_get_all_players(uint8_t* player_count);

// Input validation (similar to TS canChangeDirection)
bool mp_snake_can_change_direction(uint8_t new_direction);

// Game events (similar to TS event handlers)
void mp_snake_handle_game_start(void);
void mp_snake_handle_game_end(MultiplayerGameResult result);
void mp_snake_handle_player_collision(MultiplayerPlayerId player_id);
void mp_snake_handle_food_eaten(MultiplayerPlayerId player_id);
void mp_snake_handle_game_event(const char* event_data);

// Game stats (similar to TS getGameStats)
typedef struct {
    uint32_t p1_score;
    uint32_t p1_lives;
    uint32_t p2_score;
    uint32_t p2_lives;
    uint32_t target_score;
} GameStats;

GameStats mp_snake_get_game_stats(void);


#endif /* INC_GAME_ENGINE_GAMES_MULTI_PLAYER_SNAKE_GAME_MP_SNAKE_GAME_CORE_H_ */
