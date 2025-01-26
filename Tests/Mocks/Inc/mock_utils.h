#ifndef MOCK_TIME_UTILS_H
#define MOCK_TIME_UTILS_H

#include <stdint.h>
#include "Utils/misc_utils.h"

// Original function from misc_utils.h
uint32_t get_current_ms(void);

// Test helper functions
void mock_time_set_ms(uint32_t ms);
uint32_t mock_time_get_ms(void);
void mock_time_reset(void);

#endif // MOCK_TIME_UTILS_H