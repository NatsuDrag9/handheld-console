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
    {"Snake Game", 0, SCREEN_GAME_SNAKE, 1},
    {"Game 2", 0, SCREEN_GAME_2, 1},
    {"Game 3", 0, SCREEN_GAME_3, 1},
    {"Game 4", 0, SCREEN_GAME_4, 1},
    {"Game 5", 0, SCREEN_GAME_5, 1},
};

const uint8_t game_menu_size = sizeof(game_menu) / sizeof(game_menu[0]);

void get_game_menu(MenuItem** menu_ptr, uint8_t* size_ptr) {
    // Return NULL/0 if any one of the pointers is NULL
    if (!menu_ptr || !size_ptr) {
        if (menu_ptr) *menu_ptr = NULL;
        if (size_ptr) *size_ptr = 0;
        return;
    }

    // Only set values if both pointers are valid
    *menu_ptr = game_menu;
    *size_ptr = game_menu_size;
}

#endif /* SRC_GAME_ENGINE_GAME_MENU_C_ */
