/*
 * serial_comm_core.c
 *
 *  Created on: Jul 16, 2025
 *      Author: rohitimandi
 */


#include "Console_Peripherals/Hardware/serial_comm_core.h"
#include "Utils/debug_conf.h"

/* Hardware communication handles */
static circular_buffer_t rx_buffer;
static message_queue_t msg_queue;
static message_parser_t parser;
static comm_stats_t stats;

/* Message storage */
static uart_message_t message_storage[MESSAGE_QUEUE_SIZE];
static uint8_t parse_buffer[sizeof(uart_message_t)];

/* Callback for processed messages */
static uart_message_received_callback_t message_callback = NULL;

/* Private function prototypes */
static void UART_RxCallback(uint8_t data);
static void parse_incoming_data(void);

/* UART receive callback - called from interrupt (keep minimal!) */
static void UART_RxCallback(uint8_t data)
{
    // Simply store data in circular buffer - keep ISR minimal
    circular_buffer_put(&rx_buffer, data);
}

/* Parse incoming data from circular buffer */
static void parse_incoming_data(void)
{
    uint8_t data;

    while (circular_buffer_get(&rx_buffer, &data)) {
        if (message_parser_add_byte(&parser, data, MSG_START_BYTE)) {
            stats.bytes_received++;

            // Check if we have a complete message
            if (message_parser_is_complete(&parser)) {
                uart_message_t* msg = (uart_message_t*)message_parser_get_buffer(&parser);

                if (hardware_serial_validate_message(msg)) {
                    if (message_queue_put(&msg_queue, msg)) {
                        DEBUG_PRINTF(false, "Serial Comm Core: Message parsed and queued: type=0x%02X\r\n", msg->msg_type);
                        stats.messages_parsed++;
                    }
                    else {
                        DEBUG_PRINTF(false, "Serial Comm Core: Message queue full, dropping message\r\n");
                    }
                }
                else {
                    DEBUG_PRINTF(false, "Serial Comm Core: Message validation failed\r\n");
                }
                message_parser_reset(&parser);
            }
        }
    }
}

/* Calculate checksum to match ESP32 */
uint8_t hardware_serial_calculate_checksum(const uart_message_t* msg)
{
    uint8_t checksum = 0;
    const uint8_t* data = (const uint8_t*)msg;

    // Calculate for all bytes except checksum and end_byte (last 2 bytes)
    size_t total_length = sizeof(uart_message_t);
    for (size_t i = 0; i < total_length - 2; i++) {
        checksum ^= data[i];
    }

    return checksum;
}

/* Validate received message */
bool hardware_serial_validate_message(const uart_message_t* msg)
{
    /* Check start and end bytes */
    if (msg->start_byte != MSG_START_BYTE || msg->end_byte != MSG_END_BYTE) {
        DEBUG_PRINTF(false, "Serial Comm Core: Invalid delimiters: start=0x%02X, end=0x%02X\r\n",
            msg->start_byte, msg->end_byte);
        return false;
    }

    /* Check message type */
    if (msg->msg_type < MSG_TYPE_DATA || msg->msg_type > MSG_TILE_SIZE_VALIDATION) {
        DEBUG_PRINTF(false, "Serial Comm Core: Invalid message type: 0x%02X\r\n", msg->msg_type);
        return false;
    }

    /* Check length */
    if (msg->length > MAX_PAYLOAD_SIZE) {
        DEBUG_PRINTF(false, "Serial Comm Core: Invalid message length: %d\r\n", msg->length);
        return false;
    }

    /* Calculate and verify checksum */
    uint8_t calculated_checksum = hardware_serial_calculate_checksum(msg);
    if (msg->checksum != calculated_checksum) {
        DEBUG_PRINTF(false, "Serial Comm Core: Checksum mismatch: expected=0x%02X, calculated=0x%02X\r\n",
            msg->checksum, calculated_checksum);
        stats.parse_errors++;
        return false;
    }

    return true;
}

