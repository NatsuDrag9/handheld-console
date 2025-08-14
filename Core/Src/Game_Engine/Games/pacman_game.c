/*
 * pacman_game.c
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 *  Modified on: Apr 17, 2025
 *      Added dirty rectangle optimization
 */

#include "Game_Engine/Games/pacman_game.h"
#include "Utils/misc_utils.h"
#include <stdlib.h>
#include <limits.h>

static uint32_t last_move_time = 0;

// For dirty rectangle optimization
static Position previous_pacman_pos = { 0, 0 };
static Position previous_ghost_pos[NUM_GHOSTS] = { {{0}} };
static bool maze_drawn = false;
static uint32_t previous_score = 0;
static uint8_t previous_lives = 0;
static bool first_render = true;
static uint32_t last_border_redraw_time = 0;

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
static void pacman_update_dpad(DPAD_STATUS dpad_status);
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

// New functions for dirty rectangle optimization
static void render_status_area(bool force_redraw);
static void clear_and_redraw_border_if_needed(coord_t x, coord_t y);
static void clear_previous_positions(void);
static void draw_game_elements(void);
static void draw_dots_and_pellets(void);
static void redraw_full_maze_if_needed(void);

// Game engine instance
GameEngine pacman_game_engine = {
    .init = pacman_init,
    .update_func = {
        .update_dpad = pacman_update_dpad
    },
    .render = pacman_render,
    .cleanup = pacman_cleanup,
    .game_data = &pacman_data,
    .base_state = {
        .state_data = {
            .single = {
                .score = 0,
                .lives = 3,
            }
        },
        .paused = false,
        .game_over = false
    },
    .is_d_pad_game = true,  // Pacman is a D-pad game
    .is_mp_game = false
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
    for (uint8_t y = 0; y < MAZE_HEIGHT_ACTUAL; y++) {
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
           {maze_to_screen_x(1), maze_to_screen_y(MAZE_HEIGHT_ACTUAL - 2)},    // Inky - bottom left
           {maze_to_screen_x(MAZE_WIDTH - 2), maze_to_screen_y(MAZE_HEIGHT_ACTUAL - 2)}    // Clyde - bottom right
    };

    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        pacman_data.ghosts[i].pos = ghost_starts[i];
        previous_ghost_pos[i] = ghost_starts[i];
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
    previous_pacman_pos = pacman_data.pacman_pos;

    pacman_data.curr_dir = DIR_RIGHT;
    pacman_data.next_dir = DIR_RIGHT;

    init_dots();
    init_ghosts();

    last_move_time = get_current_ms();
    pacman_data.ghost_mode_timer = last_move_time;
    pacman_data.ghost_mode_duration = GHOST_SCATTER_TIME;
    pacman_data.power_pellet_active = false;

    // Reset dirty rectangle tracking
    first_render = true;
    maze_drawn = false;
    previous_score = 0;
    previous_lives = 3;
    last_border_redraw_time = 0;
}

