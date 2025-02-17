/*
 * game_menu.h
 *
 *  Created on: Jan 12, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAME_MENU_H_
#define INC_GAME_ENGINE_GAME_MENU_H_

#include <stdint.h>
#include "Utils/debug_conf.h"

#define MAX_MENU_ITEMS  5  // Maximum number of games that can be displayed in menu

typedef enum {
    SCREEN_WELCOME,
    SCREEN_MENU,
    SCREEN_GAME_SNAKE,
	SCREEN_GAME_PACMAN,
    SCREEN_GAME_3,
    SCREEN_GAME_4,
    SCREEN_GAME_5,
	SCREEN_ERROR,
    // Add more screens as you add games
} ScreenType;

typedef struct {
    char* title;
    uint8_t selected;  // For menu items that can be selected
    uint8_t screen;
    uint8_t is_game; // For menu items that are games
} MenuItem;

// Function to get the menu and its size
void get_game_menu(MenuItem** menu, uint8_t* size);

#endif /* INC_GAME_ENGINE_GAME_MENU_H_ */
