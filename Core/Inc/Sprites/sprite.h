/*
 * sprite.h
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_SPRITES_SPRITE_H_
#define INC_SPRITES_SPRITE_H_

#include "Console_Peripherals/Drivers/display_driver.h"

typedef struct {
    const uint8_t* bitmap;    // Bitmap data
    uint8_t width;           // Sprite width
    uint8_t height;          // Sprite height
} Sprite;

typedef struct {
    const Sprite* frames;     // Array of sprite frames
    uint8_t num_frames;       // Number of frames
    uint8_t current_frame;    // Current frame index
    uint32_t frame_delay;      // Delay between frames
    uint32_t last_update;     // Time of last frame update
} AnimatedSprite;

// Macros
#define SPRITE_SIZE  8    // Size of sprites in pixels
#define BORDER_OFFSET     8    // Offset from screen border
#define GAME_AREA_TOP 12       // Offset from top to draw border as score and lives are displayed
#define TILE_SIZE 8

// Sprite operations
void sprite_draw(const Sprite* sprite, uint16_t x, uint16_t y, DisplayColor color);
void sprite_draw_rotated(const Sprite* sprite, uint16_t x, uint16_t y, uint16_t angle, DisplayColor color);
void sprite_draw_scaled(const Sprite* sprite, uint16_t x, uint16_t y, float scale, DisplayColor color);
void animated_sprite_update(AnimatedSprite* sprite);
void animated_sprite_draw(const AnimatedSprite* sprite, uint16_t x, uint16_t y, DisplayColor color);

#endif /* INC_SPRITES_SPRITE_H_ */
