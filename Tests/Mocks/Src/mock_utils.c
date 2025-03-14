#include "../Inc/mock_utils.h"

static uint32_t current_ms = 0;
static uint32_t next_random_value = 0;
static uint8_t mock_random_initialized = 0;

// Time functions
uint32_t get_current_ms(void) {
    return current_ms;
}

void mock_time_set_ms(uint32_t ms) {
    current_ms = ms;
}

uint32_t mock_time_get_ms(void) {
    return current_ms;
}

void mock_time_reset(void) {
    current_ms = 0;
}

// Random functions
uint32_t get_random(void) {
    if (!mock_random_initialized) {
        return 0;
    }
    return next_random_value;
}

void mock_random_set_next_value(uint32_t value) {
    next_random_value = value;
    mock_random_initialized = 1;
}

void mock_random_reset(void) {
    next_random_value = 0;
    mock_random_initialized = 0;
}