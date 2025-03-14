/*
 * sprite.c
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#include "Sprites/sprite.h"
#include <math.h>
#include <Utils/misc_utils.h>  // For get_current_ms()

void sprite_draw(const Sprite* sprite, uint16_t x, uint16_t y, DisplayColor color) {
    display_draw_bitmap(x, y, sprite->bitmap, sprite->width, sprite->height, color);
}

void sprite_draw_rotated(const Sprite* sprite, uint16_t x, uint16_t y, uint16_t angle, DisplayColor color) {
    float rad = angle * 3.14159f / 180.0f;
    float sin_a = sinf(rad);
    float cos_a = cosf(rad);

    for(uint16_t sy = 0; sy < sprite->height; sy++) {
        for(uint16_t sx = 0; sx < sprite->width; sx++) {
            uint8_t byte_idx = sy * ((sprite->width + 7) / 8) + sx / 8;
            uint8_t bit_idx = 7 - (sx % 8);

            if(sprite->bitmap[byte_idx] & (1 << bit_idx)) {
                float dx = (float)(sx - sprite->width/2);
                float dy = (float)(sy - sprite->height/2);
                int16_t rx = x + (int16_t)(dx * cos_a - dy * sin_a + sprite->width/2);
                int16_t ry = y + (int16_t)(dx * sin_a + dy * cos_a + sprite->height/2);

                if(rx >= 0 && rx < DISPLAY_WIDTH && ry >= 0 && ry < DISPLAY_HEIGHT) {
                    display_draw_pixel(rx, ry, color);
                }
            }
        }
    }
}

void sprite_draw_scaled(const Sprite* sprite, uint16_t x, uint16_t y, float scale, DisplayColor color) {
    uint16_t scaled_width = (uint16_t)(sprite->width * scale);
    uint16_t scaled_height = (uint16_t)(sprite->height * scale);

    for(uint16_t dy = 0; dy < scaled_height; dy++) {
        for(uint16_t dx = 0; dx < scaled_width; dx++) {
            uint16_t sx = (uint16_t)(dx / scale);
            uint16_t sy = (uint16_t)(dy / scale);

            uint8_t byte_idx = sy * ((sprite->width + 7) / 8) + sx / 8;
            uint8_t bit_idx = 7 - (sx % 8);

            if(sprite->bitmap[byte_idx] & (1 << bit_idx)) {
                uint16_t px = x + dx;
                uint16_t py = y + dy;
                if(px < DISPLAY_WIDTH && py < DISPLAY_HEIGHT) {
                    display_draw_pixel(px, py, color);
                }
            }
        }
    }
}

void animated_sprite_update(AnimatedSprite* sprite) {
    uint32_t current_time = get_current_ms();
    if(current_time - sprite->last_update >= sprite->frame_delay) {
        sprite->current_frame = (sprite->current_frame + 1) % sprite->num_frames;
        sprite->last_update = current_time;
    }
}

void animated_sprite_draw(const AnimatedSprite* sprite, uint16_t x, uint16_t y, DisplayColor color) {
    sprite_draw(&sprite->frames[sprite->current_frame], x, y, color);
}