/* Hardware UART initialization */
UART_Status hardware_serial_init(void)
{
    /* Initialize all communication utilities */
    circular_buffer_init(&rx_buffer);
    message_queue_init(&msg_queue, message_storage, MESSAGE_QUEUE_SIZE, sizeof(uart_message_t));
    message_parser_init(&parser, parse_buffer, sizeof(uart_message_t));
    comm_stats_init(&stats);

    /* Initialize callback to NULL */
    message_callback = NULL;

    /* Initialize UART driver */
    UART_Status status = UART_Init();
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "Serial Comm Core: UART initialization failed: %d\r\n", status);
        return status;
    }

    /* Register callback for interrupt-based reception */
    status = UART_RegisterRxCallback(UART_PORT_2, UART_RxCallback);
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "Serial Comm Core: UART callback registration failed: %d\r\n", status);
        return status;
    }

    /* Enable Rx interrupt */
    status = UART_EnableRxInterrupt(UART_PORT_2);
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "Serial Comm Core: UART interrupt enable failed: %d\r\n", status);
        return status;
    }

    DEBUG_PRINTF(false, "Serial Comm Core: UART Communication Hardware Initialized\r\n");
    return UART_OK;
}

/* Hardware UART deinitialization */
UART_Status hardware_serial_deinit(void)
{
    UART_DisableRxInterrupt(UART_PORT_2);

    /* Clear callback */
    message_callback = NULL;

    /* Clear all buffers */
    circular_buffer_flush(&rx_buffer);
    message_queue_flush(&msg_queue);
    message_parser_reset(&parser);

    DEBUG_PRINTF(false, "Serial Comm Core: UART Communication Hardware Deinitialized\r\n");
    return UART_OK;
}

/* Send raw message over UART */
UART_Status hardware_serial_send_message(const uart_message_t* msg)
{
    UART_Status status = UART_SendBuffer(UART_PORT_2, (uint8_t*)msg,
        sizeof(uart_message_t), 1000);

    if (status == UART_OK) {
        stats.bytes_sent += sizeof(uart_message_t);
        DEBUG_PRINTF(false, "Serial Comm Core: Message sent successfully\r\n");
    }
    else {
        DEBUG_PRINTF(false, "Serial Comm Core: Failed to send message, status: %d\r\n", status);
    }

    return status;
}

/* Check if messages are ready for processing */
bool hardware_serial_is_message_ready(void)
{
    /* Parse any available data from circular buffer */
    parse_incoming_data();

    /* Check if we have any parsed messages ready */
    return !message_queue_is_empty(&msg_queue);
}

/* Process incoming messages and call callback */
void hardware_serial_process_incoming(void)
{
    uart_message_t msg;

    /* Process all available messages */
    while (message_queue_get(&msg_queue, &msg)) {
        if (message_callback) {
            message_callback(&msg);
        }
    }
}

/* Register callback for received messages */
void hardware_serial_register_callback(uart_message_received_callback_t callback)
{
    message_callback = callback;
    DEBUG_PRINTF(false, "Serial Comm Core: Message callback registered\r\n");
}

/* Get hardware statistics */
void hardware_serial_get_stats(uint32_t* bytes_sent, uint32_t* bytes_received,
                               uint32_t* messages_parsed, uint32_t* parse_errors)
{
    if (bytes_sent) *bytes_sent = stats.bytes_sent;
    if (bytes_received) *bytes_received = stats.bytes_received;
    if (messages_parsed) *messages_parsed = stats.messages_parsed;
    if (parse_errors) *parse_errors = stats.parse_errors;
}

/* Reset hardware buffers */
void hardware_serial_reset_buffers(void)
{
    circular_buffer_flush(&rx_buffer);
    message_queue_flush(&msg_queue);
    message_parser_reset(&parser);
    DEBUG_PRINTF(false, "Serial Comm Core: Buffers reset\r\n");
}
