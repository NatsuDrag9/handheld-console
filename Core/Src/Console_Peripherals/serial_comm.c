/*
 * serial_comm.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 *  Modified: To support AT commands and game data transmission
 */

#include "Console_Peripherals/serial_comm.h"
#include "Utils/debug_conf.h"

/* Private variables */
static uint8_t rx_buffer[MAX_BUFFER_SIZE];
static uint16_t rx_index = 0;
static bool message_complete = false;
static ProtocolState current_state = PROTO_STATE_INIT;
static AtCommandState at_state = AT_STATE_IDLE;
static uint32_t last_activity_time = 0;
static uint32_t last_at_command_time = 0;
static const uint32_t AT_COMMAND_TIMEOUT_MS = 3000; // 3 second AT command timeout

/* Connection status */
static bool is_serial_connected = false;
static bool at_response_complete = false;
static bool is_wifi_connected = false;

/* Private function prototypes */
static void UART_RxCallback(uint8_t data);
static bool check_for_at_response_end(void);

UART_Status serial_comm_init(void)
{
    /* Initialize message buffer */
    memset(rx_buffer, 0, MAX_BUFFER_SIZE);
    rx_index = 0;
    message_complete = false;
    at_response_complete = false;
    current_state = PROTO_STATE_INIT;
    at_state = AT_STATE_IDLE;
    is_serial_connected = false;

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
    last_activity_time = get_current_ms();
    last_at_command_time = get_current_ms();

    /* Send debug message */
    serial_comm_send_debug("STM32 UART Communication Initialized\r\n", 100);

    return UART_OK;
}

bool serial_comm_is_message_ready(void)
{
    uint32_t current_time = get_current_ms();

    /* Check for timeout in AT command state */
    if (at_state != AT_STATE_IDLE &&
        current_time - last_at_command_time > AT_COMMAND_TIMEOUT_MS) {
        /* AT command timeout */
        serial_comm_send_debug("AT command timeout occurred\r\n", 100);

        /* Reset AT state but keep protocol state */
        at_state = AT_STATE_IDLE;
    }

    /* In initial state, start AT command sequence */
    if (current_state == PROTO_STATE_INIT && at_state == AT_STATE_IDLE) {
        /* Start with basic AT command */
        serial_comm_send_debug("Starting AT sequence...\r\n", 100);
        serial_comm_send_at_command("AT", 100);
        current_state = PROTO_STATE_AT_MODE;
        at_state = AT_STATE_WAITING_BASIC;
        last_at_command_time = current_time;
    }

    return message_complete || at_response_complete;
}

void serial_comm_process_messages(void)
{
    if (!message_complete && !at_response_complete) {
        return;
    }

    /* Print received message to debug port */
    serial_comm_send_debug("Received: ", 100);
    serial_comm_send_debug((char*)rx_buffer, 100);
    serial_comm_send_debug("\r\n", 100);

    /* Process message based on current state */
    switch (current_state) {
        case PROTO_STATE_INIT:
            /* Only AT commands should trigger in init state */
            serial_comm_process_at_response();
            break;

        case PROTO_STATE_AT_MODE:
            /* Handle AT command responses */
            serial_comm_process_at_response();
            break;

        case PROTO_STATE_CONNECTED:
            /* Process messages when WiFi is connected */
            if (strstr((char*)rx_buffer, "+IPD")) {
                /* Received data from network */
                serial_comm_send_debug("Received network data\r\n", 100);
                /* Process the incoming network data here */
            }
            else if (at_response_complete) {
                /* Process AT command response when already connected */
                serial_comm_process_at_response();
            }
            is_serial_connected = true;
            break;

        case PROTO_STATE_DATA_MODE:
            /* Process data mode messages (game data) */
            /* Add game data specific handling here */
            serial_comm_send_debug("Processing game data\r\n", 100);
            break;

        case PROTO_STATE_ERROR:
            /* In error state, try to recover with a basic AT command */
            serial_comm_send_at_command("AT", 100);
            at_state = AT_STATE_WAITING_BASIC;
            current_state = PROTO_STATE_AT_MODE;
            break;

        default:
            /* Should not reach here, but reset if we do */
            current_state = PROTO_STATE_INIT;
            at_state = AT_STATE_IDLE;
            is_serial_connected = false;
            break;
    }

    /* Update activity time on any message */
    last_activity_time = get_current_ms();

    /* Reset buffer for next message */
    rx_index = 0;
    message_complete = false;
    at_response_complete = false;
}

