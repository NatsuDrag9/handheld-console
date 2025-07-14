#ifndef WEBSOCKET_CLIENT_TO_SERVER_H
#define WEBSOCKET_CLIENT_TO_SERVER_H

#include "esp_websocket_client.h"
#include "../../uart_comm.h"
#include "../../../components/cmp/cmp.h"

// Buffer size for MessagePack operations
#define MSGPACK_BUFFER_SIZE 1024

// UART → WebSocket conversion functions
esp_err_t ws_to_server_convert_connection_message(const uart_connection_message_t* uart_msg,
    esp_websocket_client_handle_t ws_client);
esp_err_t ws_to_server_convert_tile_size_validation(const uart_tile_size_validation_t* uart_msg,
    esp_websocket_client_handle_t ws_client);
esp_err_t ws_to_server_convert_game_data(const uart_game_data_t* uart_msg,
    esp_websocket_client_handle_t ws_client);
esp_err_t ws_to_server_convert_chat_message(const uart_chat_message_t* uart_msg,
    esp_websocket_client_handle_t ws_client);

// UART callback registration
void ws_to_server_register_uart_callbacks(void);

#endif // WEBSOCKET_CLIENT_TO_SERVER_H