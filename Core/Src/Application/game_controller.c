/*
 * game_controller.c
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#include "Application/game_controller.h"
#include "Console_Peripherals/Hardware/display_manager.h"
#include "Console_Peripherals/Hardware/d_pad.h"
#include "Console_Peripherals/Hardware/serial_comm.h"
//#include "Game_Engine/Games/snake_game.h"
#include "Game_Engine/Games/Single_Player/snake_game.h"
#include "Game_Engine/Games/pacman_game.h"
#include "Utils/debug_conf.h"

#ifdef UNITY_TEST
#define add_delay(x)
#define get_current_ms() 0
#define pb1_get_state() 0
#else
extern void add_delay(uint32_t ms);
extern uint32_t get_current_ms(void);
extern uint8_t pb1_get_state(void);
#endif

/* Private state variables */
static AppState current_app_state = APP_STATE_WELCOME;
static MenuState main_menu;
static ScreenType current_game_screen = SCREEN_MENU;
static uint32_t last_frame_time = 0;
static uint32_t last_status_update_time = 0;
static uint32_t last_menu_refresh_time = 0;

/* Private function declarations */
static void handle_welcome_input(JoystickStatus js_status);
static void handle_menu_navigation(uint8_t direction);
static void handle_menu_selection(void);
static bool handle_game_button_actions(GameEngine* engine);
static void process_game_loop(GameEngine* engine);
static bool handle_game_over(GameEngine* engine);
static uint8_t convert_joystick_to_menu_nav(JoystickStatus js_status);
static uint8_t convert_dpad_to_menu_nav(void);

void game_controller_init(MenuItem* menu_items, uint8_t menu_count) {
    /* Initialize display manager */
    display_manager_init();

    /* Initialize menu system */
    menu_system_init(&main_menu, menu_items, menu_count);

    /* Start with welcome screen */
    current_app_state = APP_STATE_WELCOME;

    /* Initialize timing */
    last_frame_time = 0;
    last_status_update_time = 0;
    last_menu_refresh_time = 0;

    /* Show welcome screen */
    game_controller_show_welcome_screen();
}

void game_controller_init_with_default_menu(void) {
    MenuItem* game_menu;
    uint8_t game_menu_size;

    /* Get the default game menu from menu system */
    menu_system_get_game_menu(&game_menu, &game_menu_size);

    /* Initialize with default menu */
    game_controller_init(game_menu, game_menu_size);
}

void game_controller_update(void) {
    uint32_t current_time = get_current_ms();

    /* Update status bar periodically */
    if (current_time - last_status_update_time > STATUS_BAR_UPDATE_INTERVAL) {
        game_controller_update_status_bar();
        last_status_update_time = current_time;
    }

    /* Run game loop if in game state */
    if (current_app_state == APP_STATE_GAME_ACTIVE) {
        game_controller_run_game_loop();
    }
}

void game_controller_handle_input(JoystickStatus js_status) {
    switch (current_app_state) {
        case APP_STATE_WELCOME:
            handle_welcome_input(js_status);
            break;

        case APP_STATE_MENU:
            game_controller_handle_menu_input(js_status);
            break;

        case APP_STATE_GAME_ACTIVE:
            game_controller_handle_game_input(js_status);
            break;

        case APP_STATE_GAME_OVER:
            /* Game over screen - any input returns to menu */
//            if (js_status.is_new && (js_status.button || pb1_get_state())) {
//                game_controller_return_to_menu();
//            }
//        	Return to main menu after pressing push-button one or joystick button
//        	if (pb1_get_state() || (js_status.is_new && js_status.button)) {
//        		game_controller_return_to_menu();
//        	}

//        	Auto-return to main menu
        	game_controller_return_to_menu();
            break;

        default:
            /* Error state or unknown - return to menu */
            game_controller_return_to_menu();
            break;
    }
}

static void handle_welcome_input(JoystickStatus js_status) {
    /* Any input moves to menu */
    if (js_status.is_new || pb1_get_state()) {
        game_controller_set_state(APP_STATE_MENU);
        game_controller_show_menu();
    }
}

