/*
 * menu_system.c
 *
 *  Created on: Jun 2, 2025
 *      Author: rohitimandi
 *  Updated: Added Single Player / Multiplayer support
 */

#include "Console_Peripherals/UI/menu_system.h"
#include "Console_Peripherals/Hardware/display_manager.h"
#include "Communication/serial_comm.h"

 /* Private function declarations */
static bool handle_navigation_up(MenuState* menu_state);
static bool handle_navigation_down(MenuState* menu_state);
static void update_selection_display(const MenuState* menu_state, uint8_t old_selection);
static const char* get_menu_title(MenuType menu_type);

/* Menu data definitions */
static MenuItem main_menu[] = {
    {"Single Player", 0, SCREEN_MENU, 0, GAME_MODE_SINGLE_PLAYER},
    {"Multi Player", 0, SCREEN_MENU, 0, GAME_MODE_MULTIPLAYER}
};

static MenuItem single_player_menu[] = {
    {"Snake Game", 0, SCREEN_GAME_SNAKE, 1, GAME_MODE_SINGLE_PLAYER},
    {"Pacman Game", 0, SCREEN_GAME_PACMAN, 1, GAME_MODE_SINGLE_PLAYER},
    {"Game 3", 0, SCREEN_GAME_3, 1, GAME_MODE_SINGLE_PLAYER},
    {"Game 4", 0, SCREEN_GAME_4, 1, GAME_MODE_SINGLE_PLAYER},
    {"Game 5", 0, SCREEN_GAME_5, 1, GAME_MODE_SINGLE_PLAYER}
};

static MenuItem multiplayer_menu[] = {
    {"Snake Game", 0, SCREEN_MP_GAME_SNAKE, 1, GAME_MODE_MULTIPLAYER},
    {"Pacman Game", 0, SCREEN_MP_GAME_PACMAN, 1, GAME_MODE_MULTIPLAYER},
    {"Game 3", 0, SCREEN_MP_GAME_3, 1, GAME_MODE_MULTIPLAYER},
    {"Game 4", 0, SCREEN_MP_GAME_4, 1, GAME_MODE_MULTIPLAYER},
    {"Game 5", 0, SCREEN_MP_GAME_5, 1, GAME_MODE_MULTIPLAYER}
};

/* Menu system initialization and cleanup */
void menu_system_init(MenuState* menu_state, MenuItem* items, uint8_t item_count) {
    if (!menu_state || !items || item_count == 0) {
        return;
    }

    /* Clear the menu state */
    memset(menu_state, 0, sizeof(MenuState));

    /* Limit item count to maximum */
    menu_state->item_count = (item_count <= MAX_MENU_ITEMS) ? item_count : MAX_MENU_ITEMS;

    /* Copy menu items */
    for (uint8_t i = 0; i < menu_state->item_count; i++) {
        menu_state->items[i] = items[i];
    }

    /* Initialize navigation state */
    menu_state->current_selection = 0;
    menu_state->scroll_position = 0;
    menu_state->needs_full_refresh = true;
    menu_state->current_menu_type = MENU_TYPE_MAIN;
    menu_state->previous_menu_type = MENU_TYPE_MAIN;
}

void menu_system_reset(MenuState* menu_state) {
    if (!menu_state) {
        return;
    }

    menu_state->current_selection = 0;
    menu_state->scroll_position = 0;
    menu_state->needs_full_refresh = true;
    menu_state->current_menu_type = MENU_TYPE_MAIN;
    menu_state->previous_menu_type = MENU_TYPE_MAIN;

    /* Clear all selection flags */
    for (uint8_t i = 0; i < menu_state->item_count; i++) {
        menu_state->items[i].selected = 0;
    }

}

