/*
 * serial_comm_protocol.h
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_COMMUNICATION_SERIAL_COMM_PROTOCOL_H_
#define INC_COMMUNICATION_SERIAL_COMM_PROTOCOL_H_

#include <Console_Peripherals/Hardware/serial_comm_core.h>
#include "System/system_conf.h"
#include "Utils/comm_utils.h"
#include "common_status_types.h"

/* Protocol States */
typedef enum {
    PROTO_STATE_INIT,                    // Initial state - waiting for ESP32
    PROTO_STATE_ESP32_READY,             // ESP32 has sent ready signal
    PROTO_STATE_WIFI_CONNECTING,         // ESP32 connecting to WiFi
    PROTO_STATE_WIFI_CONNECTED,          // WiFi connected
    PROTO_STATE_WEBSOCKET_CONNECTING,    // WebSocket connecting
    PROTO_STATE_WEBSOCKET_CONNECTED,     // WebSocket connected - ready for game data
    PROTO_STATE_GAME_READY,              // Game session ready (waiting for players/setup)
    PROTO_STATE_GAME_ACTIVE,             // Game session active (game in progress)
    PROTO_STATE_ERROR                    // Error occurred
} ProtocolState;

#pragma pack(push, 1)
/* Game Data Structure */
typedef struct {
    char data_type[16];     // "player_move", "game_state", etc.
    char game_data[64];     // JSON string or custom format
    char metadata[32];      // Additional info like player_id, timestamp, etc.
    uint32_t sequence_num;  // For ordering/deduplication
} uart_game_data_t;

/* Chat Message Structure */
typedef struct {
    char message[96];       // Chat message content (larger than game_data)
    char sender[32];        // Sender identifier (server, client, player_id, etc.)
    char chat_type[16];     // "global", "private", "system", etc.
    uint32_t timestamp;     // Message timestamp
} uart_chat_message_t;

/* Command Structure */
typedef struct {
    char command[32];       // Command name
    char parameters[64];    // Command parameters
} uart_command_t;

/* Status Structure */
typedef struct {
    uint8_t system_status;  // System status code
    uint8_t error_code;     // Error code if any
    char status_message[32]; // Human readable status
} uart_status_t;

/* Connection message structure (matches WebSocket ConnectionMessage) */
typedef struct {
    char client_id[7];      // Server generates client id of 6 characters + 1 terminating null character
    char message[64];       // Message content (e.g., "Acknowledge game server connection")
    uint32_t timestamp;     // Message timestamp
} uart_connection_message_t;

/* Tile size validation structure (matches WebSocket TileSizeValidationMessage) */
typedef struct {
    uint16_t tile_size;     // Tile size value (must be multiple of 8)
    uint32_t timestamp;     // Message timestamp
} uart_tile_size_validation_t;
#pragma pack(pop)

/* Protocol initialization and control */
void protocol_init(void);
void protocol_deinit(void);
void protocol_process(void);
void protocol_reset(void);

/* Connection status functions */
bool protocol_is_esp32_ready(void);
bool protocol_is_wifi_connected(void);
bool protocol_is_websocket_connected(void);
ProtocolState protocol_get_state(void);
bool protocol_needs_ui_update(void);
void protocol_clear_ui_update_flag(void);

/* Enhanced error handling functions */
bool protocol_has_network_error(void);
void protocol_clear_network_error(void);
const char* protocol_get_error_message(void);

/* Message sending functions */
UART_Status protocol_send_message(MessageType type, const uint8_t* data, uint8_t length);
UART_Status protocol_send_game_data(const char* data_type, const char* game_data, const char* metadata);
UART_Status protocol_send_chat_message(const char* message, const char* sender, const char* chat_type);
UART_Status protocol_send_command(const char* command, const char* parameters);
UART_Status protocol_send_status(uint8_t system_status, uint8_t error_code, const char* message);
UART_Status protocol_send_connection_message(const char* message, const char* client_id);
UART_Status protocol_send_tile_size_validation(uint8_t tile_size);
UART_Status protocol_send_ack(void);
UART_Status protocol_send_nack(void);
UART_Status protocol_send_heartbeat(void);

/* Multiplayer communication getter Functions */
const char* protocol_get_client_id(void);
bool protocol_get_mp_game_over(void);
bool protocol_get_player_assignment(int* player_id, char* session_id, size_t session_id_size,
                                   int* player_count, char* color, size_t color_size);
bool protocol_get_opponent_data(int* player_id, char* session_id, size_t session_id_size,
                               int* player_count, char* color, size_t color_size);
bool protocol_has_player_assignment(void);
bool protocol_has_opponent_connected(void);
int protocol_get_local_player_id(void);
int protocol_get_opponent_player_id(void);

/* Utility functions */
void protocol_send_debug(const char* message, uint32_t timeout);
void protocol_print_stats(void);



#endif /* INC_COMMUNICATION_SERIAL_COMM_PROTOCOL_H_ */