void game_controller_handle_menu_input(JoystickStatus js_status) {
    uint32_t current_time = get_current_ms();

    /* Throttle menu updates to avoid too frequent refreshes */
    if (current_time - last_menu_refresh_time < MENU_REFRESH_THROTTLE) {
        return;
    }

    /* Check push button 1 first (highest priority) */
    uint8_t pb1_state = pb1_get_state();
    if (pb1_state) {
        handle_menu_selection();
        last_menu_refresh_time = current_time;
        return;
    }

    /* Then check joystick button */
    if (js_status.is_new && js_status.button) {
        handle_menu_selection();
        last_menu_refresh_time = current_time;
        return;
    }

    /* Handle D-pad navigation if it has new input */
    uint8_t dpad_nav = convert_dpad_to_menu_nav();
    if (dpad_nav != MENU_NAV_NONE) {
        handle_menu_navigation(dpad_nav);
        last_menu_refresh_time = current_time;
        return;
    }

    /* Handle joystick navigation if it has new input */
    if (js_status.is_new) {
        uint8_t js_nav = convert_joystick_to_menu_nav(js_status);
        if (js_nav != MENU_NAV_NONE) {
            handle_menu_navigation(js_nav);
            last_menu_refresh_time = current_time;
        }
    }
}

static void handle_menu_navigation(uint8_t direction) {
    uint8_t old_selection = menu_system_get_current_selection(&main_menu);

    if (menu_system_handle_navigation(&main_menu, direction)) {
        if (menu_system_needs_full_refresh(&main_menu)) {
            menu_system_render(&main_menu);
        } else {
            menu_system_render_partial_update(&main_menu, old_selection);
        }
        menu_system_clear_refresh_flag(&main_menu);
    }
}

static void handle_menu_selection(void) {
    menu_system_handle_navigation(&main_menu, MENU_NAV_SELECT);
    MenuItem selected = menu_system_get_selected_item(&main_menu);

    if (selected.title != NULL) {
        if (selected.is_game) {
            game_controller_start_game(selected.screen);
        } else {
            /* Handle non-game menu items here if needed */
            display_manager_show_status_message("Feature not implemented");
        }
    }
}

void game_controller_handle_game_input(JoystickStatus js_status) {
    /* Game input is handled in the game loop */
    /* This function is here for future extension if needed */
}

void game_controller_run_game_loop(void) {
    uint32_t current_time = get_current_ms();

    if (current_time - last_frame_time >= FRAME_RATE) {
        GameEngine* current_engine = game_controller_get_current_game_engine();

        if (current_engine) {
            /* Handle button-triggered actions (reset, return to menu) */
            if (!handle_game_button_actions(current_engine)) {
                last_frame_time = current_time;
                return;
            }

            /* Regular game processing */
            process_game_loop(current_engine);

            /* Handle game over condition */
            if (!handle_game_over(current_engine)) {
                last_frame_time = current_time;
                return;
            }
        }

        last_frame_time = current_time;
    }
}

static bool handle_game_button_actions(GameEngine* engine) {
    /* Check for game reset request */
    if (engine->base_state.is_reset) {
        game_engine_cleanup(engine);
        game_engine_init(engine);
        engine->base_state.is_reset = false;
    }

    /* Check for return to main menu request */
    if (engine->return_to_main_menu) {
        add_delay(100);
        game_engine_cleanup(engine);
        engine->return_to_main_menu = false;
        game_controller_return_to_menu();
        return false;
    }

    return true;
}

static void process_game_loop(GameEngine* engine) {
    void* controller_data;

    if (engine->is_d_pad_game) {
        DPAD_STATUS dpad_status = d_pad_get_status();
        controller_data = &dpad_status;
    } else {
        JoystickStatus js_status = joystick_get_status();
        controller_data = &js_status;
    }

    /* Update game state with the appropriate controller data */
    game_engine_update(engine, controller_data);

    /* Render the game */
    game_engine_render(engine);
}

