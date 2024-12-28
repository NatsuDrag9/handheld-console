/*
 * console_peripherals_conf.c
 *
 *  Created on: Dec 8, 2024
 *      Author: rohitimandi
 */

#include "Console_Peripherals/console_peripherals_conf.h"

void console_peripherals_init(void) {
	joystick_init();
	pb_init();
}
