/*
 * game_controller.h
 *
 *  Created on: Jun 2, 2025
 *      Author: rohitimandi
 */

#ifndef INC_APPLICATION_GAME_CONTROLLER_H_
#define INC_APPLICATION_GAME_CONTROLLER_H_

#include <stdbool.h>
#include <stdint.h>
#include "Console_Peripherals/types.h"
#include "Console_Peripherals/Hardware/joystick.h"
#include "Console_Peripherals/UI/menu_system.h"  /* Includes MenuItem and ScreenType */
#include "Game_Engine/game_engine_conf.h"
#include "Game_Engine/game_engine.h"

/* Application states */
typedef enum {
    APP_STATE_WELCOME,
    APP_STATE_MENU,
    APP_STATE_GAME_ACTIVE,
    APP_STATE_GAME_OVER,
    APP_STATE_ERROR
} AppState;

/* Status update intervals */
#define STATUS_BAR_UPDATE_INTERVAL 1000  /* Update status bar every 1 sec */
#define MENU_REFRESH_THROTTLE      100   /* Throttle menu updates to 100ms */

/* Game controller initialization and main loop */
void game_controller_init(MenuItem* menu_items, uint8_t menu_count);
void game_controller_init_with_default_menu(void);  /* Convenience function */
void game_controller_update(void);
void game_controller_handle_input(JoystickStatus js_status);

/* State management functions */
AppState game_controller_get_state(void);
void game_controller_set_state(AppState new_state);
void game_controller_show_welcome_screen(void);
void game_controller_show_menu(void);
void game_controller_start_game(ScreenType game_screen);
void game_controller_return_to_menu(void);

/* Game management functions */
bool game_controller_is_game_active(void);
void game_controller_run_game_loop(void);
GameEngine* game_controller_get_current_game_engine(void);

/* Input handling functions */
void game_controller_handle_menu_input(JoystickStatus js_status);
void game_controller_handle_game_input(JoystickStatus js_status);

/* Menu navigation functions  */
bool game_controller_handle_menu_selection(void);
void game_controller_handle_back_navigation(void);

/* Status and display functions */
void game_controller_update_status_bar(void);
MenuItem game_controller_get_selected_menu_item(void);

/* Menu state functions - NEW */
MenuType game_controller_get_current_menu_type(void);
bool game_controller_is_in_main_menu(void);

/* Utility functions for testing */
uint8_t game_controller_get_current_menu_selection(void);
uint8_t game_controller_get_menu_size(void);

#endif /* INC_APPLICATION_GAME_CONTROLLER_H_ */
