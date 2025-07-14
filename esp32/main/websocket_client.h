#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "esp_http_client.h"
#include "esp_websocket_client.h"
#include "uart_comm.h"
#include "../components/cmp/cmp.h"

// Configuration
#define WEBSOCKET_URI "ws://192.168.2.151:3001/?client=esp32"
#define WEBSOCKET_RECONNECT_TIMEOUT_MS 10000
#define GAME_DATA_POLLING_INTERVAL 100  // Check for STM32 data every 100ms

// WebSocket status callback type
typedef void (*websocket_status_callback_t)(bool connected, const char* client_id);

// Game data callback type - called when data is received from server  
typedef void (*websocket_game_callback_t)(const char* data_type, const char* game_data, const char* metadata);

// Connection message callback type
typedef void (*websocket_connection_callback_t)(const char* message_id, const char* message_text);

// Status message callback type  
typedef void (*websocket_status_message_callback_t)(const char* status_type, const char* message_data);

// Global WebSocket client handle (accessible by helper modules)
extern esp_websocket_client_handle_t ws_client;

// Function declarations
void websocket_app_main(void);
bool websocket_is_connected(void);
const char* websocket_get_client_id(void);

// Callback registration
void websocket_register_status_callback(websocket_status_callback_t callback);
void websocket_register_game_callback(websocket_game_callback_t callback);
void websocket_register_connection_callback(websocket_connection_callback_t callback);
void websocket_register_status_message_callback(websocket_status_message_callback_t callback);

// Integration initialization
void websocket_init_integrations(void);

// MessagePack utility functions (shared with helper modules)
bool websocket_buffer_reader(struct cmp_ctx_s* ctx, void* data, size_t limit);
bool websocket_buffer_skipper(struct cmp_ctx_s* ctx, size_t count);
size_t websocket_buffer_writer(struct cmp_ctx_s* ctx, const void* data, size_t count);

#endif // WEBSOCKET_CLIENT_H