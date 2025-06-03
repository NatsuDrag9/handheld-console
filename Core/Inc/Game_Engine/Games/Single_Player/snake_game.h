/*
 * snake_game.h
 *
 *  Created on: Jun 3, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAMES_SINGLE_PLAYER_SNAKE_GAME_H_
#define INC_GAME_ENGINE_GAMES_SINGLE_PLAYER_SNAKE_GAME_H_

#include "Game_Engine/game_engine.h"
#include "Game_Engine/Games/Helpers/snake_game_helpers.h"
#include "Sprites/snake_sprite.h"
#include "Console_Peripherals/Hardware/display_manager.h"

// Snake game specific data structure
typedef struct {
    SnakeState snake;
    Position food;
} SnakeGameData;

// Snake game engine instance
extern GameEngine snake_game_engine;

#endif /* INC_GAME_ENGINE_GAMES_SINGLE_PLAYER_SNAKE_GAME_H_ */
