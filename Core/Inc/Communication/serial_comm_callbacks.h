/*
 * serial_comm_callbacks.h
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_COMMUNICATION_SERIAL_COMM_CALLBACKS_H_
#define INC_COMMUNICATION_SERIAL_COMM_CALLBACKS_H_

#include "serial_comm_protocol.h"

/* Callback function types */
typedef void (*game_data_received_callback_t)(const uart_game_data_t* game_data);
typedef void (*chat_message_received_callback_t)(const uart_chat_message_t* chat_message);
typedef void (*command_received_callback_t)(const uart_command_t* command);
typedef void (*status_received_callback_t)(const uart_status_t* status);
typedef void (*connection_message_callback_t)(const uart_connection_message_t* connection_msg);

/* Callback system initialization */
void callbacks_init(void);
void callbacks_deinit(void);

/* Callback registration functions */
void callbacks_register_game_data(game_data_received_callback_t callback);
void callbacks_register_chat_message(chat_message_received_callback_t callback);
void callbacks_register_command(command_received_callback_t callback);
void callbacks_register_status(status_received_callback_t callback);
void callbacks_register_connection_message(connection_message_callback_t callback);

/* Message handling functions - called by protocol layer */
void callbacks_handle_game_data(const uart_game_data_t* game_data);
void callbacks_handle_chat_message(const uart_chat_message_t* chat_message);
void callbacks_handle_command(const uart_command_t* command);
void callbacks_handle_status(const uart_status_t* status);
void callbacks_handle_connection_message(const uart_connection_message_t* connection_msg);



#endif /* INC_COMMUNICATION_SERIAL_COMM_CALLBACKS_H_ */
