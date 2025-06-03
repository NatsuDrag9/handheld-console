/*
 * display_manager.h
 *
 *  Created on: Jun 2, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_HARDWARE_DISPLAY_MANAGER_H_
#define INC_CONSOLE_PERIPHERALS_HARDWARE_DISPLAY_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>
#include "Console_Peripherals/Hardware/Drivers/display_driver.h"

/* Display configuration constants - centralized here */
#if defined(DISPLAY_MODULE_OLED)
    #define DISPLAY_WELCOME_LINE1_Y     (SEPARATOR_LINE_Y + 10)
    #define DISPLAY_WELCOME_LINE2_Y     (SEPARATOR_LINE_Y + 25)
    #define DISPLAY_WELCOME_FONT        Font_7x10
    #define DISPLAY_TITLE_FONT          Font_7x10
    #define DISPLAY_MENU_FONT           Font_7x10
    #define DISPLAY_STATUS_FONT         Font_7x10
    #define DISPLAY_ERROR_FONT          Font_7x10
    #define DISPLAY_GAME_TITLE_Y        15
    #define DISPLAY_GAME_OVER_TITLE_Y   20
    #define DISPLAY_GAME_OVER_MSG_Y     35
    #define DISPLAY_GAME_OVER_SCORE_Y   50
    #define SEPARATOR_LINE_Y            16
    #define STATUS_BAR_HEIGHT           15
#elif defined(DISPLAY_MODULE_LCD)
    #define DISPLAY_WELCOME_LINE1_Y     (SEPARATOR_LINE_Y + 10)
    #define DISPLAY_WELCOME_LINE2_Y     (SEPARATOR_LINE_Y + 40)
    #define DISPLAY_WELCOME_FONT        Font_16x26
    #define DISPLAY_TITLE_FONT          Font_16x26
    #define DISPLAY_MENU_FONT           Font_11x18
    #define DISPLAY_STATUS_FONT         Font_11x18
    #define DISPLAY_ERROR_FONT          Font_11x18
    #define DISPLAY_GAME_TITLE_Y        30
    #define DISPLAY_GAME_OVER_TITLE_Y   40
    #define DISPLAY_GAME_OVER_MSG_Y     80
    #define DISPLAY_GAME_OVER_SCORE_Y   120
    #define SEPARATOR_LINE_Y            21
    #define STATUS_BAR_HEIGHT           20
#else
    /* Default to OLED configuration */
    #define DISPLAY_WELCOME_LINE1_Y     26
    #define DISPLAY_WELCOME_LINE2_Y     41
    #define DISPLAY_WELCOME_FONT        Font_7x10
    #define DISPLAY_TITLE_FONT          Font_7x10
    #define DISPLAY_MENU_FONT           Font_7x10
    #define DISPLAY_STATUS_FONT         Font_7x10
    #define DISPLAY_ERROR_FONT          Font_7x10
    #define DISPLAY_GAME_TITLE_Y        15
    #define DISPLAY_GAME_OVER_TITLE_Y   20
    #define DISPLAY_GAME_OVER_MSG_Y     35
    #define DISPLAY_GAME_OVER_SCORE_Y   50
    #define SEPARATOR_LINE_Y            16
    #define STATUS_BAR_HEIGHT           15
#endif

/* Core display manager functions */
void display_manager_init(void);
void display_manager_clear_screen(void);
void display_manager_clear_main_area(void);
void display_manager_update(void);

/* Semantic display functions - these hide display-specific details */
void display_manager_show_welcome_message(char* line1, char* line2);
void display_manager_show_game_title(char* title);
void display_manager_show_game_over_message(char* message, uint32_t final_score);
void display_manager_show_status_message(char* message);
void display_manager_show_centered_message(char* message, uint8_t y_position);
void display_manager_show_error_message(char* error);

/* Status bar functions */
void display_manager_draw_status_bar(bool wifi_connected, uint32_t score, int lives, bool in_game);
void display_manager_draw_border(void);

/* Menu-specific drawing functions */
void display_manager_draw_menu_title(const char* title);
void display_manager_draw_menu_item(const char* item_text, uint8_t position, bool is_selected);
void display_manager_draw_scrollbar(uint8_t total_items, uint8_t visible_items, uint8_t scroll_position);
void display_manager_clear_menu_item_area(uint8_t position);

/* Low-level positioning helpers */
uint8_t display_manager_get_menu_item_y(uint8_t position);
uint8_t display_manager_get_menu_start_y(void);
uint8_t display_manager_get_menu_item_height(void);
uint8_t display_manager_get_visible_items_count(void);


#endif /* INC_CONSOLE_PERIPHERALS_HARDWARE_DISPLAY_MANAGER_H_ */
