/*
 * comm_utils.h
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 *  Modified: Added circular buffer and message queue utilities
 */

#ifndef INC_UTILS_COMM_UTILS_H_
#define INC_UTILS_COMM_UTILS_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Circular Buffer Configuration */
#define CIRCULAR_BUFFER_SIZE 1024
#define MESSAGE_QUEUE_SIZE 8

/* Circular Buffer Handle */
typedef struct {
    uint8_t buffer[CIRCULAR_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t count;
    uint32_t overflow_count;
} circular_buffer_t;

/* Message Queue Handle */
typedef struct {
    void* queue_buffer;           // Points to array of any type
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t count;
    uint8_t max_messages;		// Max number of messages
    uint16_t message_size;        // Size of each message
    uint32_t overflow_count;
} message_queue_t;

/* Message Parser State */
typedef struct {
    uint8_t* parse_buffer;
    uint16_t parse_index;
    uint16_t expected_size;
    bool parsing_active;
} message_parser_t;

/* Statistics Structure */
typedef struct {
    uint32_t bytes_received;
    uint32_t bytes_sent;
    uint32_t messages_parsed;
    uint32_t parse_errors;
    uint32_t buffer_overflows;
    uint32_t queue_overflows;
} comm_stats_t;

// Circular Buffer APIs
void circular_buffer_init(circular_buffer_t* cb);
bool circular_buffer_put(circular_buffer_t* cb, uint8_t data);
bool circular_buffer_get(circular_buffer_t* cb, uint8_t* data);
uint16_t circular_buffer_available(const circular_buffer_t* cb);
uint16_t circular_buffer_free_space(const circular_buffer_t* cb);
void circular_buffer_flush(circular_buffer_t* cb);
uint16_t circular_buffer_peek(const circular_buffer_t* cb, uint8_t* buffer, uint16_t max_bytes);

// Message Queue APIs
void message_queue_init(message_queue_t* mq, void* queue_buffer, uint8_t max_messages, uint16_t message_size);
bool message_queue_put(message_queue_t* mq, const void* msg);
bool message_queue_get(message_queue_t* mq, void* msg);
uint8_t message_queue_available(const message_queue_t* mq);
bool message_queue_is_full(const message_queue_t* mq);
bool message_queue_is_empty(const message_queue_t* mq);
void message_queue_flush(message_queue_t* mq);

// Message Parser APIs
void message_parser_init(message_parser_t* parser, uint8_t* buffer, uint16_t buffer_size);
void message_parser_reset(message_parser_t* parser);
bool message_parser_add_byte(message_parser_t* parser, uint8_t data, uint8_t start_byte);
bool message_parser_is_complete(const message_parser_t* parser);
uint8_t* message_parser_get_buffer(const message_parser_t* parser);

// Circular buffer Communication Statistics APIs
void comm_stats_init(comm_stats_t* stats);
void comm_stats_update_buffer(comm_stats_t* stats, const circular_buffer_t* cb);
void comm_stats_update_queue(comm_stats_t* stats, const message_queue_t* mq);
void comm_stats_print(const comm_stats_t* stats);

// AT Error Translation APIs
const char* translate_at_error(const char* error_response);


#endif /* INC_UTILS_COMM_UTILS_H_ */
