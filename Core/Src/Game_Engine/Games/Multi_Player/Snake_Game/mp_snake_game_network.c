/*
 * mp_snake_game_network.c
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *  Network communication and message parsing for multiplayer snake
 *  Similar to TypeScript MultiplayerSnakeNetwork class
 */

#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_network.h"
#include "Utils/debug_conf.h"

// Message sending functions
void mp_snake_send_input_to_server(uint8_t direction) {
    char metadata[32];
    snprintf(metadata, sizeof(metadata), "player:%d,time:%lu",
             mp_snake_data.local_player_id, get_current_ms());

    char game_data[64];
    snprintf(game_data, sizeof(game_data), "direction:%d", direction);

    UART_Status status = serial_comm_send_game_data("player_action", game_data, metadata);
    if (status == UART_OK) {
        DEBUG_PRINTF(false, "Network: Sent input to server: direction=%d\r\n", direction);
    } else {
        DEBUG_PRINTF(false, "Network: Failed to send input to server: %d\r\n", status);
    }
}

void mp_snake_send_player_ready(void) {
    char metadata[32];
    snprintf(metadata, sizeof(metadata), "player:%d", mp_snake_data.local_player_id);

    char game_data[64];
    snprintf(game_data, sizeof(game_data), "target_score:%lu", mp_snake_data.target_score);

    UART_Status status = serial_comm_send_game_data("game_event", game_data, metadata);
    if (status == UART_OK) {
        DEBUG_PRINTF(false, "Network: Sent player ready signal\r\n");
    } else {
        DEBUG_PRINTF(false, "Network: Failed to send player ready: %d\r\n", status);
    }
}

// Message parsing functions
bool mp_snake_parse_player_update(const char* game_data) {
    if (!game_data) return false;

    // Parse format: "p1:x:45,y:67,len:8,alive:1;p2:x:23,y:34,len:5,alive:1;food:x:12,y:56;scores:4,7"
    DEBUG_PRINTF(false, "Network: Parsing player update: %s\r\n", game_data);

    // Find player 1 data
    const char* p1_data = strstr(game_data, "p1:");
    if (p1_data) {
        const char* p1_end = strstr(p1_data, ";");
        if (p1_end) {
            char p1_section[64];
            size_t len = p1_end - p1_data;
            if (len < sizeof(p1_section)) {
                strncpy(p1_section, p1_data, len);
                p1_section[len] = '\0';

                mp_snake_parse_coordinate_pair(p1_section,
                    &mp_snake_data.server_player1.head_x,
                    &mp_snake_data.server_player1.head_y);
                mp_snake_data.server_player1.length = mp_snake_parse_single_value(p1_section, "len");
                mp_snake_data.players_alive[0] = (mp_snake_parse_single_value(p1_section, "alive") == 1);
            }
        }
    }

    // Find player 2 data
    const char* p2_data = strstr(game_data, "p2:");
    if (p2_data) {
        const char* p2_end = strstr(p2_data, ";");
        if (p2_end) {
            char p2_section[64];
            size_t len = p2_end - p2_data;
            if (len < sizeof(p2_section)) {
                strncpy(p2_section, p2_data, len);
                p2_section[len] = '\0';

                mp_snake_parse_coordinate_pair(p2_section,
                    &mp_snake_data.server_player2.head_x,
                    &mp_snake_data.server_player2.head_y);
                mp_snake_data.server_player2.length = mp_snake_parse_single_value(p2_section, "len");
                mp_snake_data.players_alive[1] = (mp_snake_parse_single_value(p2_section, "alive") == 1);
            }
        }
    }

    // Find food data
    const char* food_data = strstr(game_data, "food:");
    if (food_data) {
        mp_snake_parse_coordinate_pair(food_data,
            &mp_snake_data.server_food.x,
            &mp_snake_data.server_food.y);
    }

    // Find scores data
    const char* scores_data = strstr(game_data, "scores:");
    if (scores_data) {
        const char* score_start = scores_data + 7; // Skip "scores:"
        mp_snake_data.server_scores[0] = atoi(score_start);

        const char* comma = strchr(score_start, ',');
        if (comma) {
            mp_snake_data.server_scores[1] = atoi(comma + 1);
        }
    }

    // Check for target score
    const char* target_data = strstr(game_data, "target_score:");
    if (target_data) {
        mp_snake_data.target_score = atoi(target_data + 13);
        DEBUG_PRINTF(false, "Network: Target score updated to: %lu\r\n", mp_snake_data.target_score);
    }

    return true;
}

bool mp_snake_parse_game_event(const char* game_data) {
    if (!game_data) return false;

    DEBUG_PRINTF(false, "Network: Parsing game event: %s\r\n", game_data);

    // Route to core event handler
    mp_snake_handle_game_event(game_data);
    return true;
}

bool mp_snake_parse_game_state(const char* game_data) {
    // Full game state parsing - similar to player_update but with complete snake body data
    DEBUG_PRINTF(false, "Network: Parsing full game state: %s\r\n", game_data);

    // For now, use the same parsing as player_update
    // In a full implementation, this would parse complete snake body positions
    return mp_snake_parse_player_update(game_data);
}

// Utility parsing functions
void mp_snake_parse_coordinate_pair(const char* str, coord_t* x, coord_t* y) {
    if (!str || !x || !y) return;

    // Parse "x:45,y:67" format
    const char* x_pos = strstr(str, "x:");
    const char* y_pos = strstr(str, "y:");

    if (x_pos) {
        *x = (coord_t)atoi(x_pos + 2);
    }
    if (y_pos) {
        *y = (coord_t)atoi(y_pos + 2);
    }
}

uint8_t mp_snake_parse_single_value(const char* str, const char* key) {
    if (!str || !key) return 0;

    char search_key[16];
    snprintf(search_key, sizeof(search_key), "%s:", key);

    const char* pos = strstr(str, search_key);
    if (pos) {
        return (uint8_t)atoi(pos + strlen(search_key));
    }
    return 0;
}

// Callback handler for serial_comm (simplified - just parsing)
void mp_snake_on_game_data_received(const uart_game_data_t* game_data) {
    if (!game_data) return;

    DEBUG_PRINTF(false, "Network: Game data received: type='%s'\r\n", game_data->data_type);

    // Route based on data type - just parsing, main module handles the rest
    if (strcmp(game_data->data_type, "player_update") == 0) {
        mp_snake_parse_player_update(game_data->game_data);
    }
    else if (strcmp(game_data->data_type, "game_event") == 0) {
        mp_snake_parse_game_event(game_data->game_data);
    }
    else if (strcmp(game_data->data_type, "game_state") == 0) {
        mp_snake_parse_game_state(game_data->game_data);
    }
    else {
        DEBUG_PRINTF(false, "Network: Unknown game data type: %s\r\n", game_data->data_type);
    }
}
