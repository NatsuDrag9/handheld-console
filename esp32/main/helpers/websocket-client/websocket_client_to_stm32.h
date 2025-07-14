#ifndef WEBSOCKET_CLIENT_TO_STM32_H
#define WEBSOCKET_CLIENT_TO_STM32_H

#include "../../uart_comm.h"
#include <string.h>


// WebSocket → STM32 conversion functions
void ws_to_stm32_handle_connection_message(const char* message_id, const char* message_text);
void ws_to_stm32_handle_status_message(const char* status_type, const char* message_data,
    const char* additional_data);
void ws_to_stm32_handle_command_message(const char* command, const char* command_data,
    const char* session_id);
void ws_to_stm32_handle_game_data(const char* data_type, const char* game_data,
    const char* metadata, const char* session_id);
void ws_to_stm32_handle_chat_message(const char* message, const char* metadata);

// Specific StatusMessage handlers
void ws_to_stm32_handle_player_assignment(const char* player_data);
void ws_to_stm32_handle_opponent_connection(const char* opponent_data);
void ws_to_stm32_handle_tile_size_response(const char* response_data);
void ws_to_stm32_handle_session_timeout(const char* session_data);

// Message processing function (called from main websocket_client.c)
void ws_to_stm32_process_server_message(const char* type_str,
    const char* message_id,
    const char* message_text,
    const char* data_type,
    const char* game_data,
    const char* metadata,
    const char* status_type,
    const char* command,
    const char* command_data,
    const char* session_id,
    const char* player_id,
    const char* status_data);

#endif // WEBSOCKET_CLIENT_TO_STM32_H