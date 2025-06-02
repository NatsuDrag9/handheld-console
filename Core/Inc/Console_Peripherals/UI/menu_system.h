/*
 * menu_system.h
 *
 *  Created on: Jun 2, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_UI_MENU_SYSTEM_H_
#define INC_CONSOLE_PERIPHERALS_UI_MENU_SYSTEM_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "Console_Peripherals/types.h"

#define MAX_MENU_ITEMS 5

/* Screen types - moved from game_menu.h */
typedef enum {
    SCREEN_WELCOME,
    SCREEN_MENU,
    SCREEN_GAME_SNAKE,
    SCREEN_GAME_PACMAN,
    SCREEN_GAME_3,
    SCREEN_GAME_4,
    SCREEN_GAME_5,
    SCREEN_ERROR,
    /* Add more screens as you add games */
} ScreenType;

/* Menu item structure - moved from game_menu.h */
typedef struct {
    char* title;
    uint8_t selected;  /* For menu items that can be selected */
    ScreenType screen; /* Changed from uint8_t to ScreenType for type safety */
    uint8_t is_game;   /* For menu items that are games */
} MenuItem;

/* Menu navigation directions */
typedef enum {
    MENU_NAV_UP = 1,
    MENU_NAV_DOWN = 2,
    MENU_NAV_SELECT = 3,
    MENU_NAV_NONE = 0
} MenuNavigation;

/* Menu state structure */
typedef struct {
    MenuItem items[MAX_MENU_ITEMS];
    uint8_t item_count;
    uint8_t current_selection;
    uint8_t scroll_position;
    bool needs_full_refresh;
} MenuState;

/* Menu system initialization and cleanup */
void menu_system_init(MenuState* menu_state, MenuItem* items, uint8_t item_count);
void menu_system_reset(MenuState* menu_state);

/* Menu rendering functions */
void menu_system_render(const MenuState* menu_state);
void menu_system_render_partial_update(const MenuState* menu_state, uint8_t old_selection);

/* Menu navigation functions */
bool menu_system_handle_navigation(MenuState* menu_state, MenuNavigation direction);
MenuItem menu_system_get_selected_item(const MenuState* menu_state);
bool menu_system_is_selection_valid(const MenuState* menu_state);

/* Menu state queries */
uint8_t menu_system_get_current_selection(const MenuState* menu_state);
uint8_t menu_system_get_scroll_position(const MenuState* menu_state);
uint8_t menu_system_get_item_count(const MenuState* menu_state);
bool menu_system_needs_full_refresh(const MenuState* menu_state);

/* Menu state manipulation */
void menu_system_set_selection(MenuState* menu_state, uint8_t selection);
void menu_system_mark_selected_item(MenuState* menu_state);
void menu_system_clear_refresh_flag(MenuState* menu_state);

/* Utility functions */
bool menu_system_is_item_visible(const MenuState* menu_state, uint8_t item_index);
uint8_t menu_system_get_visible_position(const MenuState* menu_state, uint8_t item_index);

/* Game menu data and utilities - integrated from game_menu module */
void menu_system_get_game_menu(MenuItem** menu_ptr, uint8_t* size_ptr);
const MenuItem* menu_system_get_default_game_menu(void);
uint8_t menu_system_get_default_game_menu_size(void);

#endif /* INC_CONSOLE_PERIPHERALS_UI_MENU_SYSTEM_H_ */
