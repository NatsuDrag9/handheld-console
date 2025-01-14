/*
 * console_peripherals_conf.c
 *
 *  Created on: Dec 8, 2024
 *      Author: rohitimandi
 */

#include "Console_Peripherals/console_peripherals_conf.h"

void console_peripherals_init(void) {
	MenuItem* game_menu;
	uint8_t game_menu_size;

	joystick_init();
	pb_init();

	get_game_menu(&game_menu, &game_menu_size);
	oled_init(game_menu, game_menu_size);
}
