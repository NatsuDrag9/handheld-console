/*
 * display_driver.h
 *
 *  Created on: Jan 11, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_DRIVERS_DISPLAY_DRIVER_H_
#define INC_CONSOLE_PERIPHERALS_DRIVERS_DISPLAY_DRIVER_H_

#include <stdint.h>
#include <Utils/misc_utils.h>
#include "string.h"
#include "../../../../Drivers/Display/Inc/ssd1306_conf_template.h"
#include "../../../../Drivers/Display/Inc/ssd1306.h"
#include "../../../../Drivers/Display/Inc/ssd1306_fonts.h"

 // Display dimensions
#ifndef UNITY_TEST
#define DISPLAY_WIDTH    SSD1306_WIDTH
#define DISPLAY_HEIGHT   SSD1306_HEIGHT
#else
#define DISPLAY_WIDTH    128
#define DISPLAY_HEIGHT   64
#endif

// Display colors
typedef enum {
    DISPLAY_BLACK = 0x00,
    DISPLAY_WHITE = 0x01
} DisplayColor;

// Display driver interface
void display_init(void);
void display_clear(void);
void display_update(void);
void display_fill_white(void);
void display_set_cursor(uint8_t x, uint8_t y);
void display_write_string(char* str, FontDef font, DisplayColor color);
void display_write_string_centered(char* str, FontDef font, uint8_t y, DisplayColor color);
void display_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color);
void display_fill_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color);
void display_draw_border(void);
void display_draw_pixel(uint8_t x, uint8_t y, DisplayColor color);
void display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t* bitmap, uint8_t width, uint8_t height, DisplayColor color);


#endif /* INC_CONSOLE_PERIPHERALS_DRIVERS_DISPLAY_DRIVER_H_ */
