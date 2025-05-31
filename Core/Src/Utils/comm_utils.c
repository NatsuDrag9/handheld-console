/*
 * comm_utils.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 */
#include "Utils/comm_utils.h"
#include "Utils/debug_conf.h"

// Circular buffer implementation
void circular_buffer_init(circular_buffer_t* cb) {
    if (cb == NULL) return;

    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    cb->overflow_count = 0;
    memset(cb->buffer, 0, CIRCULAR_BUFFER_SIZE);
}

bool circular_buffer_put(circular_buffer_t* cb, uint8_t data) {
    if (cb == NULL) return false;

    if (cb->count >= CIRCULAR_BUFFER_SIZE) {
        cb->overflow_count++;
        return false; // Buffer full
    }

    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % CIRCULAR_BUFFER_SIZE;
    cb->count++;
    return true;
}

bool circular_buffer_get(circular_buffer_t* cb, uint8_t* data) {
    if (cb == NULL || data == NULL) return false;

    if (cb->count == 0) {
        return false; // Buffer empty
    }

    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % CIRCULAR_BUFFER_SIZE;
    cb->count--;
    return true;
}

uint16_t circular_buffer_available(const circular_buffer_t* cb) {
    if (cb == NULL) return 0;
    return cb->count;
}

uint16_t circular_buffer_free_space(const circular_buffer_t* cb) {
    if (cb == NULL) return 0;
    return CIRCULAR_BUFFER_SIZE - cb->count;
}

void circular_buffer_flush(circular_buffer_t* cb) {
    if (cb == NULL) return;

    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
}

uint16_t circular_buffer_peek(const circular_buffer_t* cb, uint8_t* buffer, uint16_t max_bytes) {
    if (cb == NULL || buffer == NULL) return 0;

    uint16_t available = cb->count;
    uint16_t to_peek = (available < max_bytes) ? available : max_bytes;
    uint16_t tail = cb->tail;

    for (uint16_t i = 0; i < to_peek; i++) {
        buffer[i] = cb->buffer[(tail + i) % CIRCULAR_BUFFER_SIZE];
    }

    return to_peek;
}

// Message queue implementation
void message_queue_init(message_queue_t* mq, void* queue_buffer,
                       uint8_t max_messages, uint16_t message_size) {
    if (mq == NULL || queue_buffer == NULL) return;

    mq->queue_buffer = queue_buffer;
    mq->head = 0;
    mq->tail = 0;
    mq->count = 0;
    mq->max_messages = max_messages;
    mq->message_size = message_size;
    mq->overflow_count = 0;
}

bool message_queue_put(message_queue_t* mq, const void* msg) {
    if (mq == NULL || msg == NULL || mq->queue_buffer == NULL) return false;

    if (mq->count >= mq->max_messages) {
        mq->overflow_count++;
        return false; // Queue full
    }

    // Calculate offset for this message slot
    uint8_t* queue_bytes = (uint8_t*)mq->queue_buffer;
    uint8_t* dest = &queue_bytes[mq->head * mq->message_size];

    memcpy(dest, msg, mq->message_size);
    mq->head = (mq->head + 1) % mq->max_messages;
    mq->count++;
    return true;
}

bool message_queue_get(message_queue_t* mq, void* msg) {
    if (mq == NULL || msg == NULL || mq->queue_buffer == NULL) return false;

    if (mq->count == 0) {
        return false; // Queue empty
    }

    // Calculate offset for this message slot
    uint8_t* queue_bytes = (uint8_t*)mq->queue_buffer;
    uint8_t* src = &queue_bytes[mq->tail * mq->message_size];

    memcpy(msg, src, mq->message_size);
    mq->tail = (mq->tail + 1) % mq->max_messages;
    mq->count--;
    return true;
}

uint8_t message_queue_available(const message_queue_t* mq) {
    if (mq == NULL) return 0;
    return mq->count;
}

bool message_queue_is_full(const message_queue_t* mq) {
    if (mq == NULL) return true;
    return (mq->count >= mq->max_messages);
}

bool message_queue_is_empty(const message_queue_t* mq) {
    if (mq == NULL) return true;
    return (mq->count == 0);
}

void message_queue_flush(message_queue_t* mq) {
    if (mq == NULL) return;

    mq->head = 0;
    mq->tail = 0;
    mq->count = 0;
    // Don't reset message_size or max_messages - they're configuration
}

// Message parser implementation
void message_parser_init(message_parser_t* parser, uint8_t* buffer, uint16_t buffer_size) {
    if (parser == NULL || buffer == NULL) return;

    parser->parse_buffer = buffer;
    parser->parse_index = 0;
    parser->expected_size = buffer_size;
    parser->parsing_active = false;
    memset(buffer, 0, buffer_size);
}

void message_parser_reset(message_parser_t* parser) {
    if (parser == NULL) return;

    parser->parsing_active = false;
    parser->parse_index = 0;
    if (parser->parse_buffer != NULL) {
        memset(parser->parse_buffer, 0, parser->expected_size);
    }
}

