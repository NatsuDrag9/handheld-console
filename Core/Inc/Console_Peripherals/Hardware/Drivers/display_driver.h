/*
 * display_driver.h
 *
 *  Created on: Jan 11, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_DISPLAY_DRIVER_H_
#define INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_DISPLAY_DRIVER_H_

// Select the display module
//#define DISPLAY_MODULE_LCD 1
//#define DISPLAY_MODULE_OLED 1

#include <stdint.h>
#include <Utils/misc_utils.h>
#include "string.h"

#if defined(DISPLAY_MODULE_OLED)
#include "../../../../Drivers/Display/Inc/ssd1306_conf_template.h"
#include "../../../../Drivers/Display/Inc/ssd1306.h"
#include "../../../../Drivers/Display/Inc/ssd1306_fonts.h"
#endif

#if defined(DISPLAY_MODULE_LCD)
#include "../../../../Drivers/Display/Inc/ili9341.h"
#include "../../../../Drivers/Display/Inc/ili9341_fonts.h"
#endif

// Display dimensions
#ifndef UNITY_TEST

#if defined(DISPLAY_MODULE_OLED)
#define DISPLAY_WIDTH    SSD1306_WIDTH
#define DISPLAY_HEIGHT   SSD1306_HEIGHT
// Use uint8_t for coordinate types since OLED is small (128x64)
typedef uint8_t coord_t;

#elif defined(DISPLAY_MODULE_LCD)
#define DISPLAY_WIDTH    ILI9341_WIDTH
#define DISPLAY_HEIGHT   ILI9341_HEIGHT
// Use uint16_t for coordinate types since LCD is larger (240x320)
typedef uint16_t coord_t;

#else
// For unit-tests - default to smaller size
#define DISPLAY_WIDTH    128
#define DISPLAY_HEIGHT   64
typedef uint8_t coord_t;
#endif

#endif /* UNITY_TEST */

// Display colors
typedef enum {
    DISPLAY_BLACK = 0x00,
    DISPLAY_WHITE = 0x01
} DisplayColor;

// Define FontDef here if neither display is included
#if !defined(DISPLAY_MODULE_OLED) && !defined(DISPLAY_MODULE_LCD)
typedef struct {
    uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;
#endif

// Display driver interface
void display_init(void);
void display_clear(void);
void display_clear_region(coord_t x, coord_t y, coord_t width, coord_t height);
void display_update(void);
void display_fill_white(void);
void display_set_cursor(coord_t x, coord_t y);
void display_write_string(char* str, FontDef font, DisplayColor color);
void display_write_string_centered(char* str, FontDef font, coord_t y, DisplayColor color);
void display_draw_horizontal_line(coord_t x, coord_t y, coord_t length, DisplayColor color);
void display_draw_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2, DisplayColor color);
void display_fill_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2, DisplayColor color);
void display_draw_border(void);
void display_draw_border_at(coord_t x_offset, coord_t y_offset, coord_t dist_from_width, coord_t dist_from_height);
void display_draw_pixel(coord_t x, coord_t y, DisplayColor color);
void display_draw_bitmap(coord_t x, coord_t y, const uint8_t* bitmap, coord_t width, coord_t height, DisplayColor color);

#endif /* INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_DISPLAY_DRIVER_H_ */
