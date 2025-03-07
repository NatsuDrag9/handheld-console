/*
 * push_button.h
 *
 *  Created on: Dec 28, 2024
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_PUSH_BUTTON_H_
#define INC_CONSOLE_PERIPHERALS_PUSH_BUTTON_H_

#include "Console_Peripherals/Drivers/push_button_driver.h"

// Button states structure (made public for testing)
typedef struct {
    uint8_t current_state;
    uint8_t last_reading;
    uint32_t last_debounce_time;
    uint8_t pressed;  // New flag for edge detection
} Button_TypeDef;

// Initialize button logic
void pb_init(void);

// Get debounced button states
uint8_t pb1_get_state(void);
uint8_t pb2_get_state(void);
uint8_t dpad_pin_left_get_state();
uint8_t dpad_pin_right_get_state();
uint8_t dpad_pin_up_get_state();
uint8_t dpad_pin_down_get_state();

// Made public for testing
void process_button(Button_TypeDef *button, uint8_t current_reading, uint32_t current_time);



#endif /* INC_CONSOLE_PERIPHERALS_PUSH_BUTTON_H_ */
