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
    GPIO_PinState current_state;
    GPIO_PinState last_reading;
    uint32_t last_debounce_time;
    uint8_t pressed;  // New flag for edge detection
} Button_TypeDef;

// Initialize button logic
void pb_init(void);

// Get debounced button states
uint8_t pb1_get_state(void);
uint8_t pb2_get_state(void);

// Made public for testing
void Process_Button(Button_TypeDef *button, GPIO_PinState current_reading, uint32_t current_time);



#endif /* INC_CONSOLE_PERIPHERALS_PUSH_BUTTON_H_ */
