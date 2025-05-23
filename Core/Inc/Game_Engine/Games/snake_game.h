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

#define SNAKE_SPEED        500  // Movement delay in ms

 // Snake game specific data structure
typedef struct {
    coord_t head_x;
    coord_t head_y;
    uint8_t direction;  // Will use DPAD_DIR  values
    uint8_t length;
    struct {
        coord_t x;
        coord_t y;
    } body[64];  // Maximum snake length
    struct {
        coord_t x;
        coord_t y;
    } food;
} SnakeGameData;

// Snake game engine instance
extern GameEngine snake_game_engine;

#endif /* INC_GAMES_SNAKE_GAME_H_ */
