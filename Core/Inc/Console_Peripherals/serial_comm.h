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

/* Communication Settings */
#define MAX_BUFFER_SIZE 128
#define LINE_ENDING '\n'

/* Protocol States */
typedef enum {
    PROTO_STATE_INIT,          // Initial state
    PROTO_STATE_HELLO_SENT,    // STM32 sent HELLO/READY
    PROTO_STATE_CONNECTED,     // Handshake complete, connection established
    PROTO_STATE_ERROR          // Error occurred
} ProtocolState;

/* Function Prototypes */
UART_Status serial_comm_init(void);
UART_Status serial_comm_send_message(const char *message, uint32_t timeout);
bool serial_comm_is_message_ready(void);
// Send a debug message to the PC via ST-Link Virtual COM port
void serial_comm_send_debug(const char *message, uint32_t timeout);
// Process messages received from ESP32. Should be called from main loop when a message is received
void serial_comm_process_messages(void);
bool serial_comm_is_connected(void);

UART_Status serial_comm_ping(void);
UART_Status serial_comm_send_command(const char *command, uint32_t timeout);
UART_Status serial_comm_reset(void);
void serial_comm_process_command_response(const char* response);


#endif /* INC_CONSOLE_PERIPHERALS_SERIAL_COMM_H_ */
