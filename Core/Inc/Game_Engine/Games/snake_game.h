/*
 * snake_game.h
 *
 *  Created on: Jan 15, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAMES_SNAKE_GAME_H_
#define INC_GAMES_SNAKE_GAME_H_

#include "Game_Engine/game_engine.h"
#include "Sprites/snake_sprite.h"

// Snake game specific data structure
typedef struct {
    uint8_t head_x;
    uint8_t head_y;
    uint8_t direction;  // Will use JS_DIR_* values
    uint8_t length;
    struct {
        uint8_t x;
        uint8_t y;
    } body[64];  // Maximum snake length
    struct {
        uint8_t x;
        uint8_t y;
    } food;
} SnakeGameData;

// Snake game engine instance
extern GameEngine snake_game_engine;

#endif /* INC_GAMES_SNAKE_GAME_H_ */
