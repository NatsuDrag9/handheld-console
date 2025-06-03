/*
 * console_peripherals_conf.c
 *
 *  Created on: Dec 8, 2024
 *      Author: rohitimandi
 */

#include "Console_Peripherals/console_peripherals_conf.h"
#include "Utils/misc_utils.h"

void console_peripherals_init(void) {
//	MenuItem* game_menu;
//	uint8_t game_menu_size;

	// Initialize random seed to spawn food and snake
	init_random();

	joystick_init();
	pb_init();

//	get_game_menu(&game_menu, &game_menu_size);
//	oled_init(game_menu, game_menu_size);
	 console_ui_init_with_default_menu();
	audio_init();
	UART_Status uart_status = serial_comm_init();
	if(uart_status) {
//		Replace this with an alert screen on LCD
		DEBUG_PRINTF(false, "UART status after initialization: %d\n", uart_status);
	}

}
