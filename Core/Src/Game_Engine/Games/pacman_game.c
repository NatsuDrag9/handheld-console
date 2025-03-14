/*
 * pacman_game.c
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 */

#include "Game_Engine/Games/pacman_game.h"
#include "Utils/misc_utils.h"
#include <stdlib.h>
#include <limits.h>

static uint32_t last_move_time = 0;

// Initial game data
static PacmanGameData pacman_data = {
    .pacman_pos = {0, 0},
    .curr_dir = DIR_RIGHT,
    .next_dir = DIR_RIGHT,
    .ghosts = {{{0}}},    // Triple braces for nested struct
    .dots = {{{0}}},      // Triple braces for nested struct
    .ghost_mode_timer = 0,
    .ghost_mode_duration = 0,
    .num_dots_remaining = 0,
    .power_pellet_active = false
};

// Forward declarations
static void pacman_init(void);
static void pacman_update(JoystickStatus js_status);
static void pacman_render(void);
static void pacman_cleanup(void);
static void init_dots(void);
static void init_ghosts(void);
static void update_ghosts(void);
static bool check_wall_collision(Position pos);
static void handle_dot_collision(void);
static void handle_ghost_collision(void);
static void move_pacman(void);
static Position get_ghost_target(Ghost* ghost);
static Direction get_next_direction(Ghost* ghost);

// Game engine instance
GameEngine pacman_game_engine = {
    .init = pacman_init,
    .update = pacman_update,
    .render = pacman_render,
    .cleanup = pacman_cleanup,
    .game_data = &pacman_data,
    .base_state = {
        .score = 0,
        .lives = 3,
        .paused = false,
        .game_over = false
    }
};

static bool check_wall_collision(Position pos) {
    // Just pass through the screen coordinates to is_wall
    return is_wall(pos.x, pos.y);
}

static Position get_next_position(Position current, Direction dir) {
    Position next = current;
    switch (dir) {
    case DIR_RIGHT: next.x += TILE_SIZE; break;
    case DIR_LEFT:  next.x -= TILE_SIZE; break;
    case DIR_UP:    next.y -= TILE_SIZE; break;
    case DIR_DOWN:  next.y += TILE_SIZE; break;
    case DIR_NONE:  break;
    }

    // Add border constraints
        // Keep within horizontal bounds
        if (next.x < BORDER_OFFSET) {
            next.x = BORDER_OFFSET;
        }
        else if (next.x > DISPLAY_WIDTH - BORDER_OFFSET - TILE_SIZE) {
            next.x = DISPLAY_WIDTH - BORDER_OFFSET - TILE_SIZE;
        }

        // Keep within vertical bounds
        if (next.y < GAME_AREA_TOP) {
            next.y = GAME_AREA_TOP;
        }
        else if (next.y > DISPLAY_HEIGHT - BORDER_OFFSET - TILE_SIZE) {
            next.y = DISPLAY_HEIGHT - BORDER_OFFSET - TILE_SIZE;
        }
        return next;
}

static void init_dots(void) {
    pacman_data.num_dots_remaining = 0;

    // Place dots according to maze layout
    for (uint8_t y = 0; y < MAZE_HEIGHT; y++) {
        for (uint8_t x = 0; x < MAZE_WIDTH; x++) {
            if (MAZE_LAYOUT[y][x] == MAZE_DOT || MAZE_LAYOUT[y][x] == MAZE_POWER) {
                Position pos = {
                    .x = maze_to_screen_x(x),
                    .y = maze_to_screen_y(y)
                };

                pacman_data.dots[pacman_data.num_dots_remaining].pos = pos;
                pacman_data.dots[pacman_data.num_dots_remaining].active = true;
                pacman_data.dots[pacman_data.num_dots_remaining].is_power_pellet =
                    (MAZE_LAYOUT[y][x] == MAZE_POWER);
                pacman_data.num_dots_remaining++;
            }
        }
    }
}

static void init_ghosts(void) {
    const Position ghost_starts[NUM_GHOSTS] = {
           {maze_to_screen_x(1), maze_to_screen_y(1)},    // Blinky - top left
           {maze_to_screen_x(MAZE_WIDTH - 2), maze_to_screen_y(1)},   // Pinky - top right
           {maze_to_screen_x(1), maze_to_screen_y(MAZE_HEIGHT - 2)},    // Inky - bottom left
           {maze_to_screen_x(MAZE_WIDTH - 2), maze_to_screen_y(MAZE_HEIGHT - 2)}    // Clyde - bottom right
    };

    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        pacman_data.ghosts[i].pos = ghost_starts[i];
        pacman_data.ghosts[i].dir = DIR_RIGHT;
        pacman_data.ghosts[i].type = (GhostType)i;
        pacman_data.ghosts[i].mode = MODE_CHASE;
        pacman_data.ghosts[i].active = true;
        pacman_data.ghosts[i].target = ghost_starts[i]; // Initial target is start position
    }
}

