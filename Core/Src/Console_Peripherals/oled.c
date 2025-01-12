/*
 * oled.c
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/oled.h"

static MenuItem game_menu[] = {
    {"Snake Game", 1},
	{"Game 2", 0},
	{"Game 3", 0},
	{"Game 4", 0},
	{"Game 5", 0},
    // Add more games here
};

static const uint8_t NUM_MENU_ITEMS = sizeof(game_menu) / sizeof(MenuItem);
static uint8_t menu_scroll_position = 0;  // Top visible item index
static uint8_t current_cursor_item = 0;         // Currently selected item index

/* Private functions */
static void oled_show_welcome(char* str1, char* str2);

/* Initializes SPI and the OLED */
void oled_init(){
	display_init();
	add_delay(10);
	display_clear();
}


/* Displays a string on the screen */
//void oled_display_string(char* str, FontDef Font, SSD1306_COLOR color) {
//	  display_set_cursor(10, 20);
//	  ssd1306_WriteString(str, Font, color);
//	  display_update();
//	  add_delay(500);
//}

static void oled_show_welcome(char* str1, char* str2) {
    display_clear();
    display_draw_border();

    // Welcome message
    display_write_string_centered(str1, Font_7x10, 20, DISPLAY_WHITE);
    display_write_string_centered(str2, Font_7x10, 35, DISPLAY_WHITE);

    display_update();
    add_delay(5000);  // Show welcome screen for 3 seconds
}

void oled_show_menu(MenuItem* items, uint8_t num_items) {
    display_clear();
    display_draw_border();

    // Menu title
    display_write_string_centered("Select Game", Font_7x10, 10, DISPLAY_WHITE);

    // Draw visible menu items
    uint8_t display_count = (num_items < VISIBLE_ITEMS) ? num_items : VISIBLE_ITEMS;

    for(uint8_t i = 0; i < display_count; i++) {
        uint8_t menu_index = menu_scroll_position + i;
        if(menu_index >= num_items) break;

        display_set_cursor(20, MENU_START_Y + (i * MENU_ITEM_HEIGHT));

        // Show selection cursor only for selected item
        if(menu_index == current_cursor_item) {
            display_write_string("> ", Font_7x10, DISPLAY_WHITE);
        } else {
            display_write_string("  ", Font_7x10, DISPLAY_WHITE);
        }
        display_write_string(items[menu_index].title, Font_7x10, DISPLAY_WHITE);
    }

    // Draw scrollbar if needed
    if(num_items > VISIBLE_ITEMS) {
        uint8_t scrollbar_height = SCREEN_HEIGHT - MENU_START_Y - 4;
        uint8_t thumb_height = (scrollbar_height * VISIBLE_ITEMS) / num_items;
        uint8_t thumb_position = MENU_START_Y +
            (scrollbar_height - thumb_height) * menu_scroll_position / (num_items - VISIBLE_ITEMS);

        // Draw scrollbar background
        display_draw_rectangle(SCREEN_WIDTH-5, MENU_START_Y,
                          SCREEN_WIDTH-3, SCREEN_HEIGHT-4, DISPLAY_WHITE);

        // Draw scrollbar thumb
        display_fill_rectangle(SCREEN_WIDTH-4, thumb_position,
                          SCREEN_WIDTH-4, thumb_position + thumb_height, DISPLAY_WHITE);
    }

    display_update();
}

void oled_menu_handle_input(JoystickStatus js_status) {
    if(!js_status.is_new) return;

    bool menu_updated = false;

    // Handle up/down navigation
    if(js_status.direction == JS_DIR_UP && current_cursor_item > 0) {
        current_cursor_item--;
        if(current_cursor_item < menu_scroll_position) {
            menu_scroll_position = current_cursor_item;
        }
        menu_updated = true;
    }
    else if(js_status.direction == JS_DIR_DOWN && current_cursor_item < NUM_MENU_ITEMS - 1) {
        current_cursor_item++;
        if(current_cursor_item >= menu_scroll_position + VISIBLE_ITEMS) {
            menu_scroll_position = current_cursor_item - VISIBLE_ITEMS + 1;
        }
        menu_updated = true;
    }

    // Handle selection via button press
    if(js_status.button) {
        // Clear all selections first
        for(uint8_t i = 0; i < NUM_MENU_ITEMS; i++) {
            game_menu[i].selected = 0;
        }
        // Set new selection
        game_menu[current_cursor_item].selected = 1;
        menu_updated = true;
    }

    // Update display if needed
    if(menu_updated) {
        oled_show_menu(game_menu, NUM_MENU_ITEMS);
    }
}

void oled_show_screen(ScreenType screen) {
    switch(screen) {
        case SCREEN_WELCOME:
            oled_show_welcome("Welcome to", "Game Console!");
            break;

        case SCREEN_MENU:
            oled_show_menu(game_menu, NUM_MENU_ITEMS);
            break;

        case SCREEN_GAME_SNAKE:
            // Initialize snake game screen
            break;

        default:
            break;
    }
}
