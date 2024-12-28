#ifdef UNITY_TEST
#include <stdint.h>
#include "Console_Peripherals/Drivers/push_button_driver.h"
#include "../Inc/mock_push_button_driver.h"

void mock_pb_driver_reset(void);

uint8_t mock_pb1_state = 0;
uint8_t mock_pb2_state = 0;
uint32_t mock_tick_count = 0;

// Mock implementations with conversion
uint8_t PB_Driver_ReadPin1(void) {
    return mock_pb1_state;
}

uint8_t PB_Driver_ReadPin2(void) {
    return mock_pb2_state;
}

uint32_t PB_Driver_GetTick(void) {
    return mock_tick_count;
}

void mock_pb_driver_reset(void) {
    mock_pb1_state = 0;
    mock_pb2_state = 0;
    mock_tick_count = 0;
}

#endif