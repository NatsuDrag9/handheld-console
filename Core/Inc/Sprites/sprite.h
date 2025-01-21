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

// Sprite operations
void sprite_draw(const Sprite* sprite, uint8_t x, uint8_t y, DisplayColor color);
void sprite_draw_rotated(const Sprite* sprite, uint8_t x, uint8_t y, uint8_t angle, DisplayColor color);
void sprite_draw_scaled(const Sprite* sprite, uint8_t x, uint8_t y, float scale, DisplayColor color);
void animated_sprite_update(AnimatedSprite* sprite);
void animated_sprite_draw(const AnimatedSprite* sprite, uint8_t x, uint8_t y, DisplayColor color);
void animated_sprite_update(AnimatedSprite* sprite);

#endif /* INC_SPRITES_SPRITE_H_ */
