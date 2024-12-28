#ifdef UNITY_TEST
#include "Console_Peripherals/Drivers/joystick_driver.h"

// Test-specific global variables
static uint16_t test_x = 2048, test_y = 2048;
static uint8_t test_button = 0;

void joystick_driver_init(void) {
    // Minimal initialization for tests
}

void joystick_driver_start_conversion(void) {
    // Stub implementation
}

void joystick_driver_set_values(uint16_t x, uint16_t y, uint8_t button) {
    test_x = x;
    test_y = y;
    test_button = button;
}

uint8_t joystick_driver_read_button(void) {
    return test_button;
}

void joystick_driver_get_adc_values(uint16_t* x, uint16_t* y) {
    *x = test_x;
    *y = test_y;
}

void joystick_driver_tim_callback(void* htim) {
    // Empty stub for testing
}

void joystick_driver_adc_callback(void* hadc) {
    // Empty stub for testing
}

// Mock implementations for interrupt control
void __disable_irq(void) {
    // Simulate disabling interrupts for testing
}

void __enable_irq(void) {
    // Simulate enabling interrupts for testing
}

#endif