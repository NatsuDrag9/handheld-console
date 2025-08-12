/*
 * oled.h
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_OLED_H_
#define INC_CONSOLE_PERIPHERALS_OLED_H_

#include <Console_Peripherals/Hardware/d_pad.h>
#include <Console_Peripherals/Hardware/Drivers/display_driver.h>
#include <Console_Peripherals/Hardware/joystick.h>
#include <Communication/serial_comm.h>
#include <Console_Peripherals/types.h>
#include <stdbool.h>
#include <stddef.h>
#include "string.h"
#include "Game_Engine/game_menu.h"
#include "Game_Engine/game_engine.h"
#include "Game_Engine/Games/snake_game.h"
#include "Game_Engine/Games/pacman_game.h"

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
#define MENU_REFRESH_THROTTLE 100  // 100ms throttle (100Hz refresh rate)
#define STATUS_BAR_HEIGHT   15    // Height of the status bar area
#define SEPARATOR_LINE_Y    16    // Y position of the separator line
#elif defined(DISPLAY_MODULE_LCD)
#define MENU_START_Y     50
#define MENU_ITEM_HEIGHT 35
#define VISIBLE_ITEMS    4
#define DISPLAY_FONT Font_7x10
#define DISPLAY_MENU_CURSOR_FONT Font_11x18
#define DISPLAY_TITLE_FONT Font_16x26
#define STATUS_BAR_HEIGHT   20
#define SEPARATOR_LINE_Y    21
#define MENU_TITLE_Y 25
#define MENU_REFRESH_THROTTLE 50  // 50ms throttle (20Hz refresh rate) -
#else
	// Default to OLED screen
#define MENU_START_Y     25    // Y position where menu items start
#define MENU_ITEM_HEIGHT 12    // Height of each menu item
#define VISIBLE_ITEMS    3     // Number of items visible at once
#define DISPLAY_FONT Font_7x10
#define MENU_TITLE_Y 10
#define STATUS_BAR_HEIGHT   15
#define SEPARATOR_LINE_Y    16
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
