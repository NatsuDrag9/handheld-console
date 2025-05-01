/*
 * oled.c
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/oled.h"
#include "Sprites/status_bar_sprite.h" // Didn't include in header as this is local to the implementation file
#include "Utils/debug_conf.h"

static MenuItem current_menu[MAX_MENU_ITEMS] = { 0 };
static uint8_t current_menu_size = 0;
static uint8_t menu_scroll_position = 0;  // Top visible item index
static uint8_t current_cursor_item = 0;   // Currently selected item index
static bool is_in_game = false;
//static uint8_t previous_cursor_item = 0;   // Previously selected item
static uint32_t last_menu_refresh_time = 0; // For throttling menu refreshes

/* Private functions */
static void oled_show_welcome(char* str1, char* str2);
static void draw_status_bar(void);
static void oled_display_string(char* str, FontDef Font, DisplayColor color);
static void oled_draw_scrollbar(uint8_t num_items);
static bool handle_menu_navigation(uint8_t direction);
static void handle_menu_selection(void);
static GameEngine* get_current_game_engine(void);
static bool handle_game_button_actions(GameEngine* engine);
static void process_game_loop(GameEngine* engine);
static bool handle_game_over(GameEngine* engine);
static void update_menu_selection(uint8_t old_item, uint8_t new_item);

/* Displays a string on the screen */
static void oled_display_string(char* str, FontDef Font, DisplayColor color) {
    display_clear();
    display_write_string_centered(str, DISPLAY_FONT, 25, DISPLAY_WHITE);
    display_update();
    add_delay(100);
}

static void oled_show_welcome(char* str1, char* str2) {
    display_clear();
    display_draw_border();

    // Draw status bar with WiFi status and other indicators
    draw_status_bar();

    // Welcome message for OLED
#if defined(DISPLAY_MODULE_OLED)
    display_write_string_centered(str1, DISPLAY_FONT, SEPARATOR_LINE_Y + 10, DISPLAY_WHITE);
    display_write_string_centered(str2, DISPLAY_FONT, SEPARATOR_LINE_Y + 25, DISPLAY_WHITE);
#elif defined(DISPLAY_MODULE_LCD)
    display_write_string_centered(str1, DISPLAY_TITLE_FONT, SEPARATOR_LINE_Y + 10, DISPLAY_WHITE);
    display_write_string_centered(str2, DISPLAY_TITLE_FONT, SEPARATOR_LINE_Y + 40, DISPLAY_WHITE);
#endif

    display_update();
    add_delay(3000);  // Show welcome screen for 3 seconds
}

static void draw_status_bar(void) {
    // Draw horizontal separator line
    display_draw_horizontal_line(0, SEPARATOR_LINE_Y, SCREEN_WIDTH, DISPLAY_WHITE);

    // Draw WiFi icon based on connection status
    if (serial_comm_is_wifi_connected()) {
        // Draw connected WiFi icon
    	display_draw_bitmap(SCREEN_WIDTH - 15, 3, wifi_connected_icon,
    	                           STATUS_ICON_WIDTH, STATUS_ICON_HEIGHT, DISPLAY_WHITE);
    } else {
        // Draw disconnected WiFi icon
    	display_draw_bitmap(SCREEN_WIDTH - 15, 3, wifi_disconnected_icon,
    	                           STATUS_ICON_WIDTH, STATUS_ICON_HEIGHT, DISPLAY_WHITE);
    }

    // Draw score if in game
    if (is_in_game) {
        GameEngine* current_engine = get_current_game_engine();
        if (current_engine) {
//            char score_str[16];
//            sprintf(score_str, "Score: %d", current_engine->base_state.score);
            char status_text[32];
            snprintf(status_text, sizeof(status_text), "Score: %lu Lives: %d",
            		current_engine->base_state.score,
					current_engine->base_state.lives);
            display_set_cursor(5, 2);
            display_write_string(status_text, DISPLAY_MENU_CURSOR_FONT, DISPLAY_WHITE);
        }
    }
}

