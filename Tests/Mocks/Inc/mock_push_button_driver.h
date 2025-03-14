#ifndef MOCK_PUSH_BUTTON_DRIVER_H_
#define MOCK_PUSH_BUTTON_DRIVER_H_

#include <stdint.h>

// Variables to control mock behavior
// Using uint8_t instead of GPIO_PinState
extern uint8_t mock_pb1_state;
extern uint8_t mock_pb2_state;
extern uint32_t mock_tick_count;

// D-pad state variables
extern uint8_t mock_dpad_left_state;
extern uint8_t mock_dpad_right_state;
extern uint8_t mock_dpad_up_state;
extern uint8_t mock_dpad_down_state;

// Function to reset mocks
void mock_reset(void);

#endif