/* Menu rendering functions */
bool menu_system_render(const MenuState* menu_state) {
    if (!menu_state || menu_state->item_count == 0) {
        return false;
    }

    display_manager_clear_screen();
    display_manager_draw_border();

    // Draw status bar on main menu screen without score
    bool wifi_connected = serial_comm_is_wifi_connected();
    display_manager_draw_status_bar(wifi_connected, 0, 0, false);

    /* Draw menu title based on current menu type */
    const char* title = get_menu_title(menu_state->current_menu_type);
    display_manager_draw_menu_title(title);

    /* Draw visible menu items */
    uint8_t visible_count = display_manager_get_visible_items_count();
    uint8_t display_count = (menu_state->item_count < visible_count) ?
        menu_state->item_count : visible_count;

    for (uint8_t i = 0; i < display_count; i++) {
        uint8_t menu_index = menu_state->scroll_position + i;
        if (menu_index >= menu_state->item_count) {
            break;
        }

        bool is_selected = (menu_index == menu_state->current_selection);
        display_manager_draw_menu_item(menu_state->items[menu_index].title, i, is_selected);
    }

    /* Draw scrollbar if needed */
    if (menu_state->item_count > visible_count) {
        display_manager_draw_scrollbar(menu_state->item_count, visible_count,
            menu_state->scroll_position);
    }

    display_manager_update();
}

bool menu_system_render_partial_update(const MenuState* menu_state, uint8_t old_selection) {
    if (!menu_state) {
        return false;
    }

    /* Update the changed items only if both are visible */
    update_selection_display(menu_state, old_selection);

    /* Update scrollbar */
    uint8_t visible_count = display_manager_get_visible_items_count();
    if (menu_state->item_count > visible_count) {
        display_manager_draw_scrollbar(menu_state->item_count, visible_count,
            menu_state->scroll_position);
    }

    display_manager_update();
}

/* Menu navigation functions */
bool menu_system_handle_navigation(MenuState* menu_state, MenuNavigation direction) {
    if (!menu_state || menu_state->item_count == 0) {
        return false;
    }

    uint8_t old_scroll_position = menu_state->scroll_position;
    bool changed = false;

    switch (direction) {
    case MENU_NAV_UP:
        changed = handle_navigation_up(menu_state);
        break;

    case MENU_NAV_DOWN:
        changed = handle_navigation_down(menu_state);
        break;

    case MENU_NAV_SELECT:
        menu_system_mark_selected_item(menu_state);
        return true; /* Always indicate change for selection */

    case MENU_NAV_BACK:
        menu_system_navigate_back(menu_state);
        return true;

    default:
        return false;
    }

    /* Determine if we need a full refresh */
    if (old_scroll_position != menu_state->scroll_position) {
        menu_state->needs_full_refresh = true;
    }
    else if (changed) {
        menu_state->needs_full_refresh = false;
    }

    return changed;
}

MenuItem menu_system_get_selected_item(const MenuState* menu_state) {
    MenuItem empty = { NULL, 0, SCREEN_ERROR, 0, GAME_MODE_SINGLE_PLAYER };

    if (!menu_state || !menu_system_is_selection_valid(menu_state)) {
        return empty;
    }

    return menu_state->items[menu_state->current_selection];
}

bool menu_system_is_selection_valid(const MenuState* menu_state) {
    return (menu_state && menu_state->current_selection < menu_state->item_count);
}

/* Menu state queries */
uint8_t menu_system_get_current_selection(const MenuState* menu_state) {
    return menu_state ? menu_state->current_selection : 0;
}

uint8_t menu_system_get_scroll_position(const MenuState* menu_state) {
    return menu_state ? menu_state->scroll_position : 0;
}

uint8_t menu_system_get_item_count(const MenuState* menu_state) {
    return menu_state ? menu_state->item_count : 0;
}

bool menu_system_needs_full_refresh(const MenuState* menu_state) {
    return menu_state ? menu_state->needs_full_refresh : true;
}

MenuType menu_system_get_current_menu_type(const MenuState* menu_state) {
    return menu_state ? menu_state->current_menu_type : MENU_TYPE_MAIN;
}

