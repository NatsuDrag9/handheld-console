#ifndef MOCK_MATH_H
#define MOCK_MATH_H

#include <stdint.h>

// Math functions
float sinf(float x);
float cosf(float x);

// Test control functions
void mock_math_set_sin_return(float value);
void mock_math_set_cos_return(float value);
float mock_math_get_last_sin_input(void);
float mock_math_get_last_cos_input(void);
void mock_math_reset(void);

#endif // MOCK_MATH_H