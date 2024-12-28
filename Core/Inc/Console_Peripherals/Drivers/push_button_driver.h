/*
 * push-button_driver.h
 *
 *  Created on: Dec 28, 2024
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_DRIVERS_PUSH_BUTTON_DRIVER_H_
#define INC_CONSOLE_PERIPHERALS_DRIVERS_PUSH_BUTTON_DRIVER_H_

#include "main.h"

// Function prototypes
// Read raw pin state
GPIO_PinState PB_Driver_ReadPin1(void);
GPIO_PinState PB_Driver_ReadPin2(void);

// Get system time for debouncing
uint32_t PB_Driver_GetTick(void);

#endif /* INC_CONSOLE_PERIPHERALS_DRIVERS_PUSH_BUTTON_DRIVER_H_ */
