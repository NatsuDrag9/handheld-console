/*
 * mp_snake_game_network.c
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *
 *  Network communication and message parsing for multiplayer snake
 *  Similar to TypeScript MultiplayerSnakeNetwork class
 */

#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_network.h"
#include "Utils/debug_conf.h"

static TempServerState temp_server_state = {0};

 // Private function declarations
static void mp_snake_parse_coordinate_pair(const char* str, coord_t* x, coord_t* y);
static bool mp_snake_extract_target_score(const char* game_data);

// Utility parsing functions
static void mp_snake_parse_coordinate_pair(const char* str, coord_t* x, coord_t* y) {
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


static bool mp_snake_extract_target_score(const char* game_data) {
    if (!game_data) return false;

    // Look for "target_score:" pattern
    const char* target_pos = strstr(game_data, "target_score:");
    if (!target_pos) {
        return false; // No target_score found
    }

    // Move to the value part
    const char* value_start = target_pos + 13; // Skip "target_score:"

    // Skip any whitespace
    while (*value_start == ' ' || *value_start == '\t') {
        value_start++;
    }

    // Parse the number until we hit a non-digit character
    char temp_buffer[16] = { 0 };
    int i = 0;
    while (i < 15 && *value_start >= '0' && *value_start <= '9') {
        temp_buffer[i] = *value_start;
        value_start++;
        i++;
    }

    if (i > 0) {
        uint32_t extracted_target_score = (uint32_t)atoi(temp_buffer);
        mp_snake_data.target_score = extracted_target_score;
        DEBUG_PRINTF(false, "Network: Target score extracted and set to: %lu\r\n", extracted_target_score);
        return true;
    }

    return false;
}


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
    }
    else {
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
    }
    else {
        DEBUG_PRINTF(false, "Network: Failed to send player ready: %d\r\n", status);
    }
}

// Message parsing functions
bool mp_snake_parse_player_update(const char* game_data) {
    if (!game_data) return false;

    DEBUG_PRINTF(false, "Network: Parsing player update: %s\r\n", game_data);

    // Reset and prepare temporary state
    memset(&temp_server_state, 0, sizeof(TempServerState));
    temp_server_state.valid = false;

    // Parse player 1 data into temporary buffer
    const char* p1_data = strstr(game_data, "p1:");
    if (p1_data) {
        const char* p1_end = strstr(p1_data, ";");
        if (p1_end) {
            char p1_section[64];
            size_t len = p1_end - p1_data;
            if (len < sizeof(p1_section)) {
                strncpy(p1_section, p1_data, len);
                p1_section[len] = '\0';

                temp_server_state.player1_length = mp_snake_parse_single_value(p1_section, "len");
                temp_server_state.player1_alive = (mp_snake_parse_single_value(p1_section, "alive") == 1);

                DEBUG_PRINTF(false, "Parsed P1: len=%d, alive=%d\r\n",
                           temp_server_state.player1_length, temp_server_state.player1_alive);
            }
        }
    }

    // Parse player 2 data into temporary buffer
    const char* p2_data = strstr(game_data, "p2:");
    if (p2_data) {
        const char* p2_end = strstr(p2_data, ";");
        if (p2_end) {
            char p2_section[64];
            size_t len = p2_end - p2_data;
            if (len < sizeof(p2_section)) {
                strncpy(p2_section, p2_data, len);
                p2_section[len] = '\0';

                temp_server_state.player2_length = mp_snake_parse_single_value(p2_section, "len");
                temp_server_state.player2_alive = (mp_snake_parse_single_value(p2_section, "alive") == 1);

                DEBUG_PRINTF(false, "Parsed P2: len=%d, alive=%d\r\n",
                           temp_server_state.player2_length, temp_server_state.player2_alive);
            }
        }
    }

    // Parse food data into temporary buffer
    const char* food_data = strstr(game_data, "food:");
    if (food_data) {
        coord_t server_x, server_y;
        mp_snake_parse_coordinate_pair(food_data, &server_x, &server_y);

        // Convert server coordinates to device coordinates
        temp_server_state.food_position.x = mp_snake_server_to_device_coord(server_x);
        temp_server_state.food_position.y = mp_snake_server_to_device_coord(server_y);

        DEBUG_PRINTF(false, "Parsed Food: server(%d,%d) -> device(%d,%d)\r\n",
                     server_x, server_y, temp_server_state.food_position.x, temp_server_state.food_position.y);
    }

    // Parse scores data into temporary buffer
    const char* scores_data = strstr(game_data, "scores:");
    if (scores_data) {
        const char* score_start = scores_data + 7; // Skip "scores:"
        temp_server_state.player1_score = atoi(score_start);

        const char* comma = strchr(score_start, ',');
        if (comma) {
            temp_server_state.player2_score = atoi(comma + 1);
        }

        DEBUG_PRINTF(false, "Parsed Scores: P1=%lu, P2=%lu\r\n",
                     temp_server_state.player1_score, temp_server_state.player2_score);
    }

    temp_server_state.valid = true;

    // Perform reconciliation with comparison BEFORE applying changes
    mp_snake_reconcile_with_server(&temp_server_state);

    return true;
}

bool mp_snake_parse_game_event(const char* game_data) {
    if (!game_data) return false;

    DEBUG_PRINTF(false, "Network: Parsing game event: %s\r\n", game_data);

    // Route to core event handler
    mp_snake_handle_game_event(game_data);
    return true;
}

// Parse the game state type in game_data_message
bool mp_snake_parse_game_state(const char* game_data) {
    if (!game_data) return false;

    DEBUG_PRINTF(false, "Network: Parsing full game state: %s\r\n", game_data);

    // Extract target_score if present (similar to TS regex check)
    mp_snake_extract_target_score(game_data);

    // Parse the rest of the game state data (players, food, scores, etc.)
    bool parse_result = mp_snake_parse_player_update(game_data);

    return parse_result;
}

// Callback handler for serial_comm game_data_message (simplified - just parsing)
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

// Callback handler for serial_comm connection_message
void mp_snake_on_connection_received(const uart_connection_message_t* connection_message) {
	// Don't really need the connection_message parameter in this function
	// Left it unused for future

    // Get the already-stored client ID from serial_comm
    const char* client_id = serial_comm_get_client_id();

    if (client_id && strlen(client_id) > 0) {
        strncpy(mp_snake_data.local_player_client_id, client_id,
                sizeof(mp_snake_data.local_player_client_id) - 1);
        mp_snake_data.local_player_client_id[sizeof(mp_snake_data.local_player_client_id) - 1] = '\0';

        DEBUG_PRINTF(false, "Retrieved stored client ID: %s\r\n", client_id);
    } else {
        DEBUG_PRINTF(false, "No stored client ID found\r\n");
    }
}