static void pacman_init(void) {
    // Start Pacman at index 8 (an open path '0') in row 1
    pacman_data.pacman_pos.x = maze_to_screen_x(6);
    pacman_data.pacman_pos.y = maze_to_screen_y(1); // Second row

    pacman_data.curr_dir = DIR_RIGHT;
    pacman_data.next_dir = DIR_RIGHT;

    init_dots();
    init_ghosts();

    last_move_time = get_current_ms();
    pacman_data.ghost_mode_timer = last_move_time;
    pacman_data.ghost_mode_duration = GHOST_SCATTER_TIME;
    pacman_data.power_pellet_active = false;
}


static Position get_ghost_target(Ghost* ghost) {
    Position target = pacman_data.pacman_pos; // Default target

    switch (ghost->mode) {
    case MODE_FRIGHTENED:
        // Random target when frightened
        target.x = maze_to_screen_x(get_random() % MAZE_WIDTH);
        target.y = maze_to_screen_y(get_random() % MAZE_HEIGHT);
        break;

    case MODE_SCATTER:
        // Return to home corner
        switch (ghost->type) {
        case GHOST_BLINKY:
            target.x = maze_to_screen_x(1);
            target.y = maze_to_screen_y(1);
            break;
        case GHOST_PINKY:
            target.x = maze_to_screen_x(MAZE_WIDTH - 2);
            target.y = maze_to_screen_y(1);
            break;
        case GHOST_INKY:
            target.x = maze_to_screen_x(1);
            target.y = maze_to_screen_y(MAZE_HEIGHT - 2);
            break;
        case GHOST_CLYDE:
            target.x = maze_to_screen_x(MAZE_WIDTH - 2);
            target.y = maze_to_screen_y(MAZE_HEIGHT - 2);
            break;
        }
        break;

    case MODE_CHASE:
        switch (ghost->type) {
        case GHOST_BLINKY:
            // Directly target Pacman
            target = pacman_data.pacman_pos;
            break;

        case GHOST_PINKY: {
            // Target 4 tiles ahead of Pacman
            target = pacman_data.pacman_pos;
            for (int i = 0; i < 4; i++) {
                Position next = get_next_position(target, pacman_data.curr_dir);
                if (!check_wall_collision(next)) {
                    target = next;
                }
            }
            break;
        }

        case GHOST_INKY: {
            // Target based on Blinky's position
            Position blinky_pos = pacman_data.ghosts[GHOST_BLINKY].pos;
            int dx = pacman_data.pacman_pos.x - blinky_pos.x;
            int dy = pacman_data.pacman_pos.y - blinky_pos.y;
            target.x = blinky_pos.x + dx * 2;
            target.y = blinky_pos.y + dy * 2;
            break;
        }

        case GHOST_CLYDE:
            // Random behavior or chase based on distance
            if (get_random() % 2) {
                target = pacman_data.pacman_pos;
            }
            else {
                target.x = maze_to_screen_x(get_random() % MAZE_WIDTH);
                target.y = maze_to_screen_y(get_random() % MAZE_HEIGHT);
            }
            break;
        }
        break;
    }
    return target;
}

static Direction get_next_direction(Ghost* ghost) {
    Direction possible_dirs[4] = { DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT };
    Direction best_dir = ghost->dir;
    int min_distance = INT_MAX;

    // Try each possible direction
    for (int i = 0; i < 4; i++) {
        // Don't reverse direction unless necessary
        if (possible_dirs[i] == (ghost->dir + 2) % 4) continue;

        Position next_pos = get_next_position(ghost->pos, possible_dirs[i]);
        if (!check_wall_collision(next_pos)) {
            // Calculate distance to target
            int dx = next_pos.x - ghost->target.x;
            int dy = next_pos.y - ghost->target.y;
            int distance = dx * dx + dy * dy;

            if (distance < min_distance) {
                min_distance = distance;
                best_dir = possible_dirs[i];
            }
        }
    }

    return best_dir;
}

static void move_pacman(void) {

    Position next_pos = get_next_position(pacman_data.pacman_pos, pacman_data.next_dir);

    if (!check_wall_collision(next_pos)) {
        pacman_data.pacman_pos = next_pos;
        pacman_data.curr_dir = pacman_data.next_dir;
    }
    else {
        next_pos = get_next_position(pacman_data.pacman_pos, pacman_data.curr_dir);
        if (!check_wall_collision(next_pos)) {
            pacman_data.pacman_pos = next_pos;
        }
    }
}