/* Private function for drawing scrollbar */
static void oled_draw_scrollbar(uint8_t num_items) {
    if (num_items > VISIBLE_ITEMS) {

        coord_t scrollbar_height = SCREEN_HEIGHT - MENU_START_Y - 4;
        coord_t thumb_height = (scrollbar_height * VISIBLE_ITEMS) / num_items;
        coord_t thumb_position = MENU_START_Y +
            (scrollbar_height - thumb_height) * menu_scroll_position / (num_items - VISIBLE_ITEMS);

        // Draw scrollbar background
        display_draw_rectangle(SCREEN_WIDTH - 5, MENU_START_Y,
            SCREEN_WIDTH - 3, SCREEN_HEIGHT - 4, DISPLAY_WHITE);

        // Draw scrollbar thumb
        display_fill_rectangle(SCREEN_WIDTH - 4, thumb_position,
            SCREEN_WIDTH - 4, thumb_position + thumb_height, DISPLAY_WHITE);
    }
}

static bool handle_menu_navigation(uint8_t direction) {
    uint8_t old_cursor_item = current_cursor_item;
    uint8_t old_scroll_position = menu_scroll_position;
    bool needs_full_refresh = false;

    switch (direction) {
    case JS_DIR_UP:
    case DPAD_DIR_UP:
        if (current_cursor_item > 0) {
            current_cursor_item--;
            if (current_cursor_item < menu_scroll_position) {
                menu_scroll_position = current_cursor_item;
                needs_full_refresh = true; // Need full refresh when scrolling
            }
        } else {
            // Already at the top, do nothing
            return false; // No change
        }
        break;

    case JS_DIR_DOWN:
    case DPAD_DIR_DOWN:
        if (current_cursor_item < current_menu_size - 1) {
            current_cursor_item++;
            if (current_cursor_item >= menu_scroll_position + VISIBLE_ITEMS) {
                menu_scroll_position = current_cursor_item - VISIBLE_ITEMS + 1;
                needs_full_refresh = true; // Need full refresh when scrolling
            }
        } else {
            // Already at the bottom, do nothing
            return false; // No change
        }
        break;

    default:
        return false; // No valid direction
    }

    // If we scrolled, always do a full refresh to avoid the double cursor issue
    if (old_scroll_position != menu_scroll_position) {
        return true;
    }

    // Redraw only the changed menu items if we didn't scroll
    if (old_cursor_item != current_cursor_item && !needs_full_refresh) {
        // Only update the changed items rather than the whole menu
        update_menu_selection(old_cursor_item, current_cursor_item);
        return false; // Return false to indicate no full refresh needed
    }

    return needs_full_refresh;
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
        oled_display_string("Menu Init Error", DISPLAY_FONT, DISPLAY_WHITE);
        return;
    }

    // Zero out current_menu first
    memset(current_menu, 0, sizeof(current_menu));

    // Reset navigation state - we were initializing it to 0 but it's getting changed somewhere
    current_cursor_item = 0;
    menu_scroll_position = 0;

    // Limit the number of items to MAX_MENU_ITEMS
    current_menu_size = (menu_size <= MAX_MENU_ITEMS) ? menu_size : MAX_MENU_ITEMS;

    // Copy menu items
    for (uint8_t i = 0; i < current_menu_size; i++) {
        current_menu[i] = menu[i];
    }
}

