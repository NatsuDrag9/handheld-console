/*
 * pacman_sprite.h
 *
 *  Created on: Feb 14, 2025
 *      Author: rohitimandi
 */

#ifndef INC_SPRITES_PACMAN_SPRITE_H_
#define INC_SPRITES_PACMAN_SPRITE_H_

#include "Console_Peripherals/types.h"
#include "sprite.h"

// Declare animated sprites
extern AnimatedSprite pacman_animated;
extern AnimatedSprite blinky_animated;  // Red ghost
extern AnimatedSprite pinky_animated;   // Pink ghost
extern AnimatedSprite inky_animated;    // Blue ghost
extern AnimatedSprite clyde_animated;   // Orange ghost
extern AnimatedSprite scared_ghost_animated;

// Static sprites
extern Sprite dot_sprite;
extern Sprite power_pellet_sprite;

#endif /* INC_SPRITES_PACMAN_SPRITE_H_ */
