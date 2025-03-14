/*
 * pacman_sprite.c
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 */

#include "Sprites/pacman_sprite.h"

// Pacman animation frames (8x8 pixels)
static const uint8_t PACMAN_FRAME_1[] = {
    0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C  // Full circle
};

static const uint8_t PACMAN_FRAME_2[] = {
    0x3C, 0x7E, 0xFF, 0xFF, 0xF0, 0xE0, 0x40, 0x00  // Open mouth
};

// Ghost animation frames (8x8 pixels)
static const uint8_t GHOST_FRAME_1[] = {
    0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xDB, 0x99  // Normal ghost
};

static const uint8_t GHOST_FRAME_2[] = {
    0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0xDB  // Alternate legs
};

// Scared ghost frames
static const uint8_t SCARED_GHOST_FRAME_1[] = {
    0x3C, 0x7E, 0xDB, 0xBD, 0xBD, 0xDB, 0x7E, 0x3C  // Scared ghost
};

// Static sprites
static const uint8_t DOT_SPRITE[] = {
    0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00  // Small dot
};

static const uint8_t POWER_PELLET_SPRITE[] = {
    0x00, 0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x00  // Power pellet
};

// Initialize static sprites
Sprite dot_sprite = {
    .bitmap = DOT_SPRITE,
    .width = 8,
    .height = 8
};

Sprite power_pellet_sprite = {
    .bitmap = POWER_PELLET_SPRITE,
    .width = 8,
    .height = 8
};

// Define sprite frames
static const Sprite pacman_frames[] = {
    { .bitmap = PACMAN_FRAME_1, .width = 8, .height = 8 },
    { .bitmap = PACMAN_FRAME_2, .width = 8, .height = 8 }
};

static const Sprite ghost_frames[] = {
    { .bitmap = GHOST_FRAME_1, .width = 8, .height = 8 },
    { .bitmap = GHOST_FRAME_2, .width = 8, .height = 8 }
};

static const Sprite scared_ghost_frames[] = {
    { .bitmap = SCARED_GHOST_FRAME_1, .width = 8, .height = 8 }
};

// Initialize animated sprites
AnimatedSprite pacman_animated = {
    .frames = pacman_frames,
    .num_frames = 2,
    .current_frame = 0,
    .frame_delay = 200,  // ms per frame
    .last_update = 0
};

AnimatedSprite blinky_animated = {
    .frames = ghost_frames,
    .num_frames = 2,
    .current_frame = 0,
    .frame_delay = 300,  // ms per frame
    .last_update = 0
};

AnimatedSprite pinky_animated = {
    .frames = ghost_frames,
    .num_frames = 2,
    .current_frame = 0,
    .frame_delay = 300,  // ms per frame
    .last_update = 0
};

AnimatedSprite inky_animated = {
    .frames = ghost_frames,
    .num_frames = 2,
    .current_frame = 0,
    .frame_delay = 300,  // ms per frame
    .last_update = 0
};

AnimatedSprite clyde_animated = {
    .frames = ghost_frames,
    .num_frames = 2,
    .current_frame = 0,
    .frame_delay = 300,  // ms per frame
    .last_update = 0
};

AnimatedSprite scared_ghost_animated = {
    .frames = scared_ghost_frames,
    .num_frames = 1,
    .current_frame = 0,
    .frame_delay = 0,  // Static frame
    .last_update = 0
};