/* Menu state manipulation */
void menu_system_set_selection(MenuState* menu_state, uint8_t selection) {
    if (menu_state && selection < menu_state->item_count) {
        menu_state->current_selection = selection;
    }
}

void menu_system_mark_selected_item(MenuState* menu_state) {
    if (!menu_state || !menu_system_is_selection_valid(menu_state)) {
        return;
    }

    /* Clear all previous selections */
    for (uint8_t i = 0; i < menu_state->item_count; i++) {
        menu_state->items[i].selected = 0;
    }

    /* Mark current selection */
    menu_state->items[menu_state->current_selection].selected = 1;
}

void menu_system_clear_refresh_flag(MenuState* menu_state) {
    if (menu_state) {
        menu_state->needs_full_refresh = false;
    }
}

/* Menu navigation helpers */
void menu_system_navigate_to_single_player(MenuState* menu_state) {
    if (!menu_state) return;

    menu_state->previous_menu_type = menu_state->current_menu_type;
    menu_state->current_menu_type = MENU_TYPE_SINGLE_PLAYER;

    MenuItem* items;
    uint8_t count;
    menu_system_get_single_player_menu(&items, &count);
    menu_system_init(menu_state, items, count);
    menu_state->current_menu_type = MENU_TYPE_SINGLE_PLAYER;
    menu_state->previous_menu_type = MENU_TYPE_MAIN;
}

bool menu_system_navigate_to_multiplayer(MenuState* menu_state) {
    if (!menu_state) return false;

    // Check WiFi connection first - return false if not connected
    // Game controller will handle the error state
    if (!serial_comm_is_wifi_connected()) {
        return false;
    }

    menu_state->previous_menu_type = menu_state->current_menu_type;
    menu_state->current_menu_type = MENU_TYPE_MULTIPLAYER;

    MenuItem* items;
    uint8_t count;
    menu_system_get_multiplayer_menu(&items, &count);
    menu_system_init(menu_state, items, count);
    menu_state->current_menu_type = MENU_TYPE_MULTIPLAYER;
    menu_state->previous_menu_type = MENU_TYPE_MAIN;

    return true;
}

void menu_system_navigate_back(MenuState* menu_state) {
    if (!menu_state) return;

    MenuType target_menu = menu_state->previous_menu_type;

    if (target_menu == MENU_TYPE_MAIN) {
        menu_system_navigate_to_main_menu(menu_state);
    }
    else if (target_menu == MENU_TYPE_SINGLE_PLAYER) {
        menu_system_navigate_to_single_player(menu_state);
    }
    else if (target_menu == MENU_TYPE_MULTIPLAYER) {
        menu_system_navigate_to_multiplayer(menu_state);
    }
    // Add other back navigation cases as needed
}

void menu_system_navigate_to_main_menu(MenuState* menu_state) {
    if (!menu_state) return;

    menu_state->previous_menu_type = menu_state->current_menu_type;
    menu_state->current_menu_type = MENU_TYPE_MAIN;

    MenuItem* items;
    uint8_t count;
    menu_system_get_main_menu(&items, &count);
    menu_system_init(menu_state, items, count);
    menu_state->current_menu_type = MENU_TYPE_MAIN;
}

/* Utility functions */
bool menu_system_is_item_visible(const MenuState* menu_state, uint8_t item_index) {
    if (!menu_state) {
        return false;
    }

    uint8_t visible_count = display_manager_get_visible_items_count();
    return (item_index >= menu_state->scroll_position &&
        item_index < menu_state->scroll_position + visible_count);
}

uint8_t menu_system_get_visible_position(const MenuState* menu_state, uint8_t item_index) {
    if (!menu_state || !menu_system_is_item_visible(menu_state, item_index)) {
        return 0;
    }

    return item_index - menu_state->scroll_position;
}