void oled_show_menu(MenuItem* items, uint8_t num_items) {
    display_clear();
    display_draw_border();

    // Draw status bar with WiFi status and other indicators
    draw_status_bar();

    // Menu title
    display_write_string_centered("Select Game", DISPLAY_TITLE_FONT, MENU_TITLE_Y, DISPLAY_WHITE);

    // Draw visible menu items
    uint8_t display_count = (num_items < VISIBLE_ITEMS) ? num_items : VISIBLE_ITEMS;

    for (uint8_t i = 0; i < display_count; i++) {
        uint8_t menu_index = menu_scroll_position + i;
        if (menu_index >= num_items) break;

        display_set_cursor(20, MENU_START_Y + ((i+1) * MENU_ITEM_HEIGHT));

        // Show selection cursor only for selected item
        if (menu_index == current_cursor_item) {
            display_write_string("> ", DISPLAY_MENU_CURSOR_FONT, DISPLAY_WHITE);
        }
        else {
            display_write_string("  ", DISPLAY_MENU_CURSOR_FONT, DISPLAY_WHITE);
        }
        display_write_string(items[menu_index].title, DISPLAY_TITLE_FONT, DISPLAY_WHITE);
    }

    // Draw scrollbar if needed
    oled_draw_scrollbar(num_items);

    display_update();
}

static void update_menu_selection(uint8_t old_item, uint8_t new_item) {
    // Only update if the items are visible
    bool old_visible = (old_item >= menu_scroll_position &&
                        old_item < menu_scroll_position + VISIBLE_ITEMS);
    bool new_visible = (new_item >= menu_scroll_position &&
                        new_item < menu_scroll_position + VISIBLE_ITEMS);

    if (old_visible) {
        // Clear old selection cursor
        uint8_t y_pos = MENU_START_Y + ((old_item - menu_scroll_position + 1) * MENU_ITEM_HEIGHT);
        display_fill_rectangle(20, y_pos, 30, y_pos + MENU_ITEM_HEIGHT - 1, DISPLAY_BLACK);
        display_set_cursor(20, y_pos);
        display_write_string("  ", DISPLAY_MENU_CURSOR_FONT, DISPLAY_WHITE);
    }

    if (new_visible) {
        // Draw new selection cursor
        uint8_t y_pos = MENU_START_Y + ((new_item - menu_scroll_position + 1) * MENU_ITEM_HEIGHT);
        display_fill_rectangle(20, y_pos, 30, y_pos + MENU_ITEM_HEIGHT - 1, DISPLAY_BLACK);
        display_set_cursor(20, y_pos);
        display_write_string("> ", DISPLAY_MENU_CURSOR_FONT, DISPLAY_WHITE);
    }

    // Draw scrollbar thumb if needed
    oled_draw_scrollbar(current_menu_size);

    // Update the display
    display_update();
}

void oled_menu_handle_input(JoystickStatus js_status) {
    // Get D-pad status
    DPAD_STATUS dpad_status = d_pad_get_status();
    uint8_t dpad_changed = d_pad_direction_changed();
    uint32_t current_time = get_current_ms();

    // Ensure cursor is within valid bounds (protection against potential corruption)
    if (current_cursor_item >= current_menu_size) {
    	current_cursor_item = current_menu_size > 0 ? current_menu_size - 1 : 0;
    }

    // Throttle menu updates to avoid too frequent refreshes
    // Only process input if enough time has passed since last refresh
    if (current_time - last_menu_refresh_time < MENU_REFRESH_THROTTLE) {
        return;
    }

    // Check push button 1 first (highest priority)
    uint8_t pb1_state = pb1_get_state();
    if (pb1_state) {
        handle_menu_selection();
        last_menu_refresh_time = current_time;
        return;
    }

    // Then check joystick button
    if (js_status.is_new && js_status.button) {
        handle_menu_selection();
        last_menu_refresh_time = current_time;
        return;
    }

    // Handle D-pad navigation if it has new input
    if (dpad_changed) {
        bool needs_full_refresh = handle_menu_navigation(dpad_status.direction);
        if (needs_full_refresh) {
            oled_show_menu(current_menu, current_menu_size);
        }
        last_menu_refresh_time = current_time;
        return;
    }

    // Handle joystick navigation if it has new input
    if (js_status.is_new) {
        bool needs_full_refresh = handle_menu_navigation(js_status.direction);
        if (needs_full_refresh) {
            oled_show_menu(current_menu, current_menu_size);
        }
        last_menu_refresh_time = current_time;
        return;
    }
}

