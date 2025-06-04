/*
 * mp_snake_game_network.c
 *
 *  Created on: Jun 4, 2025
 *      Author: rohitimandi
 *
 *  Network communication and message parsing for multiplayer snake
 */

#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_network.h"
#include "Game_Engine/Games/Multi_Player/Snake_Game/mp_snake_game_core.h"
#include "Console_Peripherals/Hardware/serial_comm.h"
#include "Utils/debug_conf.h"
#include "Utils/misc_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Network initialization and shutdown
bool mp_snake_initialize_communication(void) {
    // Initialize serial communication
    UART_Status status = serial_comm_init();
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "Network: Failed to initialize serial communication: %d\r\n", status);
        return false;
    }

    // Register callbacks for multiplayer snake
    serial_comm_register_game_data_callback(mp_snake_on_game_data_received);
    serial_comm_register_status_callback(mp_snake_on_status_received);
    serial_comm_register_command_callback(mp_snake_on_command_received);

    DEBUG_PRINTF(false, "Network: Multiplayer snake communication initialized\r\n");
    return true;
}

void mp_snake_shutdown_communication(void) {
    // Clear callbacks
    serial_comm_register_game_data_callback(NULL);
    serial_comm_register_status_callback(NULL);
    serial_comm_register_command_callback(NULL);

    // Note: We don't deinitialize serial_comm here as other modules might be using it
    DEBUG_PRINTF(false, "Network: Multiplayer snake communication shutdown\r\n");
}

//void mp_snake_process_communication(void) {
//    // Process any pending messages from ESP32
//    if (serial_comm_is_message_ready()) {
//        serial_comm_process_messages();
//    }
//}

