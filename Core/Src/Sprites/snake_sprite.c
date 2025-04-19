/*
 * snake_sprite.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#include "Sprites/snake_sprite.h"

#ifdef DISPLAY_MODULE_LCD
// Snake head bitmap (16x16 pixels) with vertically aligned eyes for LCD
// Two frames: normal and tongue out
static const uint8_t snake_head_bitmap[] = {
    0x0F, 0xF0, 0x3F, 0xFC, 0x7F, 0xFE, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFE, 0x7F, 0xF2, 0x7F, 0xF3, 0xFF,
    0xFF, 0xFF, 0xFE, 0x7F, 0xFE, 0x7F, 0xFF, 0xFF,
    0xFF, 0xFF, 0x7F, 0xFE, 0x3F, 0xFC, 0x0F, 0xF0
};
static const uint8_t snake_head_tongue_bitmap[] = {
    0x0F, 0xF0, 0x3F, 0xFC, 0x7F, 0xFE, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFE, 0x7F, 0xF2, 0x7F, 0xF3, 0xFF,
    0xFF, 0xFF, 0xFE, 0x7F, 0xFE, 0x7F, 0xFF, 0xFF,
    0xFF, 0xFF, 0x7F, 0xFE, 0x3F, 0xFC, 0x0F, 0xF1
};

// Snake body segment bitmap (16x16 pixels) for LCD - squarer edges
static const uint8_t snake_body_bitmap[] = {
    0x0F, 0xF0, 0x3F, 0xFC, 0x7F, 0xFE, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x7F, 0xFE, 0x3F, 0xFC, 0x0F, 0xF0
};

// Food bitmap (16x16 pixels) - apple shape for LCD
static const uint8_t food_bitmap[] = {
    0x00, 0x00, 0x01, 0x80, 0x03, 0xC0, 0x03, 0xC0,
    0x07, 0xE0, 0x0F, 0xF0, 0x1F, 0xF8, 0x3F, 0xFC,
    0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x3F, 0xFC,
    0x1F, 0xF8, 0x0F, 0xF0, 0x07, 0xE0, 0x03, 0xC0
};

#else
// Snake head bitmap (8x8 pixels) with vertical pair of eyes for OLED
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

// Snake body segment bitmap (8x8 pixels) for OLED
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

// Food bitmap (8x8 pixels) - apple shape for OLED
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
#endif

// Define the sprites
const Sprite snake_head_frame1 = {
    .bitmap = snake_head_bitmap,
    .width = SPRITE_SIZE,
    .height = SPRITE_SIZE
};

const Sprite snake_head_frame2 = {
    .bitmap = snake_head_tongue_bitmap,
    .width = SPRITE_SIZE,
    .height = SPRITE_SIZE
};

const Sprite snake_body_sprite = {
    .bitmap = snake_body_bitmap,
    .width = SPRITE_SIZE,
    .height = SPRITE_SIZE
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
    .width = SPRITE_SIZE,
    .height = SPRITE_SIZE
};
