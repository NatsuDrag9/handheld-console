/*
 * game_menu.c
 *
 *  Created on: Jan 12, 2025
 *      Author: rohitimandi
 */

#ifndef SRC_GAME_ENGINE_GAME_MENU_C_
#define SRC_GAME_ENGINE_GAME_MENU_C_

#include "Game_Engine/game_menu.h"

MenuItem game_menu[] = {
    {"Snake Game", 1},
    {"Game 2", 0},
    {"Game 3", 0},
    {"Game 4", 0},
    {"Game 5", 0},
    // Add more games here
};

const uint8_t game_menu_size = sizeof(game_menu) / sizeof(game_menu[0]);

void get_game_menu(MenuItem** menu_ptr, uint8_t* size_ptr) {
    if (menu_ptr) *menu_ptr = game_menu;
    if (size_ptr) *size_ptr = game_menu_size;
}

#endif /* SRC_GAME_ENGINE_GAME_MENU_C_ */
