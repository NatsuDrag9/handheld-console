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

#define SCREEN_WIDTH    DISPLAY_WIDTH
#define SCREEN_HEIGHT   DISPLAY_HEIGHT
#define MAX_MENU_ITEMS  5  // Maximum number of games that can be displayed in menu
#define MENU_START_Y     25    // Y position where menu items start
#define MENU_ITEM_HEIGHT 12    // Height of each menu item
#define VISIBLE_ITEMS    3     // Number of items visible at once

typedef enum {
    SCREEN_WELCOME,
    SCREEN_MENU,
    SCREEN_GAME_SNAKE,
	SCREEN_GAME_2,
	SCREEN_GAME_3,
	SCREEN_GAME_4,
	SCREEN_GAME_5,
    // Add more screens as you add games
} ScreenType;

typedef struct {
    char* title;
    uint8_t selected;  // For menu items that can be selected
} MenuItem;


/* OLED function prototype */

// Core OLED functions
void oled_init(void);
void oled_clear_screen(void);

// Screen management functions
void oled_show_screen(ScreenType screen);
void oled_show_menu(MenuItem* items, uint8_t num_items);
void oled_menu_handle_input(JoystickStatus js_status);


#endif /* INC_CONSOLE_PERIPHERALS_OLED_H_ */