// Message sending functions
void mp_snake_send_input_to_server(uint8_t direction) {
    char metadata[32];
    snprintf(metadata, sizeof(metadata), "player:%d,time:%lu",
             mp_snake_data.local_player_id, get_current_ms());

    char game_data[64];
    snprintf(game_data, sizeof(game_data), "direction:%d", direction);

    UART_Status status = serial_comm_send_game_data("player_input", game_data, metadata);
    if (status == UART_OK) {
        mp_snake_data.last_input_send_time = get_current_ms();
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

    UART_Status status = serial_comm_send_game_data("player_ready", game_data, metadata);
    if (status == UART_OK) {
        DEBUG_PRINTF(false, "Network: Sent player ready signal\r\n");
    } else {
        DEBUG_PRINTF(false, "Network: Failed to send player ready: %d\r\n", status);
    }
}

// Message handling callbacks
void mp_snake_on_game_data_received(const uart_game_data_t* game_data) {
    if (!game_data) return;

    DEBUG_PRINTF(false, "Network: MP Snake received game data: type='%s'\r\n", game_data->data_type);

    mp_snake_data.last_server_update_time = get_current_ms();

    if (strcmp(game_data->data_type, "player_update") == 0) {
        if (mp_snake_parse_player_update(game_data->game_data)) {
            mp_snake_reconcile_prediction_with_server();
            mp_snake_update_display_state();
        }
    }
    else if (strcmp(game_data->data_type, "game_event") == 0) {
        mp_snake_parse_game_event(game_data->game_data);
    }
    else if (strcmp(game_data->data_type, "game_state") == 0) {
        if (mp_snake_parse_game_state(game_data->game_data)) {
            // Full state synchronization
            snake_helper_copy_snake_state(&mp_snake_data.predicted_local_player, mp_snake_get_local_player_state());
            mp_snake_update_display_state();
        }
    }
}

void mp_snake_on_status_received(const uart_status_t* status) {
    if (!status) return;

    DEBUG_PRINTF(false, "Network: MP Snake received status: %d - %s\r\n",
                 status->system_status, status->status_message);

    // Update connection status based on ESP32 status
    switch (status->system_status) {
        case 4: // WebSocket connected
            mp_snake_data.connected_to_server = true;
            if (mp_snake_data.phase == MP_PHASE_WAITING) {
                // Send player ready signal
                mp_snake_send_player_ready();
            }
            break;
        case 0: // Error state
            mp_snake_data.connected_to_server = false;
            mp_snake_data.opponent_connected = false;
            break;
    }
}

void mp_snake_on_command_received(const uart_command_t* command) {
    if (!command) return;

    DEBUG_PRINTF(false, "Network: MP Snake received command: %s %s\r\n",
                 command->command, command->parameters);

    if (strcmp(command->command, "game_start") == 0) {
        mp_snake_handle_game_start();
    }
    else if (strcmp(command->command, "game_end") == 0) {
        mp_snake_handle_game_end(MP_RESULT_ONGOING); // Parse winner from parameters if needed
    }
    else if (strcmp(command->command, "opponent_connected") == 0) {
        mp_snake_data.opponent_connected = true;
    }
    else if (strcmp(command->command, "opponent_disconnected") == 0) {
        mp_snake_data.opponent_connected = false;
    }
}

void mp_snake_handle_server_message(const uart_game_data_t* server_msg) {
    // Legacy function - redirect to new callback system
    mp_snake_on_game_data_received(server_msg);
}

// Connection status queries
bool mp_snake_is_communication_ready(void) {
    return serial_comm_is_esp32_ready();
}

bool mp_snake_is_websocket_connected(void) {
    return serial_comm_is_websocket_connected();
}

ProtocolState mp_snake_get_connection_state(void) {
    return serial_comm_get_state();
}

// Message parsing utility functions
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

                mp_snake_parse_coordinate_pair(p1_section, &mp_snake_data.server_player1.head_x, &mp_snake_data.server_player1.head_y);
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

                mp_snake_parse_coordinate_pair(p2_section, &mp_snake_data.server_player2.head_x, &mp_snake_data.server_player2.head_y);
                mp_snake_data.server_player2.length = mp_snake_parse_single_value(p2_section, "len");
                mp_snake_data.players_alive[1] = (mp_snake_parse_single_value(p2_section, "alive") == 1);
            }
        }
    }

    // Find food data
    const char* food_data = strstr(game_data, "food:");
    if (food_data) {
        mp_snake_parse_coordinate_pair(food_data, &mp_snake_data.server_food.x, &mp_snake_data.server_food.y);
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

    return true;
}

bool mp_snake_parse_game_event(const char* game_data) {
    if (!game_data) return false;

    DEBUG_PRINTF(false, "Network: Parsing game event: %s\r\n", game_data);

    // Parse events like "event:food_eaten,player:1" or "event:collision,player:2,cause:self"
    if (strstr(game_data, "event:food_eaten")) {
        uint8_t player = mp_snake_parse_single_value(game_data, "player");
        mp_snake_handle_food_eaten((MultiplayerPlayerId)player);
    }
    else if (strstr(game_data, "event:collision")) {
        uint8_t player = mp_snake_parse_single_value(game_data, "player");
        mp_snake_handle_player_collision((MultiplayerPlayerId)player);
    }
    else if (strstr(game_data, "event:game_over")) {
        uint8_t winner = mp_snake_parse_single_value(game_data, "winner");
        MultiplayerGameResult result;
        switch (winner) {
            case 1: result = MP_RESULT_PLAYER1_WINS; break;
            case 2: result = MP_RESULT_PLAYER2_WINS; break;
            case 3: result = MP_RESULT_DRAW; break;
            default: result = MP_RESULT_ONGOING; break;
        }
        mp_snake_handle_game_end(result);
    }

    return true;
}

bool mp_snake_parse_game_state(const char* game_data) {
    // Full game state parsing - similar to player_update but with complete snake body data
    DEBUG_PRINTF(false, "Network: Parsing full game state: %s\r\n", game_data);

    // For now, use the same parsing as player_update
    // In a full implementation, this would parse complete snake body positions
    return mp_snake_parse_player_update(game_data);
}
