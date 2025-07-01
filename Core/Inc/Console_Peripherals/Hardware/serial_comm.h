/*
 * serial_comm.h
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_HARDWARE_SERIAL_COMM_H_
#define INC_CONSOLE_PERIPHERALS_HARDWARE_SERIAL_COMM_H_

#include <Console_Peripherals/Hardware/Drivers/uart_driver.h>
#include "System/system_conf.h"
#include "Utils/comm_utils.h"
#include <string.h>
// Include common status types
#include "common_status_types.h"


/* Message Protocol - Matches ESP32 UART module */
#define MSG_START_BYTE 0xAA
#define MSG_END_BYTE   0x55
#define MAX_PAYLOAD_SIZE 250

/* Protocol States - Simplified without AT commands */
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
/* Message Types - Must match ESP32 uart_comm.h */
typedef enum {
    MSG_TYPE_DATA = 0x01,      // For game data (repurposed)
    MSG_TYPE_COMMAND = 0x02,   // Commands between STM32<->ESP32
    MSG_TYPE_STATUS = 0x03,    // Status updates
    MSG_TYPE_CONFIG = 0x04,    // Configuration messages
    MSG_TYPE_ACK = 0x05,       // Acknowledgment
    MSG_TYPE_NACK = 0x06,      // Negative acknowledgment
    MSG_TYPE_HEARTBEAT = 0x07, // Keep-alive
    MSG_TYPE_CHAT = 0x08       // Chat messages
} MessageType;

/* Message Structure - Must match ESP32 uart_comm.h */
typedef struct {
    uint8_t start_byte;     // 0xAA - Start delimiter
    uint8_t msg_type;       // Message type
    uint8_t length;         // Data length (0-250)
    uint8_t data[250];      // Payload data
    uint8_t checksum;       // Simple checksum
    uint8_t end_byte;       // 0x55 - End delimiter
} uart_message_t;

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

#pragma pack(pop)

/* Callback function types */
typedef void (*game_data_received_callback_t)(const uart_game_data_t* game_data);
typedef void (*chat_message_received_callback_t)(const uart_chat_message_t* chat_message);
typedef void (*command_received_callback_t)(const uart_command_t* command);
typedef void (*status_received_callback_t)(const uart_status_t* status);

/* Core communication funcitons */
UART_Status serial_comm_init(void);
UART_Status serial_comm_deinit(void);
bool serial_comm_is_message_ready(void);
void serial_comm_process_messages(void);


/* Connection status functions */
bool serial_comm_is_esp32_ready(void);
bool serial_comm_is_wifi_connected(void);
bool serial_comm_is_websocket_connected(void);
ProtocolState serial_comm_get_state(void);
bool serial_comm_needs_ui_update(void);
void serial_comm_clear_ui_update_flag(void);

/* Enhanced error handling functions */
bool serial_comm_has_network_error(void);
void serial_comm_clear_network_error(void);
const char* serial_comm_get_error_message(void);

/* Message sending functions */
UART_Status serial_comm_send_message(MessageType type, const uint8_t* data, uint8_t length);
UART_Status serial_comm_send_game_data(const char* data_type, const char* game_data, const char* metadata);
UART_Status serial_comm_send_chat_message(const char* message, const char* sender, const char* chat_type);
UART_Status serial_comm_send_command(const char* command, const char* parameters);
UART_Status serial_comm_send_status(uint8_t system_status, uint8_t error_code, const char* message);
UART_Status serial_comm_send_ack(void);
UART_Status serial_comm_send_nack(void);
UART_Status serial_comm_send_heartbeat(void);

/* Callback registration functions */
void serial_comm_register_game_data_callback(game_data_received_callback_t callback);
void serial_comm_register_chat_message_callback(chat_message_received_callback_t callback);
void serial_comm_register_command_callback(command_received_callback_t callback);
void serial_comm_register_status_callback(status_received_callback_t callback);

/* Utility functions */
void serial_comm_send_debug(const char* message, uint32_t timeout);
UART_Status serial_comm_reset(void);
void serial_comm_print_stats(void);

#endif /* INC_CONSOLE_PERIPHERALS_HARDWARE_SERIAL_COMM_H_ */
