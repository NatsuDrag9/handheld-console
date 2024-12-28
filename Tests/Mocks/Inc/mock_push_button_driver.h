#ifndef MOCK_PUSH_BUTTON_DRIVER_H_
#define MOCK_PUSH_BUTTON_DRIVER_H_

#include <stdint.h>

// Variables to control mock behavior
// Using uint8_t instead of GPIO_PinState
extern uint8_t mock_pb1_state;
extern uint8_t mock_pb2_state;
extern uint32_t mock_tick_count;

// Function to reset mocks
void mock_reset(void);

#endif