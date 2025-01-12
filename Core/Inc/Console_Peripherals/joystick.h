/*
 * joystick.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_JOYSTICK_H_
#define INC_CONSOLE_PERIPHERALS_JOYSTICK_H_

#include "System/system_conf.h"
#include <stdint.h>
#include "Console_Peripherals/types.h"

void joystick_init(void);
void update_joystick_status(); // Made public for callbacks to access
JoystickStatus joystick_get_status();
uint8_t calculate_direction(uint16_t x, uint16_t y);  // Made public for testing

#endif /* INC_CONSOLE_PERIPHERALS_JOYSTICK_H_ */
