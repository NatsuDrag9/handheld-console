/*
 * oled.h
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_OLED_H_
#define INC_CONSOLE_PERIPHERALS_OLED_H_

#include <stdbool.h>
#include "string.h"
#include "Console_Peripherals/types.h"
#include "Console_Peripherals/Drivers/display_driver.h"
#include "Game_Engine/game_menu.h"

#ifdef UNITY_TEST
#define add_delay(x)
#endif

#define SCREEN_WIDTH    DISPLAY_WIDTH
#define SCREEN_HEIGHT   DISPLAY_HEIGHT
#define MENU_START_Y     25    // Y position where menu items start
#define MENU_ITEM_HEIGHT 12    // Height of each menu item
#define VISIBLE_ITEMS    3     // Number of items visible at once

// Core OLED functions
void oled_init(MenuItem* menu, uint8_t menu_size);
void oled_clear_screen(void);

// Screen management functions
void oled_show_screen(ScreenType screen);
void oled_show_menu(MenuItem* items, uint8_t num_items);
void oled_menu_handle_input(JoystickStatus js_status);


#endif /* INC_CONSOLE_PERIPHERALS_OLED_H_ */
