/*
 * pacman_game.h
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAMES_PACMAN_GAME_H_
#define INC_GAME_ENGINE_GAMES_PACMAN_GAME_H_

#include "Game_Engine/game_engine.h"
#include "Game_Engine/Games/game_types.h"
#include "Sprites/pacman_sprite.h"
#include "Game_Engine/Games/pacman_maze.h"

#define PACMAN_SPEED      300  // Movement delay in ms
#define MAX_DOTS         100   // Maximum number of dots
#define NUM_GHOSTS        4    // Number of ghosts in the game
#define GHOST_SCATTER_TIME 7000 // Time ghosts remain scared after power pellet

// Ghost types
typedef enum {
    GHOST_BLINKY,  // Red - chases directly
    GHOST_PINKY,   // Pink - tries to ambush
    GHOST_INKY,    // Blue - unpredictable
    GHOST_CLYDE    // Orange - random movement
} GhostType;

// Movement directions
typedef enum {
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
    DIR_NONE
} Direction;

// Ghost behavior modes
typedef enum {
    MODE_CHASE,
    MODE_SCATTER,
    MODE_FRIGHTENED
} GhostMode;

// Ghost structure
typedef struct {
    Position pos;
    Direction dir;
    GhostType type;
    GhostMode mode;
    bool active;
    Position target;     // Current target tile
} Ghost;

// Dot structure
typedef struct {
    Position pos;
    bool active;
    bool is_power_pellet;
} Dot;


// Pacman game specific data structure
typedef struct {
    Position pacman_pos;
    Direction curr_dir;
    Direction next_dir;

    Ghost ghosts[NUM_GHOSTS];
    Dot dots[MAX_DOTS];

    uint32_t ghost_mode_timer;
    uint32_t ghost_mode_duration;
    uint8_t num_dots_remaining;
    bool power_pellet_active;
} PacmanGameData;

// Pacman game engine instance
extern GameEngine pacman_game_engine;

#endif /* INC_GAME_ENGINE_GAMES_PACMAN_GAME_H_ */
