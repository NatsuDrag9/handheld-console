/*
 * sprite.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#include "Sprites/sprite.h"
#include <math.h>
#include "Utils/system_utils.h"  // For get_current_ms()

void sprite_draw(const Sprite* sprite, uint8_t x, uint8_t y, DisplayColor color) {
    display_draw_bitmap(x, y, sprite->bitmap, sprite->width, sprite->height, color);
}

void sprite_draw_rotated(const Sprite* sprite, uint8_t x, uint8_t y, uint8_t angle, DisplayColor color) {
	// Convert angle to radians using FPU
    float rad = angle * 3.14159f / 180.0f;
    float sin_a = sinf(rad);
    float cos_a = cosf(rad);

    // For each pixel in sprite
    for(uint8_t sy = 0; sy < sprite->height; sy++) {
        for(uint8_t sx = 0; sx < sprite->width; sx++) {
            // Get bit position in bitmap
            uint8_t byte_idx = sy * ((sprite->width + 7) / 8) + sx / 8;
            uint8_t bit_idx = 7 - (sx % 8);

            // If this pixel is set in the bitmap
            if(sprite->bitmap[byte_idx] & (1 << bit_idx)) {
                // Calculate rotated position
                float dx = (float)(sx - sprite->width/2);
                float dy = (float)(sy - sprite->height/2);
                int16_t rx = x + (int16_t)(dx * cos_a - dy * sin_a + sprite->width/2);
                int16_t ry = y + (int16_t)(dx * sin_a + dy * cos_a + sprite->height/2);

                // Draw pixel if in bounds
                if(rx >= 0 && rx < DISPLAY_WIDTH && ry >= 0 && ry < DISPLAY_HEIGHT) {
                    display_draw_pixel(rx, ry, color);
                }
            }
        }
    }
}

void sprite_draw_scaled(const Sprite* sprite, uint8_t x, uint8_t y, float scale, DisplayColor color) {
    uint8_t scaled_width = (uint8_t)(sprite->width * scale);
    uint8_t scaled_height = (uint8_t)(sprite->height * scale);

    // For each pixel in the scaled output
    for(uint8_t dy = 0; dy < scaled_height; dy++) {
        for(uint8_t dx = 0; dx < scaled_width; dx++) {
            // Map back to original sprite coordinates
            uint8_t sx = (uint8_t)(dx / scale);
            uint8_t sy = (uint8_t)(dy / scale);

            // Get bit position in bitmap
            uint8_t byte_idx = sy * ((sprite->width + 7) / 8) + sx / 8;
            uint8_t bit_idx = 7 - (sx % 8);

            // If this pixel is set in the bitmap
            if(sprite->bitmap[byte_idx] & (1 << bit_idx)) {
                // Draw pixel if in bounds
                uint8_t px = x + dx;
                uint8_t py = y + dy;
                if(px < DISPLAY_WIDTH && py < DISPLAY_HEIGHT) {
                    display_draw_pixel(px, py, color);
                }
            }
        }
    }
}

//void animated_sprite_update(AnimatedSprite* sprite) {
//    uint32_t current_time = get_current_ms();
//
//    // Update frame if enough time has passed
//    if(current_time - sprite->last_update >= sprite->frame_delay) {
//        sprite->current_frame = (sprite->current_frame + 1) % sprite->num_frames;
//        sprite->last_update = current_time;
//    }
//}
//
//void animated_sprite_draw(const AnimatedSprite* sprite, uint8_t x, uint8_t y, DisplayColor color) {
//    // Draw current frame
//    sprite_draw(&sprite->frames[sprite->current_frame], x, y, color);
//}
