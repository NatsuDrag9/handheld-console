/*
 * display_manager.c
 *
 *  Created on: Jun 2, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Hardware/display_manager.h"
#include "Sprites/status_bar_sprite.h"
#include <stdio.h>
#include <string.h>

#ifdef UNITY_TEST
#define add_delay(x)
#else
extern void add_delay(uint32_t ms);
#endif

/* Display configuration constants based on display type */
#if defined(DISPLAY_MODULE_OLED)
#define MENU_START_Y     25
#define MENU_ITEM_HEIGHT 12
#define VISIBLE_ITEMS    3
#define MENU_TITLE_Y     10
#elif defined(DISPLAY_MODULE_LCD)
#define MENU_START_Y     50
#define MENU_ITEM_HEIGHT 35
#define VISIBLE_ITEMS    4
#define MENU_TITLE_Y     25
#else
/* Default to OLED */
#define MENU_START_Y     25
#define MENU_ITEM_HEIGHT 12
#define VISIBLE_ITEMS    3
#define MENU_TITLE_Y     10
#endif

void display_manager_init(void) {
	display_init();
	add_delay(10);
	display_clear();
}

void display_manager_clear_screen(void) {
	display_clear();
}

void display_manager_clear_main_area(void) {
	/* Clear everything below the status bar */
	display_fill_rectangle(0, STATUS_BAR_HEIGHT + 1,
			DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1,
			DISPLAY_BLACK);
}

void display_manager_update(void) {
	display_update();
}

void display_manager_show_welcome_message(const char* line1, const char* line2) {
	display_clear();
	display_draw_border();

	display_write_string_centered(line1, DISPLAY_WELCOME_FONT,
			DISPLAY_WELCOME_LINE1_Y, DISPLAY_WHITE);
	display_write_string_centered(line2, DISPLAY_WELCOME_FONT,
			DISPLAY_WELCOME_LINE2_Y, DISPLAY_WHITE);

	display_update();
	add_delay(3000);  /* Show welcome screen for 3 seconds */
}

void display_manager_show_game_title(const char* title) {
	display_write_string_centered((char*) title, DISPLAY_TITLE_FONT,
			DISPLAY_GAME_TITLE_Y, DISPLAY_WHITE);
}

void display_manager_show_game_over_message(const char* message, uint32_t final_score) {
	display_clear();
	display_draw_border();

	display_write_string_centered("Game Over", DISPLAY_TITLE_FONT,
			DISPLAY_GAME_OVER_TITLE_Y, DISPLAY_WHITE);

	if (message) {
		display_write_string_centered(message, DISPLAY_MENU_FONT,
				DISPLAY_GAME_OVER_MSG_Y, DISPLAY_WHITE);
	}

	char score_str[32];
	snprintf(score_str, sizeof(score_str), "Final Score: %lu", final_score);
	display_write_string_centered(score_str, DISPLAY_MENU_FONT,
			DISPLAY_GAME_OVER_SCORE_Y, DISPLAY_WHITE);

	display_update();
}

void display_manager_show_status_message(const char* message) {
	display_clear();
	display_write_string_centered((char*) message, DISPLAY_STATUS_FONT, 25, DISPLAY_WHITE);
	display_update();
	add_delay(100);
}

void display_manager_show_centered_message(const char* message, uint8_t y_position) {
	display_write_string_centered((char*) message, DISPLAY_STATUS_FONT, y_position, DISPLAY_WHITE);
}

void display_manager_show_error_message(const char* error) {
	display_clear();
	display_write_string_centered("ERROR", DISPLAY_ERROR_FONT, 20, DISPLAY_WHITE);
	display_write_string_centered(error, DISPLAY_ERROR_FONT, 35, DISPLAY_WHITE);
	display_update();
	add_delay(2000);
}

void display_manager_draw_status_bar(bool wifi_connected, uint32_t score, int lives, bool in_game) {
	/* Draw horizontal separator line */
	display_draw_horizontal_line(0, SEPARATOR_LINE_Y, DISPLAY_WIDTH, DISPLAY_WHITE);

	/* Draw WiFi status icon */
	if (wifi_connected) {
		display_draw_bitmap(DISPLAY_WIDTH - 15, 3, wifi_connected_icon,
				STATUS_ICON_WIDTH, STATUS_ICON_HEIGHT, DISPLAY_WHITE);
	} else {
		display_draw_bitmap(DISPLAY_WIDTH - 15, 3, wifi_disconnected_icon,
				STATUS_ICON_WIDTH, STATUS_ICON_HEIGHT, DISPLAY_WHITE);
	}

	/* Draw game score and lives if in game */
	if (in_game) {
		char status_text[32];
		snprintf(status_text, sizeof(status_text), "Score: %lu Lives: %d", score, lives);
		display_set_cursor(5, 2);
		display_write_string(status_text, DISPLAY_STATUS_FONT, DISPLAY_WHITE);
	}
}

void display_manager_draw_border(void) {
	display_draw_border();
}

void display_manager_draw_menu_title(const char* title) {
	display_write_string_centered(title, DISPLAY_TITLE_FONT, MENU_TITLE_Y, DISPLAY_WHITE);
}

void display_manager_draw_menu_item(const char* item_text, uint8_t position, bool is_selected) {
	uint8_t y_pos = display_manager_get_menu_item_y(position);

	display_set_cursor(20, y_pos);

	/* Draw selection cursor */
	if (is_selected) {
		display_write_string("> ", DISPLAY_MENU_FONT, DISPLAY_WHITE);
	} else {
		display_write_string("  ", DISPLAY_MENU_FONT, DISPLAY_WHITE);
	}

	/* Draw menu item text */
	display_write_string((char*)item_text, DISPLAY_TITLE_FONT, DISPLAY_WHITE);
}

void display_manager_draw_scrollbar(uint8_t total_items, uint8_t visible_items, uint8_t scroll_position) {
	if (total_items <= visible_items) {
		return; /* No scrollbar needed */
	}

	uint8_t scrollbar_height = DISPLAY_HEIGHT - MENU_START_Y - 4;
	uint8_t thumb_height = (scrollbar_height * visible_items) / total_items;
	uint8_t thumb_position = MENU_START_Y +
			(scrollbar_height - thumb_height) * scroll_position / (total_items - visible_items);

	/* Draw scrollbar background */
	display_draw_rectangle(DISPLAY_WIDTH - 5, MENU_START_Y,
			DISPLAY_WIDTH - 3, DISPLAY_HEIGHT - 4, DISPLAY_WHITE);

	/* Draw scrollbar thumb */
	display_fill_rectangle(DISPLAY_WIDTH - 4, thumb_position,
			DISPLAY_WIDTH - 4, thumb_position + thumb_height, DISPLAY_WHITE);
}

void display_manager_clear_menu_item_area(uint8_t position) {
	uint8_t y_pos = display_manager_get_menu_item_y(position);
	display_fill_rectangle(20, y_pos, 30, y_pos + MENU_ITEM_HEIGHT - 1, DISPLAY_BLACK);
}

/* Helper functions for positioning */
uint8_t display_manager_get_menu_item_y(uint8_t position) {
	return MENU_START_Y + ((position + 1) * MENU_ITEM_HEIGHT);
}

uint8_t display_manager_get_menu_start_y(void) {
	return MENU_START_Y;
}

uint8_t display_manager_get_menu_item_height(void) {
	return MENU_ITEM_HEIGHT;
}

uint8_t display_manager_get_visible_items_count(void) {
	return VISIBLE_ITEMS;
}