UART_Status serial_comm_send_message(const char *message, uint32_t timeout)
{
    return UART_SendString(UART_PORT_2, message, timeout);
}

void serial_comm_send_debug(const char *message, uint32_t timeout)
{
    DEBUG_PRINTF(false, message);
}

bool serial_comm_is_serial_connected(void)
{
    return is_serial_connected;
}

bool serial_comm_is_wifi_connected(void)
{
    return is_wifi_connected;
}


UART_Status serial_comm_send_at_command(const char *command, uint32_t timeout)
{
    char at_command[MAX_BUFFER_SIZE];

    /* Format the AT command with proper line ending */
    snprintf(at_command, MAX_BUFFER_SIZE, "%s\r\n", command);

    /* Log the command being sent */
    serial_comm_send_debug("Sending AT command: ", 100);
    serial_comm_send_debug(at_command, 100);

    /* Update last command time */
    last_at_command_time = get_current_ms();

    /* Send the command */
    return serial_comm_send_message(at_command, timeout);
}

AtCommandState serial_comm_get_at_state(void)
{
    return at_state;
}

void serial_comm_set_at_state(AtCommandState state)
{
    at_state = state;
    last_at_command_time = get_current_ms();
}

bool serial_comm_is_at_response_complete(void)
{
    return at_response_complete;
}