static void update_ghosts(void) {
    uint32_t current_time = get_current_ms();

    // Update ghost mode if timer expired
    if (current_time - pacman_data.ghost_mode_timer >= pacman_data.ghost_mode_duration) {
        for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
            if (pacman_data.ghosts[i].mode == MODE_FRIGHTENED) {
                pacman_data.ghosts[i].mode = MODE_CHASE;
            }
        }
        pacman_data.power_pellet_active = false;
    }

    // Update each ghost
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        Ghost* ghost = &pacman_data.ghosts[i];
        if (!ghost->active) continue;

        // Update target
        ghost->target = get_ghost_target(ghost);

        // Get new direction
        ghost->dir = get_next_direction(ghost);

        // Move ghost
        Position next_pos = get_next_position(ghost->pos, ghost->dir);
        if (!check_wall_collision(next_pos)) {
            ghost->pos = next_pos;
        }
    }
}

static void handle_dot_collision(void) {
    for (uint8_t i = 0; i < MAX_DOTS; i++) {
        if (!pacman_data.dots[i].active) continue;

        Position dot_pos = pacman_data.dots[i].pos;
        if (abs(pacman_data.pacman_pos.x - dot_pos.x) < TILE_SIZE / 2 &&
            abs(pacman_data.pacman_pos.y - dot_pos.y) < TILE_SIZE / 2) {

            pacman_data.dots[i].active = false;
            pacman_data.num_dots_remaining--;

            if (pacman_data.dots[i].is_power_pellet) {
                // Activate power pellet mode
                pacman_data.power_pellet_active = true;
                pacman_data.ghost_mode_timer = get_current_ms();
                pacman_game_engine.base_state.score += 50;

                for (uint8_t j = 0; j < NUM_GHOSTS; j++) {
                    pacman_data.ghosts[j].mode = MODE_FRIGHTENED;
                }
            }
            else {
                pacman_game_engine.base_state.score += 10;
            }
        }
    }
}

static void handle_ghost_collision(void) {
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        Ghost* ghost = &pacman_data.ghosts[i];
        if (!ghost->active) continue;

        if (abs(pacman_data.pacman_pos.x - ghost->pos.x) < TILE_SIZE / 2 &&
            abs(pacman_data.pacman_pos.y - ghost->pos.y) < TILE_SIZE / 2) {

            if (ghost->mode == MODE_FRIGHTENED) {
                // Eat ghost
                ghost->active = false;
                pacman_game_engine.base_state.score += 200;
            }
            else {
                // Lose life
                if (pacman_game_engine.base_state.lives > 0) {
                    pacman_game_engine.base_state.lives--;
                    if (pacman_game_engine.base_state.lives == 0) {
                        pacman_game_engine.base_state.game_over = true;
                    }
                    else {
                        pacman_init();
                    }
                }
            }
        }
    }
}

static void pacman_update(JoystickStatus js_status) {
    uint32_t current_time = get_current_ms();

    // Handle direction change from joystick
    if (js_status.is_new) {

        switch (js_status.direction) {
        case JS_DIR_UP:         pacman_data.next_dir = DIR_UP;    break;
        case JS_DIR_RIGHT:      pacman_data.next_dir = DIR_RIGHT; break;
        case JS_DIR_DOWN:       pacman_data.next_dir = DIR_DOWN;  break;
        case JS_DIR_LEFT:       pacman_data.next_dir = DIR_LEFT;  break;
        case JS_DIR_LEFT_UP:    pacman_data.next_dir = DIR_UP;    break;
        case JS_DIR_LEFT_DOWN:  pacman_data.next_dir = DIR_DOWN;  break;
        case JS_DIR_RIGHT_UP:   pacman_data.next_dir = DIR_UP;    break;
        case JS_DIR_RIGHT_DOWN: pacman_data.next_dir = DIR_DOWN;  break;
        case JS_DIR_CENTERED:   break;  // 0 - Keep current direction
        default: break;
        }
    }

    if (current_time - last_move_time >= PACMAN_SPEED) {

        move_pacman();
        update_ghosts();
        handle_dot_collision();
        handle_ghost_collision();

        last_move_time = current_time;
    }

    // Update animations
    animated_sprite_update(&pacman_animated);
}

