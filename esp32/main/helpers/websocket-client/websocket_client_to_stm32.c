#include "websocket_client_to_stm32.h"
#include "esp_log.h"

static const char* TAG = "WS_TO_STM32";

// WebSocket → STM32 conversion functions
void ws_to_stm32_handle_connection_message(const char* message_id, const char* message_text) {
    ESP_LOGI(TAG, "Handling server connection message: id=%s, message=%s",
        message_id ? message_id : "null", message_text ? message_text : "null");

    // Forward connection info to STM32 as connection_message
    uart_send_connection_message(message_id ? message_id : "", message_text ? message_text : "Server connected");
}

void ws_to_stm32_handle_chat_message(const char* message, const char* metadata) {
    ESP_LOGI(TAG, "Handling server chat message: %s", message ? message : "null");

    // Forward chat message to STM32
    uart_send_chat_message(message ? message : "", "server", "websocket");
}

void ws_to_stm32_handle_status_message(const char* status_type, const char* message_data,
    const char* additional_data) {
    ESP_LOGI(TAG, "Handling server status message: type=%s, data=%s",
        status_type ? status_type : "null", message_data ? message_data : "null");

    // Route to specific status handlers based on type
    if (status_type) {
        if (strcmp(status_type, "player_assignment") == 0) {
            ws_to_stm32_handle_player_assignment(additional_data);
        }
        else if (strcmp(status_type, "opponent_connected") == 0) {
            ws_to_stm32_handle_opponent_connection(additional_data);
        }
        else if (strcmp(status_type, "opponent_disconnected") == 0) {
            uart_send_status(SYSTEM_STATUS_OPPONENT_DISCONNECTED, 0, message_data ? message_data : "Opponent disconnected");
        }
        else if (strcmp(status_type, "tile_size_response") == 0) {
            ws_to_stm32_handle_tile_size_response(additional_data);
        }
        else if (strcmp(status_type, "session_timeout") == 0) {
            ws_to_stm32_handle_session_timeout(additional_data);
        }
        else {
            // Forward unknown status as general game ready status
            uart_send_status(SYSTEM_STATUS_GAME_READY, 0, message_data ? message_data : "Game status update");
        }
    }
}

void ws_to_stm32_handle_command_message(const char* command, const char* command_data,
    const char* session_id) {
    ESP_LOGI(TAG, "Handling server command: %s, data=%s, session=%s",
        command ? command : "null",
        command_data ? command_data : "null",
        session_id ? session_id : "null");

    // Convert server commands to appropriate STM32 status messages
    if (command) {
        if (strcmp(command, "game_start") == 0) {
            uart_send_status(SYSTEM_STATUS_GAME_ACTIVE, 0, "Game started");
            // Also forward as command
            uart_send_command(command, command_data);
        }
        else if (strcmp(command, "game_end") == 0) {
            uart_send_status(SYSTEM_STATUS_GAME_ENDED, 0, "Game ended");
            // Also forward as command
            uart_send_command(command, command_data);
        }
        else {
            // Forward other commands directly
            uart_send_command(command, command_data);
        }
    }
}

void ws_to_stm32_handle_game_data(const char* data_type, const char* game_data,
    const char* metadata, const char* session_id) {
    ESP_LOGI(TAG, "Handling server game data: type=%s, data=%s",
        data_type ? data_type : "null", game_data ? game_data : "null");

    // Forward game data to STM32
    uart_send_game_data(data_type ? data_type : "unknown",
        game_data ? game_data : "",
        metadata ? metadata : "");
}

// Specific StatusMessage handlers
void ws_to_stm32_handle_player_assignment(const char* player_data) {
    ESP_LOGI(TAG, "Handling player assignment: %s", player_data ? player_data : "null");

    // Format: "player_id:session_id:player_count:color"
    uart_send_status(SYSTEM_STATUS_PLAYER_ASSIGNMENT, 0, player_data ? player_data : "1:default_session:2:blue");
}

void ws_to_stm32_handle_opponent_connection(const char* opponent_data) {
    ESP_LOGI(TAG, "Handling opponent connection: %s", opponent_data ? opponent_data : "null");

    // Format: "player_id:session_id:player_count:color"
    uart_send_status(SYSTEM_STATUS_OPPONENT_CONNECTED, 0, opponent_data ? opponent_data : "Opponent connected");
}

void ws_to_stm32_handle_tile_size_response(const char* response_data) {
    ESP_LOGI(TAG, "Handling tile size response: %s", response_data ? response_data : "null");

    uart_send_status(SYSTEM_STATUS_TILE_SIZE_RESPONSE, 0, response_data ? response_data : "Failed to receive tile size from server");
}

void ws_to_stm32_handle_session_timeout(const char* session_data) {
    ESP_LOGI(TAG, "Handling session timeout: %s", session_data ? session_data : "null");

    // session_data now contains just the sessionId from SessionTimeoutData
    char timeout_message[64];
    if (session_data && strlen(session_data) > 0) {
        snprintf(timeout_message, sizeof(timeout_message), "Session %s timed out", session_data);
    }
    else {
        snprintf(timeout_message, sizeof(timeout_message), "Session timed out");
    }

    uart_send_status(SYSTEM_STATUS_SESSION_TIMEOUT, 1, timeout_message);
}

// Main message processing function (called from websocket_client.c)
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
    const char* status_data) {

    if (!type_str) {
        ESP_LOGW(TAG, "No message type provided");
        return;
    }

    ESP_LOGI(TAG, "Processing server message type: %s", type_str);

    if (strcmp(type_str, "connection") == 0) {
        ws_to_stm32_handle_connection_message(message_id, message_text);
    }
    else if (strcmp(type_str, "status") == 0) {
        ws_to_stm32_handle_status_message(status_type, message_text, status_data);
    }
    else if (strcmp(type_str, "command") == 0) {
        ws_to_stm32_handle_command_message(command, command_data, session_id);
    }
    else if (strcmp(type_str, "game_data_message") == 0 ||
        strcmp(type_str, "game_data") == 0 ||
        strcmp(type_str, "player_action") == 0 ||
        strcmp(type_str, "game_state") == 0) {
        ws_to_stm32_handle_game_data(data_type, game_data, metadata, session_id);
    }
    else if (strcmp(type_str, "chat_message") == 0) {
        ws_to_stm32_handle_chat_message(message_text, metadata);
    }
    else if (strcmp(type_str, "ping") == 0) {
        ESP_LOGI(TAG, "Received ping message - connection alive");
        // No need to forward ping to STM32
    }
    else if (strcmp(type_str, "echo") == 0) {
        ESP_LOGI(TAG, "Received echo message");
        // Echo messages are for acknowledgment only - no forwarding needed
    }
    else if (strcmp(type_str, "broadcast") == 0) {
        ESP_LOGI(TAG, "Received broadcast message: %s", message_text ? message_text : "null");
        // Could forward as status or ignore based on requirements
    }
    else {
        ESP_LOGW(TAG, "Unknown message type from server: %s", type_str);
    }
}