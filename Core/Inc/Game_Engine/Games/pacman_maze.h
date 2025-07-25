/*
 * pacman_maze.h
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAMES_PACMAN_MAZE_H_
#define INC_GAME_ENGINE_GAMES_PACMAN_MAZE_H_

#include <Console_Peripherals/Hardware/Drivers/display_driver.h>  // For DISPLAY_WIDTH and DISPLAY_HEIGHT
#include <stdbool.h>
#include "Sprites/sprite.h"
#include "Game_Engine/game_engine_conf.h"  // For GAME_AREA_TOP, TILE_SIZE, BORDER_OFFSET

// Maze elements
typedef enum {
    MAZE_PATH = 0,  // Empty path
    MAZE_WALL = 1,  // Wall
    MAZE_DOT = 2,  // Regular dot
    MAZE_POWER = 3   // Power pellet
} MazeElement;

// Layout for smaller OLED display (128x64)
static const uint8_t MAZE_LAYOUT_OLED[6][16] = {
    {1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1},  // row 0 - Shorter top walls
    {1,3,0,0,0,0,0,0,0,0,0,0,3,1,1,1},  // row 1 - Open row with power pellets
    {1,0,1,1,0,1,1,1,1,0,1,1,0,1,1,1},  // row 2 - Some obstacles
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1},  // row 3 - Open row for movement
    {1,0,1,1,0,1,1,1,1,0,1,1,0,1,1,1},  // row 4 - Mirror of row 2
    {1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1}   // row 5 - Shorter bottom walls
};

// Layout for larger LCD display (240x320)
//static const uint8_t MAZE_LAYOUT_LCD[16][16] = {
//    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // row 0
//    {1,3,0,0,0,0,0,0,0,0,0,0,3,0,0,1},  // row 1
//    {1,0,1,1,0,1,1,1,1,0,1,1,0,1,0,1},  // row 2
//    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},  // row 3
//    {1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1},  // row 4
//    {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},  // row 5
//    {1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1},  // row 6
//    {1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1},  // row 7
//    {1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1},  // row 8
//    {1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1},  // row 9
//    {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},  // row 10
//    {1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1},  // row 11
//    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},  // row 12
//    {1,0,1,1,0,1,1,1,1,0,1,1,0,1,0,1},  // row 13
//    {1,3,0,0,0,0,0,0,0,0,0,0,3,0,0,1},  // row 14
//    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // row 15
//};

// Layout for larger LCD display (320x240) with TILE_SIZE=16 - scaled to 14x19
static const uint8_t MAZE_LAYOUT_LCD[14][19] = {
    // Top border
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

    // Main maze
    {1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
    {1,0,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1},
    {1,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,1},
    {1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1},
    {1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1},
    {1,3,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

//// Layout for larger LCD display (320x240) with TILE_SIZE=12 - scaled to 19x25
//static const uint8_t MAZE_LAYOUT_LCD[19][25] = {
//    // Top border
//    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
//
//    // Main maze
//    {1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
//    {1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
//    {1,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,1,0,1},
//    {1,0,1,0,1,1,1,1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1,0,1},
//    {1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1},
//    {1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1},
//    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
//    {1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
//    {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1},
//    {1,0,1,0,1,1,1,0,1,1,0,1,1,0,1,0,1,1,1,1,1,0,1,0,1},
//    {1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1},
//    {1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1},
//    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
//    {1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1},
//    {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1},
//    {1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1},
//    {1,3,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
//    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
//};

//// Layout for larger LCD display (320x240) with TILE_SIZE=8 - scaled to 27x38
//static const uint8_t MAZE_LAYOUT_LCD[27][38] = {
//    // Top border
//    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
//
//    // Main maze
//    {1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
//    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
//    {1,0,1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1},
//    {1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1},
//    {1,0,1,0,1,0,0,0,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,1,0,1},
//    {1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1},
//    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
//    {1,0,1,1,1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1},
//    {1,0,1,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,0,0,1},
//    {1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1},
//    {1,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1},
//    {1,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1},
//    {1,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},
//    {1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,0,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1},
//    {1,0,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1},
//    {1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
//    {1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
//    {1,0,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1},
//    {1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
//    {1,0,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1},
//    {1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1},
//    {1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1},
//    {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//    {1,0,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
//    {1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
//    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
//};


// Calculate maze dimensions based on screen size
#define MAZE_WIDTH  ((DISPLAY_WIDTH - 2*BORDER_OFFSET) / TILE_SIZE)   // Varies by display
#define MAZE_HEIGHT ((DISPLAY_HEIGHT - GAME_AREA_TOP - BORDER_OFFSET) / TILE_SIZE) + 1  // Varies by display

#ifdef DISPLAY_MODULE_OLED
#define MAZE_LAYOUT MAZE_LAYOUT_OLED
#define MAZE_HEIGHT_ACTUAL MAZE_HEIGHT
#endif

#ifdef DISPLAY_MODULE_LCD
#define MAZE_LAYOUT MAZE_LAYOUT_LCD
#define MAZE_HEIGHT_ACTUAL MAZE_HEIGHT
#endif

// For unit tests, default to OLED layout
#ifdef UNITY_TEST
#define MAZE_LAYOUT MAZE_LAYOUT_OLED
#define MAZE_HEIGHT_ACTUAL 6
#endif

// Function declarations

// Convert screen coordinates to maze indices - returns int to prevent underflow
int screen_to_maze_x(coord_t x);
int screen_to_maze_y(coord_t y);

// Convert maze indices to screen coordinates
coord_t maze_to_screen_x(uint8_t x);
coord_t maze_to_screen_y(uint8_t y);

// Check if a coordinate contains a wall
bool is_wall(coord_t x, coord_t y);
void draw_maze(void);

#endif /* INC_GAME_ENGINE_GAMES_PACMAN_MAZE_H_ */
