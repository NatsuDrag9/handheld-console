/*
 * snake_sprite.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#include "Sprites/snake_sprite.h"

// Snake head bitmap (8x8 pixels) with vertical pair of eyes
// Two frames: normal and tongue out
static const uint8_t snake_head_bitmap[] = {
    0x7E, // 01111110  - Less rounded top
    0xFF, // 11111111  - Full width
	  0xE7, // 11100111  - Two eyes (** **)
    0xFF, // 11111111
    0xFF, // 11111111
	  0xE7, // 11100111  - Two eyes (** **)
    0xFF, // 11111111
    0x7E  // 01111110  - Less rounded bottom
};

static const uint8_t snake_head_tongue_bitmap[] = {
    0x7E, // 01111110  - Less rounded top
    0xFF, // 11111111  - Full width
	  0xE7, // 11100111  - Two eyes (** **)
    0xFF, // 11111111
    0xFF, // 11111111
	  0xE7, // 11100111  - Two eyes (** **)
    0xFF, // 11111111
    0x7F  // 01111111  - Tongue on right
};

// Snake body segment bitmap (8x8 pixels)
static const uint8_t snake_body_bitmap[] = {
    0xFF, // 11111111
    0xFF, // 11111111
    0xFF, // 11111111
    0xFF, // 11111111
    0xFF, // 11111111
    0xFF, // 11111111
    0xFF, // 11111111
    0xFF  // 11111111
};

// Food bitmap (8x8 pixels) - apple shape
static const uint8_t food_bitmap[] = {
    0x18, // 00011000 - Stem
    0x3C, // 00111100 - Top curve
    0x7E, // 01111110 - Upper body
    0xFF, // 11111111 - Middle
    0xFF, // 11111111 - Middle
    0x7E, // 01111110 - Lower body
    0x3C, // 00111100 - Bottom curve
    0x18  // 00011000 - Base
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

// Food
const Sprite food_sprite = {
    .bitmap = food_bitmap,
    .width = 8,
    .height = 8
};
