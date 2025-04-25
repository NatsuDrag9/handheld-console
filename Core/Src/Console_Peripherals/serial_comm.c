/*
 * serial_comm.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/serial_comm.h"
#include "Utils/debug_conf.h"

/* Private variables */
static uint8_t rx_buffer[MAX_BUFFER_SIZE];
static uint16_t rx_index = 0;
static bool message_complete = false;
static ProtocolState current_state = PROTO_STATE_INIT;
static uint32_t last_activity_time = 0;
static const uint32_t TIMEOUT_MS = 5000; // 5 second timeout

/* Connection status */
static bool is_connected = false;

/* Private function prototypes */
static void UART_RxCallback(uint8_t data);

UART_Status serial_comm_init(void)
{
    /* Initialize message buffer */
    memset(rx_buffer, 0, MAX_BUFFER_SIZE);
    rx_index = 0;
    message_complete = false;
    current_state = PROTO_STATE_INIT;
    is_connected = false;

    /* Initialize UART driver */
    UART_Status status = UART_Init();
    if (status != UART_OK) {
    	DEBUG_PRINTF(false, "UART initialization failed: %d\n", status);
    	Error_Handler();
    }

    /* Register callback for interrupt-based reception */
    status = UART_RegisterRxCallback(UART_PORT_2, UART_RxCallback);
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "UART callback registration failed: %d\n", status);
        Error_Handler();
    }

    /* Enable Rx interrupt */
    status = UART_EnableRxInterrupt(UART_PORT_2);
    if (status != UART_OK) {
        DEBUG_PRINTF(false, "UART interrupt enable failed: %d\n", status);
        Error_Handler();
    }

    /* Record start time */
    last_activity_time = HAL_GetTick();

    /* Send debug message */
    serial_comm_send_debug("STM32 UART Communication Initialized\r\n", 100);

    return UART_OK;
}

bool serial_comm_is_message_ready(void)
{
    /* Check for timeout in non-connected states */
    uint32_t current_time = HAL_GetTick();
    if (current_state != PROTO_STATE_CONNECTED &&
        current_time - last_activity_time > TIMEOUT_MS) {
        /* Timeout occurred - retry handshake */
        serial_comm_send_debug("Handshake timeout. Retrying...\r\n", 100);

        /* Send initial handshake message */
        serial_comm_send_message("STM32_READY\r\n", 100);
        current_state = PROTO_STATE_HELLO_SENT;

        last_activity_time = current_time;
    }

    /* In initial state, send handshake */
    if (current_state == PROTO_STATE_INIT) {
        /* Send initial handshake message */
        serial_comm_send_message("STM32_READY\r\n", 100);
        current_state = PROTO_STATE_HELLO_SENT;
        last_activity_time = HAL_GetTick();
    }

    return message_complete;
}

void serial_comm_process_messages(void)
{
    if (!message_complete) {
        return;
    }

    /* Print received message to debug port */
    serial_comm_send_debug("Received from ESP32: ", 100);
    serial_comm_send_debug((char*)rx_buffer, 100);
    serial_comm_send_debug("\r\n", 100);

    /* Process message based on current state */
    switch (current_state) {
        case PROTO_STATE_INIT:
            if (strncmp((char*)rx_buffer, "ESP32_READY", 11) == 0) {
                /* ESP32 initiated handshake */
                serial_comm_send_message("STM32_READY\r\n", 100);
                current_state = PROTO_STATE_HELLO_SENT;
                last_activity_time = HAL_GetTick();
            }
            break;

        case PROTO_STATE_HELLO_SENT:
            if (strncmp((char*)rx_buffer, "ESP32_ACK", 9) == 0) {
                /* ESP32 acknowledged our hello */
                current_state = PROTO_STATE_CONNECTED;
                is_connected = true;
                serial_comm_send_debug("Handshake successful. Connection established.\r\n", 100);
            }
            else if (strncmp((char*)rx_buffer, "ESP32_READY", 11) == 0) {
                /* ESP32 sent READY again (could be a retry) - respond */
                serial_comm_send_message("STM32_READY\r\n", 100);
                last_activity_time = HAL_GetTick();
            }
            break;

        case PROTO_STATE_CONNECTED:
            /* In connected state, handle commands or special messages */
            if (strncmp((char*)rx_buffer, "ESP32_PONG", 10) == 0) {
                /* Response to our ping - no action needed */
                serial_comm_send_debug("Ping successful\r\n", 100);
            }
            else if (strncmp((char*)rx_buffer, "ESP32_RESP_", 11) == 0) {
                /* Process response to a command */
                serial_comm_process_command_response((char*)(rx_buffer + 11));
            }
            else if (strncmp((char*)rx_buffer, "ESP32_ECHO: ", 12) == 0) {
                /* Echo received - no response needed */
                serial_comm_send_debug("Echo received from ESP32\r\n", 100);
            }
            break;

        case PROTO_STATE_ERROR:
            /* In error state, reset if ESP32 is ready to start over */
            if (strncmp((char*)rx_buffer, "ESP32_READY", 11) == 0) {
                current_state = PROTO_STATE_INIT;
                is_connected = false;
                serial_comm_process_messages(); /* Process this message again in INIT state */
            }
            break;

        default:
            /* Should not reach here, but reset if we do */
            current_state = PROTO_STATE_INIT;
            is_connected = false;
            break;
    }

    /* Update activity time on any message */
    last_activity_time = HAL_GetTick();

    /* Reset buffer for next message */
    rx_index = 0;
    message_complete = false;
}

UART_Status serial_comm_send_message(const char *message, uint32_t timeout)
{
    return UART_SendString(UART_PORT_2, message, timeout);
}

void serial_comm_send_debug(const char *message, uint32_t timeout)
{
    DEBUG_PRINTF(false, message);
}

bool serial_comm_is_connected(void)
{
    return is_connected;
}

UART_Status serial_comm_ping(void)
{
    if (!is_connected) {
        return UART_ERROR;
    }

    return serial_comm_send_message("STM32_PING\r\n", 100);
}

UART_Status serial_comm_send_command(const char *command, uint32_t timeout)
{
    if (!is_connected) {
        return UART_ERROR;
    }

    char command_buffer[MAX_BUFFER_SIZE];
    snprintf(command_buffer, MAX_BUFFER_SIZE, "STM32_CMD_%s\r\n", command);

    return serial_comm_send_message(command_buffer, timeout);
}

UART_Status serial_comm_reset(void)
{
    current_state = PROTO_STATE_INIT;
    is_connected = false;
    rx_index = 0;
    message_complete = false;

    /* Send reset command to ESP32 */
    UART_Status status = serial_comm_send_message("STM32_RESET\r\n", 100);
    if (status != UART_OK) {
        return status;
    }

    serial_comm_send_debug("Communication reset initiated\r\n", 100);
    last_activity_time = HAL_GetTick();

    return UART_OK;
}

void serial_comm_process_command_response(const char* response)
{
    /* Log command response */
    char debug_msg[MAX_BUFFER_SIZE];
    snprintf(debug_msg, MAX_BUFFER_SIZE, "Command response: %s\r\n", response);
    serial_comm_send_debug(debug_msg, 100);

    /* Add specific response handling here if needed */
}

static void UART_RxCallback(uint8_t data)
{
    /* Process received byte */
    if (data == LINE_ENDING || data == '\r') {
        /* End of message - null terminate the string */
        if (rx_index < MAX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index] = '\0';
            message_complete = true;
        }
    } else {
        /* Store byte in buffer if there's space */
        if (rx_index < MAX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = data;
        }
    }
}