static void pacman_render(void) {
    // Draw maze
    draw_maze();

    // Draw score and lives
    char status_text[32];
    snprintf(status_text, sizeof(status_text), "Score: %lu Lives: %d",
        pacman_game_engine.base_state.score,
        pacman_game_engine.base_state.lives);
    display_set_cursor(2, 2);
    display_write_string(status_text, Font_7x10, DISPLAY_WHITE);

    // Draw Pacman with rotation
    uint16_t rotation = 0;
    switch (pacman_data.curr_dir) {
    case DIR_RIGHT: rotation = 0;   break;
    case DIR_DOWN:  rotation = 90;  break;
    case DIR_LEFT:  rotation = 180; break;
    case DIR_UP:    rotation = 270; break;
    case DIR_NONE:  break;
    }
    sprite_draw_rotated(
        &pacman_animated.frames[pacman_animated.current_frame],
        pacman_data.pacman_pos.x,
        pacman_data.pacman_pos.y,
        rotation,
        DISPLAY_WHITE
    );

    // Draw active dots and power pellets
    for (uint8_t i = 0; i < MAX_DOTS; i++) {
        if (!pacman_data.dots[i].active) continue;

        if (pacman_data.dots[i].is_power_pellet) {
            sprite_draw(&power_pellet_sprite,
                pacman_data.dots[i].pos.x,
                pacman_data.dots[i].pos.y,
                DISPLAY_WHITE);
        }
        else {
            sprite_draw(&dot_sprite,
                pacman_data.dots[i].pos.x,
                pacman_data.dots[i].pos.y,
                DISPLAY_WHITE);
        }
    }

    // Draw ghosts
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        Ghost* ghost = &pacman_data.ghosts[i];
        if (!ghost->active) continue;

        if (ghost->mode == MODE_FRIGHTENED) {
            sprite_draw(&scared_ghost_animated.frames[scared_ghost_animated.current_frame],
                ghost->pos.x,
                ghost->pos.y,
                DISPLAY_WHITE);
        }
        else {
            AnimatedSprite* ghost_sprite;
            switch (ghost->type) {
            case GHOST_BLINKY: ghost_sprite = &blinky_animated; break;
            case GHOST_PINKY:  ghost_sprite = &pinky_animated;  break;
            case GHOST_INKY:   ghost_sprite = &inky_animated;   break;
            case GHOST_CLYDE:  ghost_sprite = &clyde_animated;  break;
            }
            sprite_draw(&ghost_sprite->frames[ghost_sprite->current_frame],
                ghost->pos.x,
                ghost->pos.y,
                DISPLAY_WHITE);
        }
    }

    // Draw game over or win text
    if (pacman_game_engine.base_state.game_over) {
        const char* message = (pacman_data.num_dots_remaining == 0) ?
            "YOU WIN!" : "GAME OVER";
        char temp_message[32];
        strcpy(temp_message, message);
        // Clear the screen before displaying the message
//        display_clear();
        display_write_string_centered(temp_message, Font_7x10, 30, DISPLAY_WHITE);
    }

//    if (pacman_game_engine.base_state.game_over) {
//        const char* message = (pacman_data.num_dots_remaining == 0) ?
//            "YOU WIN!" : "GAME OVER";
//        char temp_message[32];
//        strcpy(temp_message, message);
//        // Clear the screen before displaying the message
//        display_clear();
//        display_write_string_centered(temp_message, Font_7x10, 30, DISPLAY_WHITE);
//
//        // Add the countdown message
//        game_engine_render_countdown(&pacman_game_engine);
//    }

}

static void pacman_cleanup(void) {
    // Reset pacman position and direction
    pacman_data.pacman_pos.x = 0;
    pacman_data.pacman_pos.y = 0;
    pacman_data.curr_dir = DIR_RIGHT;
    pacman_data.next_dir = DIR_RIGHT;

    // Reset ghosts
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        Ghost* ghost = &pacman_data.ghosts[i];
        ghost->pos.x = 0;
        ghost->pos.y = 0;
        ghost->dir = DIR_RIGHT;
        ghost->mode = MODE_CHASE;
        ghost->active = false;
        ghost->target.x = 0;
        ghost->target.y = 0;
    }

    // Reset dots
    for (uint8_t i = 0; i < MAX_DOTS; i++) {
        pacman_data.dots[i].active = false;
        pacman_data.dots[i].pos.x = 0;
        pacman_data.dots[i].pos.y = 0;
        pacman_data.dots[i].is_power_pellet = false;
    }

    // Reset game state
    pacman_data.ghost_mode_timer = 0;
    pacman_data.ghost_mode_duration = 0;
    pacman_data.num_dots_remaining = 0;
    pacman_data.power_pellet_active = false;

    // Reset timing
    last_move_time = 0;

    // Reset game engine state
    pacman_game_engine.base_state.score = 0;
    pacman_game_engine.base_state.lives = 3;
    pacman_game_engine.base_state.paused = false;
    pacman_game_engine.base_state.game_over = false;
}
