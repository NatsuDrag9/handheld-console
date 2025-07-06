/*
 * menu_system.h
 *
 *  Created on: Jun 2, 2025
 *      Author: rohitimandi
 *
 */

#ifndef INC_CONSOLE_PERIPHERALS_UI_MENU_SYSTEM_H_
#define INC_CONSOLE_PERIPHERALS_UI_MENU_SYSTEM_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "Console_Peripherals/types.h"

#define MAX_MENU_ITEMS 5

/* Menu types */
typedef enum {
    MENU_TYPE_MAIN,          // "Select Gameplay" - Single Player / Multiplayer
    MENU_TYPE_SINGLE_PLAYER, // Single player game selection
    MENU_TYPE_MULTIPLAYER    // Multiplayer game selection (after WiFi check)
} MenuType;

/* Game modes */
typedef enum {
    GAME_MODE_SINGLE_PLAYER,
    GAME_MODE_MULTIPLAYER
} GameMode;

/* Screen types */
typedef enum {
    SCREEN_WELCOME,
    SCREEN_MENU,

    // Single player games
    SCREEN_GAME_SNAKE,
    SCREEN_GAME_PACMAN,
    SCREEN_GAME_3,
    SCREEN_GAME_4,
    SCREEN_GAME_5,

    // Multiplayer games (each single player game has a multiplayer counterpart)
    SCREEN_MP_GAME_SNAKE,
    SCREEN_MP_GAME_PACMAN,
    SCREEN_MP_GAME_3,
    SCREEN_MP_GAME_4,
    SCREEN_MP_GAME_5,

    // Special screens
    SCREEN_GAMEPLAY_SELECTION,  // Main menu: Single Player / Multiplayer
    SCREEN_WIFI_ERROR,          // WiFi connection error
	SCREEN_MENU_ERROR,			// Menu unavaialble error
    SCREEN_ERROR
} ScreenType;

/* Menu item structure */
typedef struct {
    char* title;
    uint8_t selected;     /* For menu items that can be selected */
    ScreenType screen;    /* Screen to navigate to */
    uint8_t is_game;      /* 1 if this is a game, 0 if menu navigation */
    GameMode game_mode;   /* Single player or multiplayer */
} MenuItem;

/* Menu navigation directions */
typedef enum {
    MENU_NAV_UP = 1,
    MENU_NAV_DOWN = 2,
    MENU_NAV_SELECT = 3,
    MENU_NAV_BACK = 4,    // Added back navigation
    MENU_NAV_NONE = 0
} MenuNavigation;

/* Menu state structure */
typedef struct {
    MenuItem items[MAX_MENU_ITEMS];
    uint8_t item_count;
    uint8_t current_selection;
    uint8_t scroll_position;
    bool needs_full_refresh;
    MenuType current_menu_type;  // Track which menu we're in
    MenuType previous_menu_type; // For back navigation
} MenuState;

/* Menu system initialization and cleanup */
void menu_system_init(MenuState* menu_state, MenuItem* items, uint8_t item_count);
void menu_system_reset(MenuState* menu_state);

/* Menu rendering functions */
bool menu_system_render(const MenuState* menu_state);
bool menu_system_render_partial_update(const MenuState* menu_state, uint8_t old_selection);

/* Menu navigation functions */
bool menu_system_handle_navigation(MenuState* menu_state, MenuNavigation direction);
MenuItem menu_system_get_selected_item(const MenuState* menu_state);
bool menu_system_is_selection_valid(const MenuState* menu_state);

/* Menu state queries */
uint8_t menu_system_get_current_selection(const MenuState* menu_state);
uint8_t menu_system_get_scroll_position(const MenuState* menu_state);
uint8_t menu_system_get_item_count(const MenuState* menu_state);
bool menu_system_needs_full_refresh(const MenuState* menu_state);
MenuType menu_system_get_current_menu_type(const MenuState* menu_state);

/* Menu state manipulation */
void menu_system_set_selection(MenuState* menu_state, uint8_t selection);
void menu_system_mark_selected_item(MenuState* menu_state);
void menu_system_clear_refresh_flag(MenuState* menu_state);

/* Menu navigation helpers */
void menu_system_navigate_to_single_player(MenuState* menu_state);
bool menu_system_navigate_to_multiplayer(MenuState* menu_state); // Returns false if WiFi not connected
void menu_system_navigate_back(MenuState* menu_state);
void menu_system_navigate_to_main_menu(MenuState* menu_state);

/* Utility functions */
bool menu_system_is_item_visible(const MenuState* menu_state, uint8_t item_index);
uint8_t menu_system_get_visible_position(const MenuState* menu_state, uint8_t item_index);

/* Game menu data functions */
void menu_system_get_main_menu(MenuItem** menu_ptr, uint8_t* size_ptr);
void menu_system_get_single_player_menu(MenuItem** menu_ptr, uint8_t* size_ptr);
void menu_system_get_multiplayer_menu(MenuItem** menu_ptr, uint8_t* size_ptr);

/* Legacy compatibility functions - for backward compatibility */
void menu_system_get_game_menu(MenuItem** menu_ptr, uint8_t* size_ptr);
const MenuItem* menu_system_get_default_game_menu(void);
uint8_t menu_system_get_default_game_menu_size(void);

#endif /* INC_CONSOLE_PERIPHERALS_UI_MENU_SYSTEM_H_ */
