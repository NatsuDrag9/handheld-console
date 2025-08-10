/*
 * serial_comm_core.h
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_HARDWARE_SERIAL_COMM_CORE_H_
#define INC_CONSOLE_PERIPHERALS_HARDWARE_SERIAL_COMM_CORE_H_

#include <Console_Peripherals/Hardware/Drivers/uart_driver.h>
#include "System/system_conf.h"
#include "Utils/comm_utils.h"
#include <string.h>
#include "../../Communication/serial_comm_types.h"

 /* Message Protocol - Hardware layer constants */
#define MSG_START_BYTE 0xAA
#define MSG_END_BYTE   0x55
#define MAX_PAYLOAD_SIZE 250

/* Message Types - Must match ESP32 uart_comm.h */
//typedef enum {
//    MSG_TYPE_DATA = 0x01,      // For game data (repurposed)
//    MSG_TYPE_COMMAND = 0x02,   // Commands between STM32<->ESP32
//    MSG_TYPE_STATUS = 0x03,    // Status updates
//    MSG_TYPE_CONNECTION = 0x04,    // Configuration messages
//    MSG_TYPE_ACK = 0x05,       // Acknowledgment
//    MSG_TYPE_NACK = 0x06,      // Negative acknowledgment
//    MSG_TYPE_HEARTBEAT = 0x07, // Keep-alive
//    MSG_TYPE_CHAT = 0x08,       // Chat messages
//    MSG_TILE_SIZE_VALIDATION = 0x09, // For TileSizeValidationMessage
//} MessageType;
//
//#pragma pack(push, 1)
///* Message Structure - Hardware frame format */
//typedef struct {
//    uint8_t start_byte;     // 0xAA - Start delimiter
//    uint8_t msg_type;       // Message type
//    uint8_t length;         // Data length (0-250)
//    uint8_t data[250];      // Payload data
//    uint8_t checksum;       // Simple checksum
//    uint8_t end_byte;       // 0x55 - End delimiter
//} uart_message_t;
//#pragma pack(pop)

/* Hardware UART functions */
UART_Status hardware_serial_init(void);
UART_Status hardware_serial_deinit(void);
UART_Status hardware_serial_send_message(const uart_message_t* msg);
bool hardware_serial_is_message_ready(void);
void hardware_serial_process_incoming(void);
void hardware_serial_register_callback(uart_message_received_callback_t callback);

/* Utility functions */
uint8_t hardware_serial_calculate_checksum(const uart_message_t* msg);
bool hardware_serial_validate_message(const uart_message_t* msg);
void hardware_serial_get_stats(uint32_t* bytes_sent, uint32_t* bytes_received,
    uint32_t* messages_parsed, uint32_t* parse_errors);
void hardware_serial_reset_buffers(void);

#endif /* INC_CONSOLE_PERIPHERALS_HARDWARE_SERIAL_COMM_CORE_H_ */
