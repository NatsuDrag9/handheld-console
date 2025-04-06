/*
 * ili9341_fonts.h
 *
 *  Created on: Apr 2, 2025
 *      Author: rohitimandi
 */

#ifndef DISPLAY_INC_ILI9341_FONTS_H_
#define DISPLAY_INC_ILI9341_FONTS_H_

#include <stdint.h>

typedef struct {
    const uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;


extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#endif /* DISPLAY_INC_ILI9341_FONTS_H_ */
