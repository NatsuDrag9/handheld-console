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
#include "uart_comm.h"  // For STM32 communication

// Configuration
#define MSGPACK_BUFFER_SIZE 1024
#define WEBSOCKET_URI "ws://192.168.53.151:3001/?client=esp32"
#define WEBSOCKET_RECONNECT_TIMEOUT_MS 10000
#define GAME_DATA_POLLING_INTERVAL 100  // Check for STM32 data every 100ms

// WebSocket status callback type
typedef void (*websocket_status_callback_t)(bool connected, const char* client_id);

// Game data callback type - called when data is received from server  
typedef void (*websocket_game_callback_t)(const char* data_type, const char* game_data, const char* metadata);

// Function declarations
void websocket_app_main(void);
esp_err_t websocket_send_game_data(const char* data_type, const char* game_data, const char* metadata);
esp_err_t websocket_send_player_action(const char* action, const char* parameters);
esp_err_t websocket_send_chat_message(const char* message);
bool websocket_is_connected(void);
const char* websocket_get_client_id(void);

// Callback registration
void websocket_register_status_callback(websocket_status_callback_t callback);
void websocket_register_game_callback(websocket_game_callback_t callback);

// Integration with STM32 UART
void websocket_init_stm32_integration(void);
void websocket_forward_to_stm32(const char* data_type, const char* game_data, const char* metadata);
void websocket_forward_from_stm32(const uart_game_data_t* stm32_data);

#endif // WEBSOCKET_CLIENT_H