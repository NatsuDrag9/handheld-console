#include "../Inc/mock_utils.h"

static uint32_t current_ms = 0;

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