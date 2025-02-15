/*
 * pacman_maze.c
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 */

#include "Game_Engine/Games/pacman_maze.h"
#include "Utils/debug_conf.h"

inline int screen_to_maze_x(uint8_t x) {
    // Convert to first tile if before border
    if (x < BORDER_OFFSET) {
        return 0;
    }
    int maze_x = (int)(x - BORDER_OFFSET) / TILE_SIZE;
    return (maze_x >= MAZE_WIDTH) ? MAZE_WIDTH - 1 : maze_x;
}

inline int screen_to_maze_y(uint8_t y) {
    // Convert to first tile if before game area
    if (y < GAME_AREA_TOP) {
        return 0;
    }
    int maze_y = (int)(y - GAME_AREA_TOP) / TILE_SIZE;
    return (maze_y >= MAZE_HEIGHT) ? MAZE_HEIGHT - 1 : maze_y;
}

inline uint8_t maze_to_screen_x(uint8_t x) {
    // Clamp x to valid range
    x = (x >= MAZE_WIDTH) ? MAZE_WIDTH : x;
    return x * TILE_SIZE + BORDER_OFFSET;
}

inline uint8_t maze_to_screen_y(uint8_t y) {
    // Clamp y to valid range
    y = (y >= MAZE_HEIGHT) ? MAZE_HEIGHT : y;
    return y * TILE_SIZE + GAME_AREA_TOP;
}

bool is_wall(uint8_t x, uint8_t y) {
    uint8_t maze_x = screen_to_maze_x(x);
    uint8_t maze_y = screen_to_maze_y(y);

    if (maze_x >= MAZE_WIDTH || maze_y >= MAZE_HEIGHT) {
        // DEBUG_PRINTF(false, "Out of maze bounds: maze(%d,%d)\n", maze_x, maze_y);
        return true;  // Out of bounds is considered a wall
    }

    //    return MAZE_LAYOUT[maze_y][maze_x] == MAZE_WALL;
    bool is_wall_collision = MAZE_LAYOUT[maze_y][maze_x] == MAZE_WALL;
    // DEBUG_PRINTF(false, "Maze element: %d, Is wall: %d\n",
    //     MAZE_LAYOUT[maze_y][maze_x], is_wall_collision);

    return is_wall_collision;
}

void draw_maze(void) {
    // Draw the outer border first
    display_draw_border_at(BORDER_OFFSET, GAME_AREA_TOP, 2, 2);

    // Then draw internal walls and items
    for (uint8_t y = 0; y < MAZE_HEIGHT; y++) {
        for (uint8_t x = 0; x < MAZE_WIDTH; x++) {
            uint8_t screen_x = maze_to_screen_x(x);
            uint8_t screen_y = maze_to_screen_y(y);

            switch (MAZE_LAYOUT[y][x]) {
            case MAZE_WALL:
                // Only draw internal walls, skip border walls
                if (x > 0 && x < MAZE_WIDTH - 1 && y > 0 && y < MAZE_HEIGHT - 1) {
                    display_fill_rectangle(screen_x, screen_y,
                        screen_x + TILE_SIZE - 1,
                        screen_y + TILE_SIZE - 1,
                        DISPLAY_WHITE);
                }
                break;

            case MAZE_DOT:
                // Single pixel dot
                display_draw_pixel(screen_x + (TILE_SIZE / 2),
                    screen_y + (TILE_SIZE / 2),
                    DISPLAY_WHITE);
                break;

            case MAZE_POWER:
                // Draw 2x2 power pellet
                display_fill_rectangle(screen_x + (TILE_SIZE / 2) - 1,
                    screen_y + (TILE_SIZE / 2) - 1,
                    screen_x + (TILE_SIZE / 2),
                    screen_y + (TILE_SIZE / 2),
                    DISPLAY_WHITE);
                break;
            }
        }
    }
}
