#include "mock_math.h"

static struct {
    float sin_return;
    float cos_return;
    float last_sin_input;
    float last_cos_input;
} mock_state = {
    .sin_return = 0.0f,
    .cos_return = 1.0f,  // Default to common values
    .last_sin_input = 0.0f,
    .last_cos_input = 0.0f
};

float sinf(float x) {
    mock_state.last_sin_input = x;
    return mock_state.sin_return;
}

float cosf(float x) {
    mock_state.last_cos_input = x;
    return mock_state.cos_return;
}

void mock_math_set_sin_return(float value) {
    mock_state.sin_return = value;
}

void mock_math_set_cos_return(float value) {
    mock_state.cos_return = value;
}

float mock_math_get_last_sin_input(void) {
    return mock_state.last_sin_input;
}

float mock_math_get_last_cos_input(void) {
    return mock_state.last_cos_input;
}

void mock_math_reset(void) {
    mock_state.sin_return = 0.0f;
    mock_state.cos_return = 1.0f;
    mock_state.last_sin_input = 0.0f;
    mock_state.last_cos_input = 0.0f;
}