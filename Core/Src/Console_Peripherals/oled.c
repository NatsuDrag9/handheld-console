/*
 * oled.c
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/oled.h"
#include "Game_Engine/game_engine.h"
#include "Game_Engine/Games/snake_game.h"
#include "Game_Engine/Games/pacman_game.h"
#include "Utils/debug_conf.h"

static MenuItem current_menu[MAX_MENU_ITEMS] = { 0 };
static uint8_t current_menu_size = 0;
static uint8_t menu_scroll_position = 0;  // Top visible item index
static uint8_t current_cursor_item = 0;         // Currently selected item index
static bool is_in_game = false;

/* Private functions */
static void oled_show_welcome(char* str1, char* str2);
static void oled_display_string(char* str, FontDef Font, DisplayColor color);
static void oled_draw_scrollbar(uint8_t num_items);
static bool handle_menu_navigation(JoystickStatus js_status);
static void handle_menu_selection(void);

/* Displays a string on the screen */
static void oled_display_string(char* str, FontDef Font, DisplayColor color) {
    display_clear();
    display_write_string_centered(str, Font_7x10, 25, DISPLAY_WHITE);
    display_update();
    add_delay(100);
}

static void oled_show_welcome(char* str1, char* str2) {
    display_clear();
    display_draw_border();

    // Welcome message
    display_write_string_centered(str1, Font_7x10, 20, DISPLAY_WHITE);
    display_write_string_centered(str2, Font_7x10, 35, DISPLAY_WHITE);

    display_update();
    add_delay(3000);  // Show welcome screen for 3 seconds
}

/* Private function for drawing scrollbar */
static void oled_draw_scrollbar(uint8_t num_items) {
    if (num_items > VISIBLE_ITEMS) {
        uint8_t scrollbar_height = SCREEN_HEIGHT - MENU_START_Y - 4;
        uint8_t thumb_height = (scrollbar_height * VISIBLE_ITEMS) / num_items;
        uint8_t thumb_position = MENU_START_Y +
            (scrollbar_height - thumb_height) * menu_scroll_position / (num_items - VISIBLE_ITEMS);

        // Draw scrollbar background
        display_draw_rectangle(SCREEN_WIDTH - 5, MENU_START_Y,
            SCREEN_WIDTH - 3, SCREEN_HEIGHT - 4, DISPLAY_WHITE);

        // Draw scrollbar thumb
        display_fill_rectangle(SCREEN_WIDTH - 4, thumb_position,
            SCREEN_WIDTH - 4, thumb_position + thumb_height, DISPLAY_WHITE);
    }
}

static bool handle_menu_navigation(JoystickStatus js_status) {
    switch (js_status.direction) {
    case JS_DIR_UP:
        if (current_cursor_item > 0) {
            current_cursor_item--;
            if (current_cursor_item < menu_scroll_position) {
                menu_scroll_position = current_cursor_item;
            }
            return true;
        }
        break;

    case JS_DIR_DOWN:
        if (current_cursor_item < current_menu_size - 1) {
            current_cursor_item++;
            if (current_cursor_item >= menu_scroll_position + VISIBLE_ITEMS) {
                menu_scroll_position = current_cursor_item - VISIBLE_ITEMS + 1;
            }
            return true;
        }
        break;
    }
    return false;
}

static void handle_menu_selection(void) {
    // Clear previous selections
    for (uint8_t i = 0; i < current_menu_size; i++) {
        current_menu[i].selected = 0;
    }

    // Set new selection
    current_menu[current_cursor_item].selected = 1;

    // Handle selected item
    MenuItem selected = current_menu[current_cursor_item];
    if (selected.title != NULL) {
        if (selected.is_game) {
            oled_set_is_game_active(true);
        }
        oled_show_screen(selected.screen);
    }
}


/* Clears the oled screen */
void oled_clear_screen() {
    display_clear();
    display_update();
}

/* Initializes SPI, OLED and sets the menu */
void oled_init(MenuItem* menu, uint8_t menu_size) {
    display_init();
    add_delay(10);
    display_clear();

    if (menu == NULL || menu_size == 0) {
        oled_display_string("Menu Init Error", Font_7x10, DISPLAY_WHITE);
        return;
    }

    // Zero out current_menu first
    memset(current_menu, 0, sizeof(current_menu));

    // Reset navigation state - we were initializing it to 0 but it's getting changed somewhere
    current_cursor_item = 0;
    menu_scroll_position = 0;

    // Limit the number of items to MAX_MENU_ITEMS
    current_menu_size = (menu_size <= MAX_MENU_ITEMS) ? menu_size : MAX_MENU_ITEMS;

    // DEBUG_PRINTF("Menu Size: %u\n", current_menu_size);

    // Copy menu items
    for (uint8_t i = 0; i < current_menu_size; i++) {
        current_menu[i] = menu[i];
        // DEBUG_PRINTF("Oled Item %u: Title=%s, Selected=%u\n",
        // 	                     i, current_menu[i].title, current_menu[i].selected);
    }
}

