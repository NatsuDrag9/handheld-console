/*
 * console_ui.c
 *
 *  Created on: Jun 2, 2025
 *      Author: rohitimandi
 *  Updated: Added support for hierarchical menu system and multiplayer games
 */

#include "Application/console_ui.h"

/*
 * This console UI module provides the main interface for the gaming console.
 * It acts as a high-level API that coordinates the underlying modular architecture:
 *
 * - display_manager: Handles display abstraction and hardware differences
 * - menu_system: Manages menu logic and navigation with hierarchical support
 * - game_controller: Orchestrates application state and game life-cycle
 *
 * Updated to support the new hierarchical menu system with Single Player / Multiplayer
 * game mode selection.
 */

void console_ui_init(MenuItem* menu, uint8_t menu_size) {
    game_controller_init(menu, menu_size);
}

void console_ui_init_with_default_menu(void) {
    game_controller_init_with_default_menu();
}

void console_ui_clear_screen(void) {
    display_manager_clear_screen();
    display_manager_update();
}

void console_ui_show_screen(ScreenType screen) {
    switch (screen) {
        case SCREEN_WELCOME:
            game_controller_show_welcome_screen();
            break;

        case SCREEN_MENU:
            game_controller_show_menu();
            break;

        /* Single Player Games */
        case SCREEN_GAME_SNAKE:
            game_controller_start_game(SCREEN_GAME_SNAKE);
            break;

        case SCREEN_GAME_PACMAN:
            game_controller_start_game(SCREEN_GAME_PACMAN);
            break;

        case SCREEN_GAME_3:
            game_controller_start_game(SCREEN_GAME_3);
            break;

        case SCREEN_GAME_4:
            game_controller_start_game(SCREEN_GAME_4);
            break;

        case SCREEN_GAME_5:
            game_controller_start_game(SCREEN_GAME_5);
            break;

        /* Multiplayer Games */
        case SCREEN_MP_GAME_SNAKE:
            game_controller_start_game(SCREEN_MP_GAME_SNAKE);
            break;

        case SCREEN_MP_GAME_PACMAN:
            game_controller_start_game(SCREEN_MP_GAME_PACMAN);
            break;

        case SCREEN_MP_GAME_3:
            game_controller_start_game(SCREEN_MP_GAME_3);
            break;

        case SCREEN_MP_GAME_4:
            game_controller_start_game(SCREEN_MP_GAME_4);
            break;

        case SCREEN_MP_GAME_5:
            game_controller_start_game(SCREEN_MP_GAME_5);
            break;

        /* Special Screens */
        case SCREEN_GAMEPLAY_SELECTION:
            game_controller_show_menu();  // Show main menu
            break;

        case SCREEN_WIFI_ERROR:
        	/* WiFi error is handled by game controller's error state */
        	game_controller_set_state(APP_STATE_ERROR);
        	break;

        case SCREEN_ERROR:
        	/* General error is handled by game controller's error state */
        	game_controller_set_state(APP_STATE_ERROR);
        	break;
        default:
            /* Default to menu screen for unknown screens */
            game_controller_return_to_menu();
            break;
    }
}

void console_ui_show_menu(MenuItem* items, uint8_t num_items) {
    /* With hierarchical system, this shows the current menu state */
    game_controller_show_menu();
}

void console_ui_handle_input(JoystickStatus js_status) {
    game_controller_handle_input(js_status);
}

MenuItem console_ui_get_selected_menu_item(void) {
    return game_controller_get_selected_menu_item();
}

void console_ui_run_game(void) {
    game_controller_update();
}

bool console_ui_is_game_active(void) {
    return game_controller_is_game_active();
}

void console_ui_set_game_active(bool is_active) {
    /*
     * Game state is now managed internally by game controller.
     * This function provides compatibility but delegates appropriately.
     */
    if (!is_active) {
        game_controller_return_to_menu();
    }
}

/* Menu navigation functions */
bool console_ui_handle_menu_selection(void) {
    return game_controller_handle_menu_selection();
}

void console_ui_handle_back_navigation(void) {
    game_controller_handle_back_navigation();
}

MenuType console_ui_get_current_menu_type(void) {
    return game_controller_get_current_menu_type();
}

bool console_ui_is_in_main_menu(void) {
    return game_controller_is_in_main_menu();
}

/* Status and query functions */
uint8_t console_ui_get_menu_size(void) {
    return game_controller_get_menu_size();
}

uint8_t console_ui_get_current_selection(void) {
    return game_controller_get_current_menu_selection();
}
