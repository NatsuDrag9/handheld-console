#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

// Include common status types from STM32 folder
#include "../../Core/Inc/Console_Peripherals/Hardware/common_status_types.h"

// UART Configuration
#define UART_PORT_NUM           UART_NUM_2
#define UART_BAUD_RATE          115200
#define UART_DATA_BITS          UART_DATA_8_BITS
#define UART_PARITY             UART_PARITY_DISABLE
#define UART_STOP_BITS          UART_STOP_BITS_1
#define UART_FLOW_CTRL          UART_HW_FLOWCTRL_DISABLE
#define UART_SOURCE_CLK         UART_SCLK_DEFAULT

// GPIO Pin Configuration
#define UART_TXD_PIN            (GPIO_NUM_17)
#define UART_RXD_PIN            (GPIO_NUM_16)
#define UART_RTS_PIN            (UART_PIN_NO_CHANGE)
#define UART_CTS_PIN            (UART_PIN_NO_CHANGE)

// Buffer Configuration
#define UART_RX_BUFFER_SIZE     (1024)
#define UART_TX_BUFFER_SIZE     (1024)
#define UART_QUEUE_SIZE         (20)
#define UART_PATTERN_CHR        '\n'
#define UART_PATTERN_QUEUE_SIZE (10)

// Message Configuration
#define UART_MAX_MESSAGE_LENGTH (256)
#define UART_TIMEOUT_MS         (1000)

// WebSocket Protocol Message Types (for ESP32 to send to server)
#define WS_MSG_TYPE_CONNECTION              "connection"
#define WS_MSG_TYPE_TILE_SIZE_VALIDATION    "tile_size_validation"
#define WS_MSG_TYPE_GAME_DATA_MESSAGE       "game_data_message"
#define WS_MSG_TYPE_CHAT_MESSAGE            "chat_message"
#define WS_MSG_TYPE_COMMAND                 "command"

#pragma pack(push, 1)
// Message Types (Protocol between ESP32 and STM32)
typedef enum {
    UART_MSG_GAME_DATA = 0x01,
    UART_MSG_COMMAND = 0x02,
    UART_MSG_STATUS = 0x03,
    UART_MSG_CONNECTION = 0x04,
    UART_MSG_ACK = 0x05,
    UART_MSG_NACK = 0x06,
    UART_MSG_HEARTBEAT = 0x07,
    UART_MSG_CHAT = 0x08,
    UART_MSG_TILE_SIZE_VALIDATION = 0x09 // For TileSizeValidationMessage
} uart_message_type_t;

// Message Structure
typedef struct {
    uint8_t start_byte;     // 0xAA - Start delimiter
    uint8_t msg_type;       // Message type from uart_message_type_t
    uint8_t length;         // Data length (0-250)
    uint8_t data[250];      // Payload data
    uint8_t checksum;       // Simple checksum
    uint8_t end_byte;       // 0x55 - End delimiter
} uart_message_t;

// Game Data Structure (repurposed from sensor data)
typedef struct {
    char data_type[16];     // "player_move", "game_state", etc.
    char game_data[64];     // JSON string or custom format
    char metadata[32];      // Additional info like player_id, timestamp, etc.
    uint32_t sequence_num;  // For ordering/deduplication
} uart_game_data_t;

// Connection Message Structure (matches WebSocket ConnectionMessage)
typedef struct {
    char client_id[7]; // Server generates client id of 6 characters + 1 terminating null character
    char message[64];       // Message content (e.g., "Acknowledge game server connection")
    uint32_t timestamp;     // Message timestamp
} uart_connection_message_t;

// Tile Size Validation Structure (matches WebSocket TileSizeValidationMessage)
typedef struct {
    uint16_t tile_size;     // Tile size value (must be multiple of 8)
    uint32_t timestamp;     // Message timestamp
} uart_tile_size_validation_t;

// Chat Message Structure
typedef struct {
    char message[96];       // Chat message content (larger than game_data)
    char sender[32];        // Sender identifier (server, client, player_id, etc.)
    char chat_type[16];     // "global", "private", "system", etc.
    uint32_t timestamp;     // Message timestamp
} uart_chat_message_t;

// Command Structure
typedef struct {
    char command[32];
    char parameters[64];
} uart_command_t;

// Status Structure
typedef struct {
    uint8_t system_status;
    uint8_t error_code;
    char status_message[32];
} uart_status_t;

// ACK tracking structure
typedef struct {
    bool waiting_for_ack;
    TickType_t ack_start_time;
    uint32_t timeout_ms;
    SemaphoreHandle_t ack_semaphore;
} ack_tracker_t;

#pragma pack(pop)

// Callback function types
typedef void (*uart_message_callback_t)(uart_message_type_t type, const uint8_t* data, size_t length);
typedef void (*uart_game_data_callback_t)(const uart_game_data_t* game_data);
typedef void (*uart_chat_message_callback_t)(const uart_chat_message_t* chat_message);
typedef void (*uart_command_callback_t)(const uart_command_t* command);
typedef void (*uart_status_callback_t)(const uart_status_t* status);
typedef void (*uart_connection_message_callback_t)(const uart_connection_message_t* connection_msg);
typedef void (*uart_tile_size_validation_callback_t)(const uart_tile_size_validation_t* tile_size_msg);

// Function Declarations
esp_err_t uart_comm_init(void);
esp_err_t uart_comm_deinit(void);
esp_err_t uart_send_message(uart_message_type_t type, const uint8_t* data, size_t length);
esp_err_t uart_send_game_data(const char* data_type, const char* game_data, const char* metadata);
esp_err_t uart_send_chat_message(const char* message, const char* sender, const char* chat_type);
esp_err_t uart_send_command(const char* command, const char* parameters);
esp_err_t uart_send_status(system_status_type_t system_status, uint8_t error_code, const char* message);
esp_err_t uart_send_connection_message(const char* client_id, const char* message);
esp_err_t uart_send_tile_size_validation(uint16_t tile_size);
esp_err_t uart_send_ack(void);
esp_err_t uart_send_nack(void);
esp_err_t uart_send_heartbeat(void);
esp_err_t uart_send_status_with_ack(system_status_type_t system_status, uint8_t error_code,
    const char* message, uint32_t timeout_ms);
esp_err_t uart_init_ack_system(void);
void uart_deinit_ack_system(void);

// Callback registration functions
void uart_register_message_callback(uart_message_callback_t callback);
void uart_register_game_data_callback(uart_game_data_callback_t callback);
void uart_register_chat_message_callback(uart_chat_message_callback_t callback);
void uart_register_command_callback(uart_command_callback_t callback);
void uart_register_status_callback(uart_status_callback_t callback);
void uart_register_connection_message_callback(uart_connection_message_callback_t callback);
void uart_register_tile_size_validation_callback(uart_tile_size_validation_callback_t callback);

// Utility functions
bool uart_is_connected(void);
void uart_flush_buffers(void);
void uart_print_stats(void);

#endif // UART_COMM_H