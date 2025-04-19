/*
 * pacman_maze.c
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 */

#include "Game_Engine/Games/pacman_maze.h"

inline int screen_to_maze_x(coord_t x) {
    // Convert to first tile if before border
    if (x < BORDER_OFFSET) {
        return 0;
    }
    int maze_x = (int)(x - BORDER_OFFSET) / TILE_SIZE;
    return (maze_x >= MAZE_WIDTH) ? MAZE_WIDTH - 1 : maze_x;
}

inline int screen_to_maze_y(coord_t y) {
    // Convert to first tile if before game area
    if (y < GAME_AREA_TOP) {
        return 0;
    }
    int maze_y = (int)(y - GAME_AREA_TOP) / TILE_SIZE;
    return (maze_y >= MAZE_HEIGHT_ACTUAL) ? MAZE_HEIGHT_ACTUAL - 1 : maze_y;
}

inline coord_t maze_to_screen_x(uint8_t x) {
    // Clamp x to valid range
    x = (x >= MAZE_WIDTH) ? MAZE_WIDTH : x;
    return x * TILE_SIZE + BORDER_OFFSET;
}

inline coord_t maze_to_screen_y(uint8_t y) {
    // Clamp y to valid range
    y = (y >= MAZE_HEIGHT_ACTUAL) ? MAZE_HEIGHT_ACTUAL : y;
    return y * TILE_SIZE + GAME_AREA_TOP;
}

bool is_wall(coord_t x, coord_t y) {
    int maze_x = screen_to_maze_x(x);
    int maze_y = screen_to_maze_y(y);

    // Validate indices - this handles negative values too
    if (maze_x < 0 || maze_y < 0 || maze_x >= MAZE_WIDTH || maze_y >= MAZE_HEIGHT_ACTUAL) {
        return true;  // Out of bounds is considered a wall
    }

    return MAZE_LAYOUT[maze_y][maze_x] == MAZE_WALL;
}

void draw_maze(void) {
    static bool already_drawing = false;

    // Prevent recursive or multiple calls
    if (already_drawing) {
        return;
    }

    already_drawing = true;

    // Draw the outer border first
    display_draw_border_at(BORDER_OFFSET, GAME_AREA_TOP, 2, 2);

    // Clear any previous maze content on LCD to avoid overlapping
#ifdef DISPLAY_MODULE_LCD
    display_fill_rectangle(
        BORDER_OFFSET + 1,
        GAME_AREA_TOP + 1,
        DISPLAY_WIDTH - BORDER_OFFSET - 2,
        DISPLAY_HEIGHT - BORDER_OFFSET - 2,
        DISPLAY_BLACK
    );
#endif

    // Then draw internal walls and items
    for (uint8_t y = 0; y < MAZE_HEIGHT_ACTUAL; y++) {
        for (uint8_t x = 0; x < MAZE_WIDTH; x++) {
            coord_t screen_x = maze_to_screen_x(x);
            coord_t screen_y = maze_to_screen_y(y);

            switch (MAZE_LAYOUT[y][x]) {
            case MAZE_WALL:
                // Draw ALL walls
#ifdef DISPLAY_MODULE_LCD
            	display_fill_rectangle(screen_x, screen_y,
            			screen_x + TILE_SIZE - 1,
						screen_y + TILE_SIZE - 1,
						DISPLAY_WHITE);
#else
            	display_fill_rectangle(screen_x, screen_y,
            			screen_x + TILE_SIZE - 1,
						screen_y + TILE_SIZE - 1,
						DISPLAY_WHITE);
#endif
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

    already_drawing = false;
}