// Returns to main menu when push-button 1 is press
//static bool handle_game_over(GameEngine* engine) {
//    if (engine->base_state.game_over && engine->countdown_over) {
//        /* Show game over screen */
//        display_manager_show_game_over_message("Press PB 1 to continue",
//                                             engine->base_state.score);
//
//        // Clean up after displaying the score
//        game_engine_cleanup(engine);
//        current_app_state = APP_STATE_GAME_OVER;
//        return false;
//    }
//
//    return true;
//}

// Auto-returns to main menu and is in sync with the countdown
static bool handle_game_over(GameEngine* engine) {
    if (engine->base_state.game_over && engine->countdown_over) {
        // Clean up after displaying the score
        game_engine_cleanup(engine);
        current_app_state = APP_STATE_GAME_OVER;
        return false;
    }

    return true;
}

/* Conversion functions */
static uint8_t convert_joystick_to_menu_nav(JoystickStatus js_status) {
    switch (js_status.direction) {
        case JS_DIR_UP:
            return MENU_NAV_UP;
        case JS_DIR_DOWN:
            return MENU_NAV_DOWN;
        default:
            return MENU_NAV_NONE;
    }
}

static uint8_t convert_dpad_to_menu_nav(void) {
    DPAD_STATUS dpad_status = d_pad_get_status();
    uint8_t dpad_changed = d_pad_direction_changed();

    if (!dpad_changed) {
        return MENU_NAV_NONE;
    }

    switch (dpad_status.direction) {
        case DPAD_DIR_UP:
            return MENU_NAV_UP;
        case DPAD_DIR_DOWN:
            return MENU_NAV_DOWN;
        default:
            return MENU_NAV_NONE;
    }
}

/* State management functions */
AppState game_controller_get_state(void) {
    return current_app_state;
}

void game_controller_set_state(AppState new_state) {
    current_app_state = new_state;
}

void game_controller_show_welcome_screen(void) {
    display_manager_show_welcome_message("Welcome to", "Game Console!");
    current_app_state = APP_STATE_WELCOME;
}

void game_controller_show_menu(void) {
    menu_system_reset(&main_menu);
    menu_system_render(&main_menu);
    current_app_state = APP_STATE_MENU;
}

void game_controller_start_game(ScreenType game_screen) {
    current_game_screen = game_screen;
    current_app_state = APP_STATE_GAME_ACTIVE;

    /* Initialize the appropriate game engine */
    GameEngine* engine = game_controller_get_current_game_engine();
    if (engine) {
        game_engine_init(engine);
    }
}

void game_controller_return_to_menu(void) {
    current_app_state = APP_STATE_MENU;
    current_game_screen = SCREEN_MENU;
    game_controller_show_menu();
}

bool game_controller_is_game_active(void) {
    return (current_app_state == APP_STATE_GAME_ACTIVE);
}

GameEngine* game_controller_get_current_game_engine(void) {
    switch (current_game_screen) {
        case SCREEN_GAME_SNAKE:
            return &snake_game_engine;
        case SCREEN_GAME_PACMAN:
            return &pacman_game_engine;
        default:
            return NULL;
    }
}

void game_controller_update_status_bar(void) {
    bool wifi_connected = serial_comm_is_wifi_connected();
    bool in_game = game_controller_is_game_active();
    uint32_t score = 0;
    int lives = 0;

    if (in_game) {
        GameEngine* engine = game_controller_get_current_game_engine();
        if (engine) {
            score = engine->base_state.score;
            lives = engine->base_state.lives;
        }
    }

    display_manager_draw_status_bar(wifi_connected, score, lives, in_game);
}

MenuItem game_controller_get_selected_menu_item(void) {
    return menu_system_get_selected_item(&main_menu);
}

/* Utility functions for testing */
uint8_t game_controller_get_current_menu_selection(void) {
    return menu_system_get_current_selection(&main_menu);
}

uint8_t game_controller_get_menu_size(void) {
    return menu_system_get_item_count(&main_menu);
}
