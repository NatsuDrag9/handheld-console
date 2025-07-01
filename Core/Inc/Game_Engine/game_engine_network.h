/*
 * game_engine_network.h
 *
 *  Created on: Jul 1, 2025
 *      Author: rohitimandi
 *
 *  Network error handling for game engine - handles connection loss and exit logic
 */

#ifndef INC_GAME_ENGINE_GAME_ENGINE_NETWORK_H_
#define INC_GAME_ENGINE_GAME_ENGINE_NETWORK_H_

/*
 * game_engine_network.h
 *
 *  Created on: Jun 6, 2025
 *      Author: rohitimandi
 *
 *  Network error handling for game engine - handles connection loss and exit logic
 */

#ifndef INC_GAME_ENGINE_NETWORK_GAME_ENGINE_NETWORK_H_
#define INC_GAME_ENGINE_NETWORK_GAME_ENGINE_NETWORK_H_

#include "Game_Engine/game_engine.h"
#include "Console_Peripherals/Hardware/serial_comm.h"
#include "Console_Peripherals/Hardware/display_manager.h"
#include <stdint.h>
#include <stdbool.h>

// Network error handling functions
void game_engine_network_init(void);
void game_engine_network_cleanup(void);
void game_engine_network_check_errors(GameEngine* engine);
void game_engine_network_render_error(void);

// Network error state queries
bool game_engine_network_has_error(void);

#endif /* INC_GAME_ENGINE_GAME_ENGINE_NETWORK_H_ */
