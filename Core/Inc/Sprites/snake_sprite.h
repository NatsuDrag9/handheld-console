/*
 * snake_sprite.h
 *
 *  Created on: Jan 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_SPRITES_SNAKE_SPRITE_H_
#define INC_SPRITES_SNAKE_SPRITE_H_

#include "Sprites/sprite.h"

// Snake sprites (8x8 pixels each)
extern const Sprite snake_head_sprite;
extern const Sprite snake_body_sprite;
extern const Sprite snake_head_frames[];
extern const Sprite food_sprite;

// Animated snake head (for tongue flicking animation)
extern AnimatedSprite snake_head_animated;

#endif /* INC_SPRITES_SNAKE_SPRITE_H_ */
