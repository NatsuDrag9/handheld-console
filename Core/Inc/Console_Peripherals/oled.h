/*
 * oled.h
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_OLED_H_
#define INC_CONSOLE_PERIPHERALS_OLED_H_

#include <stdbool.h>
#include <stddef.h>
#include "string.h"
#include "Console_Peripherals/types.h"
#include "Console_Peripherals/joystick.h"
#include "Console_Peripherals/Drivers/display_driver.h"
#include "Console_Peripherals/d_pad.h"
#include "Game_Engine/game_menu.h"

#ifdef UNITY_TEST
#define add_delay(x)
#endif

#define SCREEN_WIDTH    DISPLAY_WIDTH
#define SCREEN_HEIGHT   DISPLAY_HEIGHT

/* Define the macros for display module */
#if defined(DISPLAY_MODULE_OLED)
	#define MENU_START_Y     25    // Y position where menu items start
	#define MENU_ITEM_HEIGHT 12    // Height of each menu item
	#define VISIBLE_ITEMS    3     // Number of items visible at once
	#define DISPLAY_FONT Font_7x10
	#define DISPLAY_MENU_CURSOR_FONT Font_7x10
	#define DISPLAY_TITLE_FONT Font_7x10
	#define MENU_TITLE_Y 10
#elif defined(DISPLAY_MODULE_LCD)
	#define MENU_START_Y     45
	#define MENU_ITEM_HEIGHT 35
	#define VISIBLE_ITEMS    5
	#define DISPLAY_FONT Font_7x10
	#define DISPLAY_MENU_CURSOR_FONT Font_11x18
	#define DISPLAY_TITLE_FONT Font_16x26
	#define MENU_TITLE_Y 20
#else
	// Default to OLED screen
	#define MENU_START_Y     25    // Y position where menu items start
	#define MENU_ITEM_HEIGHT 12    // Height of each menu item
	#define VISIBLE_ITEMS    3     // Number of items visible at once
    #define DISPLAY_FONT Font_7x10
	#define MENU_TITLE_Y 10
#endif

 // Core OLED functions
void oled_init(MenuItem* menu, uint8_t menu_size);
void oled_clear_screen(void);

// Screen management functions
void oled_show_screen(ScreenType screen);
void oled_show_menu(MenuItem* items, uint8_t num_items);
void oled_menu_handle_input(JoystickStatus js_status);

// Functions for game engine
MenuItem oled_get_selected_menu_item(void);
void oled_run_game(void);
bool oled_is_game_active(void);
void oled_set_is_game_active(bool is_active);

// For tests
uint8_t oled_get_current_menu_size(void);
uint8_t oled_get_current_cursor_item(void);

#endif /* INC_CONSOLE_PERIPHERALS_OLED_H_ */
