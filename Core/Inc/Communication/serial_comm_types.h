/*
 * serial_comm_types.h
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_COMMUNICATION_SERIAL_COMM_TYPES_H_
#define INC_COMMUNICATION_SERIAL_COMM_TYPES_H_

#include <stdint.h>
 // #include "Console_Peripherals/Hardware/serial_comm_core.h"
#include "../Console_Peripherals/Hardware/common_status_types.h"

 /* Message Protocol Constants */
#define MSG_START_BYTE 0xAA
#define MSG_END_BYTE   0x55
#define MAX_PAYLOAD_SIZE 250

/* Message Types - Must match ESP32 uart_comm.h */
typedef enum {
    MSG_TYPE_DATA = 0x01,
    MSG_TYPE_COMMAND = 0x02,
    MSG_TYPE_STATUS = 0x03,
    MSG_TYPE_CONNECTION = 0x04,
    MSG_TYPE_ACK = 0x05,
    MSG_TYPE_NACK = 0x06,
    MSG_TYPE_HEARTBEAT = 0x07,
    MSG_TYPE_CHAT = 0x08,
    MSG_TILE_SIZE_VALIDATION = 0x09,
} MessageType;

/* Message Structure */
#pragma pack(push, 1)
typedef struct {
    uint8_t start_byte;
    uint8_t msg_type;
    uint8_t length;
    uint8_t data[250];
    uint8_t checksum;
    uint8_t end_byte;
} uart_message_t;
#pragma pack(pop)

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

/* Callback function types */
typedef void (*game_data_received_callback_t)(const uart_game_data_t* game_data);
typedef void (*chat_message_received_callback_t)(const uart_chat_message_t* chat_message);
typedef void (*command_received_callback_t)(const uart_command_t* command);
typedef void (*status_received_callback_t)(const uart_status_t* status);
typedef void (*connection_message_callback_t)(const uart_connection_message_t* connection_msg);
typedef void (*uart_message_received_callback_t)(const uart_message_t* message);

#endif /* INC_COMMUNICATION_SERIAL_COMM_TYPES_H_ */
