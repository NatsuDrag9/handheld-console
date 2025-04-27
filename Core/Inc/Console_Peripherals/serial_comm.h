/*
 * serial_comm.h
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_SERIAL_COMM_H_
#define INC_CONSOLE_PERIPHERALS_SERIAL_COMM_H_

#include "Console_Peripherals/Drivers/uart_driver.h"
#include "System/system_conf.h"
#include "Utils/comm_utils.h"
#include <string.h>

/* Communication Settings */
#define MAX_BUFFER_SIZE 256  // Increased to handle larger AT command responses
#define LINE_ENDING '\n'

/* WiFi connection settings */
#define WIFI_SSID "moto g(30)_1128"
#define WIFI_PASSWORD "Moto G30"

/* Protocol States */
typedef enum {
    PROTO_STATE_INIT,          // Initial state
    PROTO_STATE_AT_MODE,       // AT command mode active
    PROTO_STATE_CONNECTED,     // Connection established (WiFi connected)
    PROTO_STATE_DATA_MODE,     // Transferring game data
    PROTO_STATE_ERROR          // Error occurred
} ProtocolState;

/* AT Command States */
typedef enum {
    AT_STATE_IDLE,             // No ongoing AT command
    AT_STATE_WAITING_BASIC,    // Waiting for response to basic AT
    AT_STATE_WAITING_CWMODE,   // Waiting for WiFi mode setting
    AT_STATE_WAITING_CWLAP,    // Waiting for WiFi scan results
    AT_STATE_WAITING_CONNECT,  // Waiting for connection establishment
    AT_STATE_WAITING_IP,       // Waiting for IP info
    AT_STATE_CUSTOM            // Waiting for custom AT command response
} AtCommandState;

/* Function Prototypes */
UART_Status serial_comm_init(void);
UART_Status serial_comm_send_message(const char *message, uint32_t timeout);
bool serial_comm_is_message_ready(void);
void serial_comm_send_debug(const char *message, uint32_t timeout);
void serial_comm_process_messages(void);
bool serial_comm_on(void);
void serial_comm_off(void);

// AT command and wifi specific functions
UART_Status serial_comm_send_at_command(const char *command, uint32_t timeout);
AtCommandState serial_comm_get_at_state(void);
void serial_comm_set_at_state(AtCommandState state);
bool serial_comm_is_at_response_complete(void);
void serial_comm_process_at_response(void);
bool serial_comm_is_wifi_connected(void);

// Game data related functions
UART_Status serial_comm_send_game_data(const char *data, uint16_t length, uint32_t timeout);
void serial_comm_enter_data_mode(void);
void serial_comm_exit_data_mode(void);

// General communication functions
UART_Status serial_comm_reset(void);

#endif /* INC_CONSOLE_PERIPHERALS_SERIAL_COMM_H_ */
