/*
 * snake_sprite.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#include "Sprites/snake_sprite.h"

// Snake head bitmap (8x8 pixels)
// Two frames: normal and tongue out
static const uint8_t snake_head_bitmap[] = {
    0x3C, // 00111100
    0x7E, // 01111110
    0xFF, // 11111111
    0xFF, // 11111111
    0xE7, // 11100111
    0xC3, // 11000011
    0xC3, // 11000011
    0x66  // 01100110
};

static const uint8_t snake_head_tongue_bitmap[] = {
    0x3C, // 00111100
    0x7E, // 01111110
    0xFF, // 11111111
    0xFF, // 11111111
    0xE7, // 11100111
    0xC3, // 11000011
    0xC3, // 11000011
    0x6E  // 01101110 (tongue sticking out)
};

// Snake body segment bitmap (8x8 pixels)
static const uint8_t snake_body_bitmap[] = {
    0x3C, // 00111100
    0x7E, // 01111110
    0xFF, // 11111111
    0xFF, // 11111111
    0xFF, // 11111111
    0xFF, // 11111111
    0x7E, // 01111110
    0x3C  // 00111100
};

// Define the sprites
const Sprite snake_head_frame1 = {
    .bitmap = snake_head_bitmap,
    .width = 8,
    .height = 8
};

const Sprite snake_head_frame2 = {
    .bitmap = snake_head_tongue_bitmap,
    .width = 8,
    .height = 8
};

const Sprite snake_body_sprite = {
    .bitmap = snake_body_bitmap,
    .width = 8,
    .height = 8
};

// Array of animation frames for snake head
const Sprite snake_head_frames[] = {
    snake_head_frame1,
    snake_head_frame2
};

// Animated snake head sprite
AnimatedSprite snake_head_animated = {
    .frames = snake_head_frames,
    .num_frames = 2,
    .current_frame = 0,
    .frame_delay = 500, // 500ms between frames
    .last_update = 0
};