// Gets the current game engine based on the selected menu item
static GameEngine* get_current_game_engine(void) {
    MenuItem selected = oled_get_selected_menu_item();

    switch (selected.screen) {
    case SCREEN_GAME_SNAKE:
        return &snake_game_engine;
    case SCREEN_GAME_PACMAN:
        return &pacman_game_engine;
    default:
        return NULL;
    }
}

// Handles actions requested by button presses
// Returns: true to continue game loop, false to exit early
static bool handle_game_button_actions(GameEngine* engine) {
    // Check for game reset request
    if (engine->base_state.is_reset) {
        game_engine_cleanup(engine);
        game_engine_init(engine);
        engine->base_state.is_reset = false;  // Clear flag
    }

    // Check for return to main menu request
    if (engine->return_to_main_menu) {
        add_delay(100);  // Add a small delay for display processing

        game_engine_cleanup(engine);
        engine->return_to_main_menu = false;  // Clear flag

        // Exit game mode AFTER cleanup
        oled_set_is_game_active(false);
        oled_show_screen(SCREEN_MENU);
        return false;  // Signal to exit oled_run_game
    }

    return true;  // Continue with game loop
}

static void process_game_loop(GameEngine* engine) {
    void* controller_data;

    if (engine->is_d_pad_game) {
        // For D-pad games, get input from dedicated D-pad interface
        DPAD_STATUS dpad_status = d_pad_get_status();
        controller_data = &dpad_status;
    }
    else {
        // For joystick games, get input from joystick interface
        JoystickStatus js_status = joystick_get_status();
        controller_data = &js_status;
    }

    // Update game state with the appropriate controller data
    game_engine_update(engine, controller_data);

    // Draw status bar with updated game score
    draw_status_bar();

    // Render the game
//    display_clear();
    game_engine_render(engine);
}

// Handle game over condition
// Returns: true to continue, false if game over and transitioned to menu
static bool handle_game_over(GameEngine* engine) {
    if (engine->base_state.game_over && engine->countdown_over) {
        game_engine_cleanup(engine);
        oled_set_is_game_active(false);
        oled_show_screen(SCREEN_MENU);

        return false;  // Signal to exit oled_run_game
    }

    return true;  // Continue with game loop
}

void oled_run_game(void) {
    if (!is_in_game) {
        return;
    }

    static uint32_t last_frame_time = 0;
    uint32_t current_time = get_current_ms();

    if (current_time - last_frame_time >= FRAME_RATE) {
        GameEngine* current_engine = get_current_game_engine();

        if (current_engine) {
            // Handle button-triggered actions (reset, return to menu)
            // If it returns false, we exit early
            if (!handle_game_button_actions(current_engine)) {
                last_frame_time = current_time;
                return;
            }

            // Regular game processing
            process_game_loop(current_engine);

            // Handle game over condition
            // If it returns false, the game is over and we've transitioned to menu
            if (!handle_game_over(current_engine)) {
                last_frame_time = current_time;
                return;
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
                oled_display_string("Menu Unavailable", DISPLAY_FONT, DISPLAY_WHITE);
                break;
            }
            oled_show_menu(current_menu, current_menu_size);
            break;

    case SCREEN_GAME_SNAKE:
        // Initialize snake game screen
        game_engine_init(&snake_game_engine);
        break;
    case SCREEN_GAME_PACMAN:
        // Initialize pacman game screen
        game_engine_init(&pacman_game_engine);
        break;

    default:
        // Default to menu screen in all other cases
        if (current_menu_size == 0) {
            oled_display_string("Menu Unavailable", DISPLAY_FONT, DISPLAY_WHITE);
            break;
        }

        // Make sure the game mode is disabled when returning to menu
        oled_set_is_game_active(false);

        // Reset menu navigation state if needed
        current_cursor_item = 0;
        menu_scroll_position = 0;

        // Display the menu
        oled_show_menu(current_menu, current_menu_size);
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