bool message_parser_add_byte(message_parser_t* parser, uint8_t data, uint8_t start_byte) {
    if (parser == NULL || parser->parse_buffer == NULL) return false;

    if (!parser->parsing_active) {
        // Looking for start byte
        if (data == start_byte) {
            parser->parsing_active = true;
            parser->parse_index = 0;
            parser->parse_buffer[parser->parse_index++] = data;
            return true;
        }
        // Ignore all other bytes when not parsing
        return false;
    } else {
        // Currently parsing a message
        if (parser->parse_index < parser->expected_size) {
            parser->parse_buffer[parser->parse_index++] = data;
            return true;
        } else {
            // Buffer overflow during parsing - reset
            message_parser_reset(parser);
            return false;
        }
    }
}

bool message_parser_is_complete(const message_parser_t* parser) {
    if (parser == NULL) return false;
    return (parser->parsing_active && parser->parse_index >= parser->expected_size);
}

uint8_t* message_parser_get_buffer(const message_parser_t* parser) {
    if (parser == NULL) return NULL;
    return parser->parse_buffer;
}


// Statistics implementation

void comm_stats_init(comm_stats_t* stats) {
    if (stats == NULL) return;

    memset(stats, 0, sizeof(comm_stats_t));
}

void comm_stats_update_buffer(comm_stats_t* stats, const circular_buffer_t* cb) {
    if (stats == NULL || cb == NULL) return;

    stats->buffer_overflows = cb->overflow_count;
    // Add other buffer-related stats as needed
}

void comm_stats_update_queue(comm_stats_t* stats, const message_queue_t* mq) {
    if (stats == NULL || mq == NULL) return;

    stats->queue_overflows = mq->overflow_count;
    // Add other queue-related stats as needed
}

void comm_stats_print(const comm_stats_t* stats) {
    if (stats == NULL) return;

    DEBUG_PRINTF(false, "=== Communication Statistics ===\r\n");
    DEBUG_PRINTF(false, "Bytes received: %lu\r\n", stats->bytes_received);
    DEBUG_PRINTF(false, "Bytes sent: %lu\r\n", stats->bytes_sent);
    DEBUG_PRINTF(false, "Messages parsed: %lu\r\n", stats->messages_parsed);
    DEBUG_PRINTF(false, "Parse errors: %lu\r\n", stats->parse_errors);
    DEBUG_PRINTF(false, "Buffer overflows: %lu\r\n", stats->buffer_overflows);
    DEBUG_PRINTF(false, "Queue overflows: %lu\r\n", stats->queue_overflows);
    DEBUG_PRINTF(false, "==============================\r\n");
}


// AT error translation implementation
 /**
  * Translates ESP32 AT command error codes into human-readable messages
  * @param error_response The AT command error response string
  * @return A human-readable error message
  */
    const char* translate_at_error(const char* error_response) {
    // WiFi connection errors (AT+CWJAP)
    if (strstr(error_response, "+CWJAP:1"))
        return "WiFi connection timeout - AP not responding";
    if (strstr(error_response, "+CWJAP:2"))
        return "WiFi connection failed - Wrong password";
    if (strstr(error_response, "+CWJAP:3"))
        return "WiFi connection failed - AP not found";
    if (strstr(error_response, "+CWJAP:4"))
        return "WiFi connection failed - Connection error (incorrect credentials)";
    if (strstr(error_response, "+CWJAP:5"))
        return "WiFi connection failed - Connection refused by AP";
    if (strstr(error_response, "+CWJAP:6"))
        return "WiFi connection failed - Unknown/unhandled error";

    // TCP connection errors (AT+CIPSTART)
    if (strstr(error_response, "ALREADY CONNECTED"))
        return "TCP connection error - Connection already exists";
    if (strstr(error_response, "CONNECT FAIL"))
        return "TCP connection error - Connection failed";
    if (strstr(error_response, "IP NEGATIVE"))
        return "TCP connection error - Invalid IP address";
    if (strstr(error_response, "CLOSED"))
        return "TCP connection was closed";

    // IP errors (AT+CIPSTATUS)
    if (strstr(error_response, "+CIPSTATUS:0"))
        return "IP status - ESP32 station not initialized";
    if (strstr(error_response, "+CIPSTATUS:1"))
        return "IP status - ESP32 station initialized but not connected to AP";
    if (strstr(error_response, "+CIPSTATUS:2"))
        return "IP status - ESP32 station connected to AP but no TCP connection";
    if (strstr(error_response, "+CIPSTATUS:4"))
        return "IP status - ESP32 station disconnected from AP";
    if (strstr(error_response, "+CIPSTATUS:5"))
        return "IP status - ESP32 station not connected to AP";

    // Sending data errors
    if (strstr(error_response, "SEND FAIL"))
        return "Data transmission failed";

    // Generic AT errors
    if (strstr(error_response, "ERROR"))
        return "Generic AT command error - Command not recognized or invalid parameters";
    if (strstr(error_response, "busy"))
        return "AT system busy - Previous command still processing";

    // Default case - unknown error
    return "Unknown AT error response";
}