/* Game menu data functions */
void menu_system_get_main_menu(MenuItem** menu_ptr, uint8_t* size_ptr) {
    if (!menu_ptr || !size_ptr) {
        if (menu_ptr) *menu_ptr = NULL;
        if (size_ptr) *size_ptr = 0;
        return;
    }

    *menu_ptr = main_menu;
    *size_ptr = sizeof(main_menu) / sizeof(main_menu[0]);
}

void menu_system_get_single_player_menu(MenuItem** menu_ptr, uint8_t* size_ptr) {
    if (!menu_ptr || !size_ptr) {
        if (menu_ptr) *menu_ptr = NULL;
        if (size_ptr) *size_ptr = 0;
        return;
    }

    *menu_ptr = single_player_menu;
    *size_ptr = sizeof(single_player_menu) / sizeof(single_player_menu[0]);
}

void menu_system_get_multiplayer_menu(MenuItem** menu_ptr, uint8_t* size_ptr) {
    if (!menu_ptr || !size_ptr) {
        if (menu_ptr) *menu_ptr = NULL;
        if (size_ptr) *size_ptr = 0;
        return;
    }

    *menu_ptr = multiplayer_menu;
    *size_ptr = sizeof(multiplayer_menu) / sizeof(multiplayer_menu[0]);
}

/* Legacy compatibility functions - for backward compatibility */
void menu_system_get_game_menu(MenuItem** menu_ptr, uint8_t* size_ptr) {
    // For backward compatibility, return main menu to start with new hierarchical system
    menu_system_get_main_menu(menu_ptr, size_ptr);
}

const MenuItem* menu_system_get_default_game_menu(void) {
    return main_menu;
}

uint8_t menu_system_get_default_game_menu_size(void) {
    return sizeof(main_menu) / sizeof(main_menu[0]);
}

/* Private helper functions */
static const char* get_menu_title(MenuType menu_type) {
    switch (menu_type) {
    case MENU_TYPE_MAIN:
        return "Select Gameplay Mode";
    case MENU_TYPE_SINGLE_PLAYER:
        return "Single Player Games";
    case MENU_TYPE_MULTIPLAYER:
        return "Multiplayer Games";
    default:
        return "Menu";
    }
}

static bool handle_navigation_up(MenuState* menu_state) {
    if (menu_state->current_selection > 0) {
        menu_state->current_selection--;

        /* Check if we need to scroll up */
        if (menu_state->current_selection < menu_state->scroll_position) {
            menu_state->scroll_position = menu_state->current_selection;
        }

        return true;
    }

    return false; /* Already at the top */
}

static bool handle_navigation_down(MenuState* menu_state) {
    if (menu_state->current_selection < menu_state->item_count - 1) {
        menu_state->current_selection++;

        /* Check if we need to scroll down */
        uint8_t visible_count = display_manager_get_visible_items_count();
        if (menu_state->current_selection >= menu_state->scroll_position + visible_count) {
            menu_state->scroll_position = menu_state->current_selection - visible_count + 1;
        }

        return true;
    }

    return false; /* Already at the bottom */
}

static void update_selection_display(const MenuState* menu_state, uint8_t old_selection) {
    /* Check if old selection is visible and update it */
    if (menu_system_is_item_visible(menu_state, old_selection)) {
        uint8_t old_pos = menu_system_get_visible_position(menu_state, old_selection);
        display_manager_clear_menu_item_area(old_pos);
        display_manager_draw_menu_item(menu_state->items[old_selection].title, old_pos, false);
    }

    /* Check if new selection is visible and update it */
    if (menu_system_is_item_visible(menu_state, menu_state->current_selection)) {
        uint8_t new_pos = menu_system_get_visible_position(menu_state, menu_state->current_selection);
        display_manager_clear_menu_item_area(new_pos);
        display_manager_draw_menu_item(menu_state->items[menu_state->current_selection].title,
            new_pos, true);
    }
}
