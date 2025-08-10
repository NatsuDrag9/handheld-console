/*
 * serial_comm_callbacks.c
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */

#include "Communication/serial_comm_callbacks.h"
#include "Utils/debug_conf.h"
#include "stdbool.h"

/* Registered callback functions */
static game_data_received_callback_t game_data_callback = NULL;
static chat_message_received_callback_t chat_message_callback = NULL;
static command_received_callback_t command_callback = NULL;
static status_received_callback_t status_callback = NULL;
static connection_message_callback_t connection_message_callback = NULL;

/* Callback system initialization */
void callbacks_init(void) {
    /* Initialize all callbacks to NULL */
    game_data_callback = NULL;
    chat_message_callback = NULL;
    command_callback = NULL;
    status_callback = NULL;
    connection_message_callback = NULL;

    DEBUG_PRINTF(false, "CALLBACKS: Callback system initialized\r\n");
}

void callbacks_deinit(void) {
    /* Clear all callbacks */
    game_data_callback = NULL;
    chat_message_callback = NULL;
    command_callback = NULL;
    status_callback = NULL;
    connection_message_callback = NULL;

    DEBUG_PRINTF(false, "CALLBACKS: Callback system deinitialized\r\n");
}

/* Callback registration functions */
void callbacks_register_game_data(game_data_received_callback_t callback) {
    game_data_callback = callback;
    DEBUG_PRINTF(false, "CALLBACKS: Game data callback registered\r\n");
}

void callbacks_register_chat_message(chat_message_received_callback_t callback) {
    chat_message_callback = callback;
    DEBUG_PRINTF(false, "CALLBACKS: Chat message callback registered\r\n");
}

void callbacks_register_command(command_received_callback_t callback) {
    command_callback = callback;
    DEBUG_PRINTF(false, "CALLBACKS: Command callback registered\r\n");
}

void callbacks_register_status(status_received_callback_t callback) {
    status_callback = callback;
    DEBUG_PRINTF(false, "CALLBACKS: Status callback registered\r\n");
}

void callbacks_register_connection_message(connection_message_callback_t callback) {
    connection_message_callback = callback;
    DEBUG_PRINTF(false, "CALLBACKS: Connection message callback registered\r\n");
}

/* Message handling functions - called by protocol layer */
void callbacks_handle_game_data(const uart_game_data_t* game_data) {
    if (game_data_callback) {
        game_data_callback(game_data);
    } else {
        DEBUG_PRINTF(false, "CALLBACKS: No game data callback registered\r\n");
    }
}

void callbacks_handle_chat_message(const uart_chat_message_t* chat_message) {
    if (chat_message_callback) {
        chat_message_callback(chat_message);
    } else {
        DEBUG_PRINTF(false, "CALLBACKS: No chat message callback registered\r\n");
    }
}

void callbacks_handle_command(const uart_command_t* command) {
    if (command_callback) {
        command_callback(command);
    } else {
        DEBUG_PRINTF(false, "CALLBACKS: No command callback registered\r\n");
    }
}

void callbacks_handle_status(const uart_status_t* status) {
    if (status_callback) {
        status_callback(status);
    } else {
        DEBUG_PRINTF(false, "CALLBACKS: No status callback registered\r\n");
    }
}

void callbacks_handle_connection_message(const uart_connection_message_t* connection_msg) {
    if (connection_message_callback) {
        connection_message_callback(connection_msg);
    } else {
        DEBUG_PRINTF(false, "CALLBACKS: No connection message callback registered\r\n");
    }
}
