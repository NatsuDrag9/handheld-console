/*
 * console_ui.h
 *
 *  Created on: Jun 2, 2025
 *      Author: rohitimandi
 *  Updated: Added support for hierarchical menu system and multiplayer games
 */

#ifndef INC_APPLICATION_CONSOLE_UI_H_
#define INC_APPLICATION_CONSOLE_UI_H_

#include <stdbool.h>
#include <stdint.h>
#include "Console_Peripherals/types.h"
#include "Console_Peripherals/Hardware/joystick.h"
#include "Console_Peripherals/Hardware/display_manager.h"
#include "Console_Peripherals/UI/menu_system.h"
#include "Application/game_controller.h"

/*
 * Console UI module provides the main interface for the gaming console.
 * This module coordinates display, menu, and game functionality through
 * the underlying modular architecture.
 *
 *
 */

/* Core console UI functions */
void console_ui_init(MenuItem* menu, uint8_t menu_size);
void console_ui_init_with_default_menu(void);
void console_ui_clear_screen(void);

/* Screen management functions */
void console_ui_show_screen(ScreenType screen);
void console_ui_show_menu(MenuItem* items, uint8_t num_items);
void console_ui_handle_input(JoystickStatus js_status);

/* Game interface functions */
MenuItem console_ui_get_selected_menu_item(void);
void console_ui_run_game(void);
bool console_ui_is_game_active(void);
void console_ui_set_game_active(bool is_active);

/* Menu navigation functions - NEW */
bool console_ui_handle_menu_selection(void);
void console_ui_handle_back_navigation(void);
MenuType console_ui_get_current_menu_type(void);
bool console_ui_is_in_main_menu(void);

/* Status and query functions */
uint8_t console_ui_get_menu_size(void);
uint8_t console_ui_get_current_selection(void);

/* Backward compatibility aliases for existing code */
#ifdef CONSOLE_UI_BACKWARD_COMPATIBILITY
#define oled_init(menu, size)                console_ui_init(menu, size)
#define oled_init_with_default_menu()        console_ui_init_with_default_menu()
#define oled_clear_screen()                  console_ui_clear_screen()
#define oled_show_screen(screen)             console_ui_show_screen(screen)
#define oled_show_menu(items, num)           console_ui_show_menu(items, num)
#define oled_menu_handle_input(js)           console_ui_handle_input(js)
#define oled_get_selected_menu_item()        console_ui_get_selected_menu_item()
#define oled_run_game()                      console_ui_run_game()
#define oled_is_game_active()                console_ui_is_game_active()
#define oled_set_is_game_active(active)      console_ui_set_game_active(active)
#define oled_get_current_menu_size()         console_ui_get_menu_size()
#define oled_get_current_cursor_item()       console_ui_get_current_selection()
#endif

#endif /* INC_APPLICATION_CONSOLE_UI_H_ */
