/*
 * push_button_driver.h
 *
 *  Created on: Dec 28, 2024
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_PUSH_BUTTON_DRIVER_H_
#define INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_PUSH_BUTTON_DRIVER_H_

#include "main.h"
#include <stdint.h>

 // Function prototypes
 // Read raw pin state
uint8_t PB_Driver_ReadPin1(void);
uint8_t PB_Driver_ReadPin2(void);
uint8_t read_dpad_pin_left(void);
uint8_t read_dpad_pin_right(void);
uint8_t read_dpad_pin_up(void);
uint8_t read_dpad_pin_down(void);

// Get system time for debouncing
uint32_t PB_Driver_GetTick(void);

#endif /* INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_PUSH_BUTTON_DRIVER_H_ */