static Position get_ghost_target(Ghost* ghost) {
    Position target = pacman_data.pacman_pos; // Default target

    switch (ghost->mode) {
    case MODE_FRIGHTENED:
        // Random target when frightened
        target.x = maze_to_screen_x(get_random() % MAZE_WIDTH);
        target.y = maze_to_screen_y(get_random() % MAZE_HEIGHT_ACTUAL);
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
            target.y = maze_to_screen_y(MAZE_HEIGHT_ACTUAL - 2);
            break;
        case GHOST_CLYDE:
            target.x = maze_to_screen_x(MAZE_WIDTH - 2);
            target.y = maze_to_screen_y(MAZE_HEIGHT_ACTUAL - 2);
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
                target.y = maze_to_screen_y(get_random() % MAZE_HEIGHT_ACTUAL);
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
    // Store previous position for dirty rectangle optimization
    previous_pacman_pos = pacman_data.pacman_pos;

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

        // Store previous position for dirty rectangle optimization
        previous_ghost_pos[i] = ghost->pos;

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
                pacman_game_engine.base_state.state_data.single.score += 50;

                for (uint8_t j = 0; j < NUM_GHOSTS; j++) {
                    pacman_data.ghosts[j].mode = MODE_FRIGHTENED;
                }
            }
            else {
                pacman_game_engine.base_state.state_data.single.score += 10;
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
                pacman_game_engine.base_state.state_data.single.score += 200;
            }
            else {
                // Lose life
                if (pacman_game_engine.base_state.state_data.single.lives > 0) {
                    pacman_game_engine.base_state.state_data.single.lives--;
                    if (pacman_game_engine.base_state.state_data.single.lives == 0) {
                        pacman_game_engine.base_state.game_over = true;
                    }
                    else {
                        // Clear the game area before reinitializing
                        display_fill_rectangle(
                            BORDER_OFFSET,
                            GAME_AREA_TOP,
                            DISPLAY_WIDTH - BORDER_OFFSET,
                            DISPLAY_HEIGHT - BORDER_OFFSET,
                            DISPLAY_BLACK
                        );
                        maze_drawn = false; // Force maze redraw
                        pacman_init();
                    }
                }
            }
        }
    }
}

