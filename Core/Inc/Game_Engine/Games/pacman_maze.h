/*
 * pacman_maze.h
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAMES_PACMAN_MAZE_H_
#define INC_GAME_ENGINE_GAMES_PACMAN_MAZE_H_

#include <stdbool.h>
#include "Sprites/sprite.h"
#include "Console_Peripherals/Drivers/display_driver.h"  // For DISPLAY_WIDTH and DISPLAY_HEIGHT
#include "Game_Engine/game_engine_conf.h"  // For GAME_AREA_TOP, TILE_SIZE, BORDER_OFFSET

 // Maze elements
typedef enum {
    MAZE_PATH = 0,  // Empty path
    MAZE_WALL = 1,  // Wall
    MAZE_DOT = 2,  // Regular dot
    MAZE_POWER = 3   // Power pellet
} MazeElement;

// Calculate maze dimensions based on screen size
#define MAZE_WIDTH  ((DISPLAY_WIDTH - 2*BORDER_OFFSET) / TILE_SIZE)   // = 14 tiles
#define MAZE_HEIGHT ((DISPLAY_HEIGHT - GAME_AREA_TOP - BORDER_OFFSET) / TILE_SIZE)  // = 5 tiles

// Function declarations

// Convert screen coordinates to maze indices - cast to int first to prevent underflow
int screen_to_maze_x(uint8_t x);
int screen_to_maze_y(uint8_t y);

// Convert maze indices to screen coordinates
uint8_t maze_to_screen_x(uint8_t x);
uint8_t maze_to_screen_y(uint8_t y);

// Maze layout (14x5 tiles)
// 0 = path, 1 = wall, 2 = dot, 3 = power pellet
static const uint8_t MAZE_LAYOUT[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,0,1,1,1,1,1,1},
    {1,3,0,2,1,0,0,0,0,1,2,0,3,1},
    {1,0,1,2,0,1,0,0,1,0,2,1,0,1},
    {1,2,0,2,1,0,0,0,0,1,2,0,2,1},
    {1,1,1,1,1,1,1,0,1,1,1,1,1,1}
};

// Function declarations
bool is_wall(uint8_t x, uint8_t y);
void draw_maze(void);

#endif /* INC_GAME_ENGINE_GAMES_PACMAN_MAZE_H_ */
