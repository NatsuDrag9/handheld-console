/*
 * status_bar_sprite.h
 *
 *  Created on: May 1, 2025
 *      Author: rohitimandi
 */

#ifndef INC_SPRITES_STATUS_BAR_SPRITE_H_
#define INC_SPRITES_STATUS_BAR_SPRITE_H_

#include "Console_Peripherals/types.h"
#include "sprite.h"

// Dimensions for status bar icons
#define STATUS_ICON_WIDTH  16
#define STATUS_ICON_HEIGHT 16

// Declare status bar sprites
extern const uint8_t wifi_connected_icon[];
extern const uint8_t wifi_disconnected_icon[];

#endif /* INC_SPRITES_STATUS_BAR_SPRITE_H_ */
