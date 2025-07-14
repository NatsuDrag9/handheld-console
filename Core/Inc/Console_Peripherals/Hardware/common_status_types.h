/*
 * common_status_types.h
 *
 * Common status definitions for ESP32 and STM32 communication
 * This file should be included in both ESP32 uart_comm.h and STM32 serial_comm.h
 *
* Codes:
 * 10-19 -- Initialization states
 * 20-29 -- WiFi connection states
 * 30-39 -- WebSocket connection states
 * 40-49 -- Game session phase states (infrastructure level describing whether STM32 is ready for multiplayer gaming)
 * 60-69 -- STM32 system states
 *
 */

#ifndef COMMON_STATUS_TYPES_H
#define COMMON_STATUS_TYPES_H

 /* System Status Types - Common for ESP32 and STM32 */
typedef enum {
    /* Error and Disconnected States */
    SYSTEM_STATUS_ERROR = 0,              // General error state

    /* ESP32 Initialization States */
    SYSTEM_STATUS_ESP32_STARTED = 10,     // ESP32 has started and UART is ready
    SYSTEM_STATUS_ESP32_READY = 11,       // ESP32 initialization complete

    /* WiFi Connection States */
    SYSTEM_STATUS_WIFI_CONNECTING = 20,   // WiFi connection in progress
    SYSTEM_STATUS_WIFI_CONNECTED = 21,    // WiFi successfully connected
    SYSTEM_STATUS_WIFI_DISCONNECTED = 22, // WiFi disconnected

    /* WebSocket Connection States */
    SYSTEM_STATUS_WEBSOCKET_CONNECTING = 30, // WebSocket connection in progress  
    SYSTEM_STATUS_WEBSOCKET_CONNECTED = 31,  // WebSocket successfully connected
    SYSTEM_STATUS_WEBSOCKET_DISCONNECTED = 32, // WebSocket disconnected (kept for compatibility)

    /* Game States */
    SYSTEM_STATUS_GAME_READY = 40,        // Ready for game session
    SYSTEM_STATUS_GAME_ACTIVE = 41,       // Game session active
    SYSTEM_STATUS_GAME_ENDED = 42,        // Game session ended

    /* Player States */
    SYSTEM_STATUS_OPPONENT_DISCONNECTED = 50, // Opponent disconnected
    SYSTEM_STATUS_OPPONENT_CONNECTED = 51,    // Opponent connected
    SYSTEM_STATUS_PLAYER_ASSIGNMENT = 52,     // Player role/ID assigned
    SYSTEM_STATUS_TILE_SIZE_RESPONSE = 53, // Tile size response sent by the server
    SYSTEM_STATUS_SESSION_TIMEOUT = 54, // Inactive game session timeout

    /* STM32 States */
    SYSTEM_STATUS_STM32_READY = 60,       // STM32 ready and initialized
    SYSTEM_STATUS_STM32_GAME_READY = 61   // STM32 game logic ready
} system_status_type_t;

/* Legacy compatibility - map old enum values to new ones */
#define WEBSOCKET_DISCONNECTED  SYSTEM_STATUS_WEBSOCKET_DISCONNECTED
#define WIFI_CONNECTED          SYSTEM_STATUS_WIFI_CONNECTED  
#define WEBSOCKET_CONNECTED     SYSTEM_STATUS_WEBSOCKET_CONNECTED
#define WEBSOCKET_CONNECTING    SYSTEM_STATUS_WEBSOCKET_CONNECTING
#define OPPONENT_DISCONNECTED   SYSTEM_STATUS_OPPONENT_DISCONNECTED
#define OPPONENT_CONNECTED      SYSTEM_STATUS_OPPONENT_CONNECTED
#define PLAYER_ASSIGNMENT       SYSTEM_STATUS_PLAYER_ASSIGNMENT


/* Communication Protocol Constants */
#define HANDSHAKE_TIMEOUT_MS    3000      // Timeout for critical handshake
#define HEARTBEAT_INTERVAL_MS   30000     // Heartbeat every 30 seconds
#define RETRY_TIMEOUT_MS        1000      // Retry timeout for non-critical messages


#endif /* COMMON_STATUS_TYPES_H */