void oled_show_menu(MenuItem* items, uint8_t num_items) {
    display_clear();
    display_draw_border();

    // Menu title
    display_write_string_centered("Select Game", Font_7x10, 10, DISPLAY_WHITE);

    // Draw visible menu items
    uint8_t display_count = (num_items < VISIBLE_ITEMS) ? num_items : VISIBLE_ITEMS;

    for (uint8_t i = 0; i < display_count; i++) {
        uint8_t menu_index = menu_scroll_position + i;
        if (menu_index >= num_items) break;

        display_set_cursor(20, MENU_START_Y + (i * MENU_ITEM_HEIGHT));

        // Show selection cursor only for selected item
        if (menu_index == current_cursor_item) {
            display_write_string("> ", Font_7x10, DISPLAY_WHITE);
        }
        else {
            display_write_string("  ", Font_7x10, DISPLAY_WHITE);
        }
        display_write_string(items[menu_index].title, Font_7x10, DISPLAY_WHITE);
    }

    // Draw scrollbar if needed
    oled_draw_scrollbar(num_items);

    display_update();
}

void oled_menu_handle_input(JoystickStatus js_status) {
    if (!js_status.is_new) {
        return;
    }

    // Handle navigation
    if (handle_menu_navigation(js_status)) {
        oled_show_menu(current_menu, current_menu_size);
        return;
    }

    // Handle selection
    if (js_status.button) {
        handle_menu_selection();
    }
}

void oled_run_game(void) {
    if (!is_in_game) {
        return;
    }

    static uint32_t last_frame_time = 0;
    uint32_t current_time = get_current_ms();

    if (current_time - last_frame_time >= FRAME_RATE) {
        JoystickStatus js_status = joystick_get_status();
        MenuItem selected = oled_get_selected_menu_item();

        GameEngine* current_engine = NULL;
        switch (selected.screen) {
        case SCREEN_GAME_SNAKE:
            current_engine = &snake_game_engine;
            break;
        case SCREEN_GAME_PACMAN:
            current_engine = &pacman_game_engine;
            break;
        default:
            break;
        }

        if (current_engine) {
            game_engine_update(current_engine, js_status);
            display_clear();
            game_engine_render(current_engine);

            // Handle game over state and timing after rendering
            if (current_engine->base_state.game_over && current_engine->countdown_over) {
                game_engine_cleanup(current_engine);
                oled_set_is_game_active(false);
                oled_show_screen(SCREEN_MENU);
            }
        }

        last_frame_time = current_time;
    }
}


void oled_show_screen(ScreenType screen) {
    switch (screen) {
    case SCREEN_WELCOME:
        oled_show_welcome("Welcome to", "Game Console!");
        break;

    case SCREEN_MENU:
        if (current_menu_size == 0) {
            oled_display_string("Menu Unavailable", Font_7x10, DISPLAY_WHITE);
            break;
        }
        oled_show_menu(current_menu, current_menu_size);
        break;

    case SCREEN_GAME_SNAKE:
        // Initialize snake game screen
        // DEBUG_PRINTF(false, "Initializing snake game...\n");
        game_engine_init(&snake_game_engine);
        break;
    case SCREEN_GAME_PACMAN:
        // Initialize snake game screen
        // DEBUG_PRINTF(false, "Initializing snake game...\n");
        game_engine_init(&pacman_game_engine);
        break;

    default:
        break;
    }
}

MenuItem oled_get_selected_menu_item() {
    for (uint8_t i = 0; i < current_menu_size; i++) {
        if (current_menu[i].selected == 1) {
            return current_menu[i];
        }
    }
    // Return an empty MenuItem if nothing is selected
    MenuItem empty = { NULL, 0 };
    return empty;
}

bool oled_is_game_active(void) {
    return is_in_game;
}

void oled_set_is_game_active(bool is_active) {
    is_in_game = is_active;
}

// Primarily used in unit-testing
uint8_t oled_get_current_menu_size(void) {
    return current_menu_size;
}

uint8_t oled_get_current_cursor_item(void) {
    return current_cursor_item;
}