static void pacman_update_dpad(DPAD_STATUS dpad_status) {
    uint32_t current_time = get_current_ms();

    // Handle direction change from D-pad
    if (dpad_status.is_new) {
        switch (dpad_status.direction) {
        case DPAD_DIR_UP:    pacman_data.next_dir = DIR_UP;    break;
        case DPAD_DIR_RIGHT: pacman_data.next_dir = DIR_RIGHT; break;
        case DPAD_DIR_DOWN:  pacman_data.next_dir = DIR_DOWN;  break;
        case DPAD_DIR_LEFT:  pacman_data.next_dir = DIR_LEFT;  break;
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

    // Also update scared ghost animation if active
    if (pacman_data.power_pellet_active) {
        animated_sprite_update(&scared_ghost_animated);
    }

    // Update regular ghost animations
    animated_sprite_update(&blinky_animated);
    animated_sprite_update(&pinky_animated);
    animated_sprite_update(&inky_animated);
    animated_sprite_update(&clyde_animated);
}

// Function to render the score and lives in the status area
static void render_status_area(bool force_redraw) {
    // Check if there's a reason to redraw
    if (!force_redraw &&
        previous_score == pacman_game_engine.base_state.state_data.single.score &&
        previous_lives == pacman_game_engine.base_state.state_data.single.lives) {
        return;
    }

    // Clear the status area at the top without affecting the border
    display_fill_rectangle(2, 2, DISPLAY_WIDTH - 2, STATUS_START_Y - 1, DISPLAY_BLACK);

    // Draw score and lives
    char status_text[32];
    snprintf(status_text, sizeof(status_text), "Score: %lu Lives: %d",
        pacman_game_engine.base_state.state_data.single.score,
        pacman_game_engine.base_state.state_data.single.lives);
    display_set_cursor(2, 2);
#ifdef DISPLAY_MODULE_LCD
    display_write_string(status_text, Font_11x18, DISPLAY_WHITE);
#else
    display_write_string(status_text, Font_7x10, DISPLAY_WHITE);
#endif

    // Update previous values
    previous_score = pacman_game_engine.base_state.state_data.single.score;
    previous_lives = pacman_game_engine.base_state.state_data.single.lives;
}

// Function to clear a region and redraw border if needed
static void clear_and_redraw_border_if_needed(coord_t x, coord_t y) {
    // Check if position is near any border
    bool near_border = (x <= BORDER_OFFSET + TILE_SIZE ||
        x >= DISPLAY_WIDTH - BORDER_OFFSET - TILE_SIZE - 1 ||
        y <= GAME_AREA_TOP + TILE_SIZE ||
        y >= DISPLAY_HEIGHT - BORDER_OFFSET - TILE_SIZE - 1);

    // Clear the region
    display_clear_region(x, y, TILE_SIZE, TILE_SIZE);

    // Redraw border if needed
    if (near_border) {
        display_draw_border_at(BORDER_OFFSET, GAME_AREA_TOP, 2, 2);
        last_border_redraw_time = get_current_ms();
    }
}

// Function to clear previous positions of game elements
static void clear_previous_positions(void) {
    // Clear previous Pacman position if it moved
    if (previous_pacman_pos.x != pacman_data.pacman_pos.x ||
        previous_pacman_pos.y != pacman_data.pacman_pos.y) {
        clear_and_redraw_border_if_needed(previous_pacman_pos.x, previous_pacman_pos.y);
    }

    // Clear previous ghost positions
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        Ghost* ghost = &pacman_data.ghosts[i];
        if (!ghost->active) continue;

        if (previous_ghost_pos[i].x != ghost->pos.x ||
            previous_ghost_pos[i].y != ghost->pos.y) {
            clear_and_redraw_border_if_needed(previous_ghost_pos[i].x, previous_ghost_pos[i].y);
        }
    }
}

// Function to draw dots and power pellets
static void draw_dots_and_pellets(void) {
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
}

// Function to redraw the maze if needed
static void redraw_full_maze_if_needed(void) {
    uint32_t current_time = get_current_ms();

    // Redraw the maze if it hasn't been drawn yet or if enough time has passed
    if (!maze_drawn || (current_time - last_border_redraw_time) > 5000) {
        draw_maze();
        maze_drawn = true;
        last_border_redraw_time = current_time;
    }
}

// Function to draw all game elements (Pacman and ghosts)
static void draw_game_elements(void) {
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
}

static void pacman_render(void) {
    // Initialize display on first render
    if (first_render) {
        // Only set the flag, the maze will be drawn in redraw_full_maze_if_needed
        draw_maze();
        maze_drawn = true;
        render_status_area(true);
        first_render = false;
    }

    // Update status area if score or lives changed
    bool status_changed = (previous_score != pacman_game_engine.base_state.state_data.single.score) ||
        (previous_lives != pacman_game_engine.base_state.state_data.single.lives);
    if (status_changed) {
        render_status_area(true);
    }

    // Make sure maze is drawn
    redraw_full_maze_if_needed();

    // Clear previous positions
    clear_previous_positions();

    // Draw dots and pellets (they may be eaten, so redraw them)
    draw_dots_and_pellets();

    // Draw game elements (Pacman and ghosts)
    draw_game_elements();

    // Draw game over or win text
    if (pacman_game_engine.base_state.game_over) {
        char* message = (pacman_data.num_dots_remaining == 0) ?
            "YOU WIN!" : "GAME OVER";
#ifdef DISPLAY_MODULE_LCD
        display_write_string_centered(message, Font_11x18, 30, DISPLAY_WHITE);
#else
        display_write_string_centered(message, Font_7x10, 30, DISPLAY_WHITE);
#endif
    }

    // Periodically redraw border to ensure it's intact
    if (get_current_ms() % 500 == 0) {
        display_draw_border_at(BORDER_OFFSET, GAME_AREA_TOP, 2, 2);
        last_border_redraw_time = get_current_ms();
    }
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
        previous_ghost_pos[i].x = 0;
        previous_ghost_pos[i].y = 0;
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

    // Reset dirty rectangle tracking
    previous_pacman_pos.x = 0;
    previous_pacman_pos.y = 0;
    maze_drawn = false;
    previous_score = 0;
    previous_lives = 0;
    first_render = true;
    last_border_redraw_time = 0;

    // Reset game engine state
    pacman_game_engine.base_state.state_data.single.score = 0;
    pacman_game_engine.base_state.state_data.single.lives = 3;
    pacman_game_engine.base_state.paused = false;
    pacman_game_engine.base_state.game_over = false;
}