void serial_comm_process_at_response(void)
{
    /* Process based on current AT command state */
    if (strstr((char*)rx_buffer, "OK")) {
        /* Command succeeded */
        serial_comm_send_debug("AT command succeeded\r\n", 100);

        switch (at_state) {
            case AT_STATE_WAITING_BASIC:
                /* Basic AT succeeded, set WiFi mode */
                serial_comm_send_at_command("AT+CWMODE=1", 100);
                at_state = AT_STATE_WAITING_CWMODE;
                break;

            case AT_STATE_WAITING_CWMODE:
            	 /* WiFi mode set, check if we need to connect to WiFi */
            	if (!is_wifi_connected) {
            		/* Connect to WiFi using the defined SSID and password */
            		char connect_cmd[128];
            		snprintf(connect_cmd, sizeof(connect_cmd),
            				"AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASSWORD);
            		serial_comm_send_at_command(connect_cmd, 100);
            		at_state = AT_STATE_WAITING_CONNECT;
            	} else {
            		/* Already connected, proceed to scan */
            		serial_comm_send_at_command("AT+CWLAP", 100);
            		at_state = AT_STATE_WAITING_CWLAP;
            	}
                break;

            case AT_STATE_WAITING_CWLAP:
                /* Scan completed successfully */
                serial_comm_send_debug("WiFi scan completed\r\n", 100);
                at_state = AT_STATE_IDLE;

                /* Process the scan results here if needed */
                /* CWLAP results should have been captured in previous messages */
                break;

            case AT_STATE_WAITING_CONNECT:
                /* Connection successful */
                serial_comm_send_debug("WiFi connection established\r\n", 100);
                is_wifi_connected = true;
                current_state = PROTO_STATE_CONNECTED;

                /* Get IP address */
                serial_comm_send_at_command("AT+CIFSR", 100);
                at_state = AT_STATE_WAITING_IP;
                break;

            case AT_STATE_WAITING_IP:
                /* Received IP info */
                serial_comm_send_debug("IP address acquired\r\n", 100);
                at_state = AT_STATE_IDLE;
                break;

            case AT_STATE_CUSTOM:
                /* Custom AT command completed */
                serial_comm_send_debug("Custom AT command completed\r\n", 100);
                at_state = AT_STATE_IDLE;
                break;

            default:
                at_state = AT_STATE_IDLE;
                break;
        }
    }
    else if (strstr((char*)rx_buffer, "ERROR") || strstr((char*)rx_buffer, "FAIL")) {
        /* Command failed - get human readable error message */
    	const char* error_message = translate_at_error((char*)rx_buffer);
    	char debug_msg[MAX_BUFFER_SIZE];
    	snprintf(debug_msg, MAX_BUFFER_SIZE, "AT command failed: %s\r\n", error_message);
    	serial_comm_send_debug(debug_msg, 100);

        /* Handle specific error cases */
        if (at_state == AT_STATE_WAITING_CONNECT) {
            /* Connection attempt failed */
            is_wifi_connected = false;
            serial_comm_send_debug("WiFi connection failed\r\n", 100);

        }

        /* Reset AT state */
        at_state = AT_STATE_IDLE;
    }
    else if (strstr((char*)rx_buffer, "+CWLAP:")) {
        /* Received WiFi scan results, continue waiting for OK */
        serial_comm_send_debug("Found WiFi network: ", 100);
    }
    else if (strstr((char*)rx_buffer, "+CIFSR:")) {
        /* Received IP address info, continue waiting for OK */
        serial_comm_send_debug("IP information: ", 100);
    }
}

UART_Status serial_comm_send_game_data(const char *data, uint16_t length, uint32_t timeout)
{
    if (!is_serial_connected || current_state != PROTO_STATE_CONNECTED) {
        return UART_ERROR;
    }

    /* For game data, format will depend on your specific needs */
    /* This is just an example - modify as needed */

    char header[32];
    snprintf(header, sizeof(header), "AT+CIPSEND=%d\r\n", length);

    UART_Status status = serial_comm_send_message(header, timeout);
    if (status != UART_OK) {
        return status;
    }

    /* Wait briefly for the '>' prompt */
    HAL_Delay(100);

    /* Send the actual data */
    return UART_SendBuffer(UART_PORT_2, (uint8_t*)data, length, timeout);
}

void serial_comm_enter_data_mode(void)
{
    current_state = PROTO_STATE_DATA_MODE;
    serial_comm_send_debug("Entered data mode\r\n", 100);
}

void serial_comm_exit_data_mode(void)
{
    current_state = PROTO_STATE_CONNECTED;
    serial_comm_send_debug("Exited data mode\r\n", 100);
}

UART_Status serial_comm_reset(void)
{
    current_state = PROTO_STATE_INIT;
    at_state = AT_STATE_IDLE;
    is_serial_connected = false;
    rx_index = 0;
    message_complete = false;
    at_response_complete = false;

    /* Send reset command to ESP32 */
    UART_Status status = serial_comm_send_at_command("AT+RST", 100);
    if (status != UART_OK) {
        return status;
    }

    serial_comm_send_debug("Communication reset initiated\r\n", 100);
    last_activity_time = get_current_ms();

    return UART_OK;
}

static void UART_RxCallback(uint8_t data)
{
    /* Echo the received character to debug port for monitoring */
    char debug_char[2] = {data, '\0'};
    serial_comm_send_debug(debug_char, 10);

    /* Store byte in buffer if there's space */
    if (rx_index < MAX_BUFFER_SIZE - 1) {
        rx_buffer[rx_index++] = data;
        rx_buffer[rx_index] = '\0'; /* Always keep null-terminated */
    }

    /* Check for line endings for message completion */
    if (data == LINE_ENDING || data == '\r') {
        if (current_state == PROTO_STATE_AT_MODE ||
            current_state == PROTO_STATE_INIT ||
            at_state != AT_STATE_IDLE) {
            /* In AT mode, check if response is complete */
            at_response_complete = check_for_at_response_end();
        } else {
            /* In other modes, treat line ending as message end */
            message_complete = true;
        }
    }
}

static bool check_for_at_response_end(void)
{
    /* AT responses typically end with OK, ERROR, FAIL, or SEND OK */
    if (strstr((char*)rx_buffer, "OK\r\n") ||
        strstr((char*)rx_buffer, "ERROR\r\n") ||
        strstr((char*)rx_buffer, "FAIL\r\n") ||
        strstr((char*)rx_buffer, "SEND OK\r\n")) {
        return true;
    }

    /* Special cases for multi-line responses */
    if (at_state == AT_STATE_WAITING_CWLAP &&
        strstr((char*)rx_buffer, "OK\r\n") &&
        strstr((char*)rx_buffer, "+CWLAP:")) {
        return true;
    }

    /* Not a complete response yet */
    return false;
}
