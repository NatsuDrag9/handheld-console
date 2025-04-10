///*
// * display_driver.c
// *
// *  Created on: Jan 11, 2025
// *      Author: rohitimandi
// */
//
//#include "Console_Peripherals/Drivers/display_driver.h"
//
//// Translate DisplayColor to SSD1306_COLOR
//static SSD1306_COLOR translate_color(DisplayColor color) {
//    return (color == DISPLAY_BLACK) ? Black : White;
//}
//
//void display_init(void) {
//    ssd1306_Init();
//}
//
//void display_clear(void) {
//    ssd1306_Fill(Black);
//    ssd1306_UpdateScreen();
//}
//
//void display_update(void) {
//    ssd1306_UpdateScreen();
//}
//
//void display_set_cursor(uint8_t x, uint8_t y) {
//    ssd1306_SetCursor(x, y);
//}
//
///* Changes OLED background to white */
//void display_fill_white(){
//	  ssd1306_Fill(White);
//	  ssd1306_UpdateScreen();
//	  add_delay(100);
//}
//
//void display_write_string(char* str, FontDef font, DisplayColor color) {
//    ssd1306_WriteString(str, font, translate_color(color));
//}
//
//void display_write_string_centered(char* str, FontDef font, uint8_t y, DisplayColor color) {
//    uint8_t str_width = strlen(str) * font.FontWidth;
//    uint8_t x = (DISPLAY_WIDTH - str_width) / 2;
//    ssd1306_SetCursor(x, y);
//    ssd1306_WriteString(str, font, translate_color(color));
//}
//
//void display_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color) {
//    ssd1306_DrawRectangle(x1, y1, x2, y2, translate_color(color));
//}
//
//void display_fill_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color) {
//    ssd1306_FillRectangle(x1, y1, x2, y2, translate_color(color));
//}
//
//void display_draw_border(void) {
//    ssd1306_DrawRectangle(1, 1, DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1, White);
//}
//
//void display_draw_border_at(uint8_t x_offset, uint8_t y_offset, uint8_t dist_from_width, uint8_t dist_from_height) {
//    ssd1306_DrawRectangle(x_offset, y_offset, DISPLAY_WIDTH-dist_from_width, DISPLAY_HEIGHT-dist_from_height, White);
//}
//
//void display_draw_pixel(uint8_t x, uint8_t y, DisplayColor color) {
//    ssd1306_DrawPixel(x, y, color);
//}
//
//void display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t* bitmap,
//                        uint8_t width, uint8_t height, DisplayColor color) {
//    ssd1306_DrawBitmap(x, y, bitmap, width, height, color);
//}


/*
 * display_driver.c
 *
 *  Created on: Jan 11, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Drivers/display_driver.h"
#include "Utils/debug_conf.h"

#ifdef DISPLAY_MODULE_OLED
// Translate DisplayColor to SSD1306_COLOR
static SSD1306_COLOR translate_color(DisplayColor color) {
    return (color == DISPLAY_BLACK) ? Black : White;
}
#endif

#ifdef DISPLAY_MODULE_LCD
static uint16_t lcd_cursor_x = 0;
static uint16_t lcd_cursor_y = 0;
#endif

void display_init(void) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_Init();
#elif DISPLAY_MODULE_LCD
//    DEBUG_PRINTF(false, "Initializing LCD module\n");
    ILI9341_Init();
#endif
}

void display_clear(void) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
#elif DISPLAY_MODULE_LCD
    ILI9341_FillScreen(ILI9341_BLACK);
#endif
}

void display_update(void) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_UpdateScreen();
#elif DISPLAY_MODULE_LCD
    // ILI9341 doesn't need explicit update as it updates immediately
#endif
}

void display_fill_white(void) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_Fill(White);
    ssd1306_UpdateScreen();
#ifdef UNITY_TEST
    // add_delay is defined as empty in test mode
#else
    add_delay(100);
#endif
#elif DISPLAY_MODULE_LCD

    ILI9341_FillScreen(ILI9341_WHITE);
    // No delay needed for LCD as it updates immediately
#endif
}

void display_set_cursor(coord_t x, coord_t y) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_SetCursor((uint8_t)x, (uint8_t)y);
#elif DISPLAY_MODULE_LCD
    // ILI9341 doesn't have a persistent cursor concept
    // Position is set during write operations
    lcd_cursor_x = (uint16_t)x;
    lcd_cursor_y = (uint16_t)y;
#endif
}

void display_write_string(char* str, FontDef font, DisplayColor color) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_WriteString(str, font, translate_color(color));
#elif DISPLAY_MODULE_LCD
    DEBUG_PRINTF(false, "String: %s\n", str);
    uint16_t ili_color = (color == DISPLAY_BLACK) ? ILI9341_BLACK : ILI9341_WHITE;
    uint16_t bg_color = (color == DISPLAY_BLACK) ? ILI9341_WHITE : ILI9341_BLACK;
    ILI9341_WriteString(lcd_cursor_x, lcd_cursor_y, str, font, ili_color, bg_color);

    // Update cursor position after writing
    lcd_cursor_x += strlen(str) * font.width;
    // Check if we need to wrap to next line
    if (lcd_cursor_x >= DISPLAY_WIDTH) {
    	lcd_cursor_x = 0;
    	lcd_cursor_y += font.height;
    }

    // We need current cursor position, but ILI9341 doesn't track it
    // Use a default position if cursor position is not available
    //    ILI9341_WriteString(0, 0, str, font, ili_color, bg_color);
#endif
}

void display_write_string_centered(char* str, FontDef font, coord_t y, DisplayColor color) {
#ifdef DISPLAY_MODULE_OLED
    uint8_t str_width = strlen(str) * font.FontWidth;
    uint8_t x = (DISPLAY_WIDTH - str_width) / 2;
    ssd1306_SetCursor(x, (uint8_t)y);
    ssd1306_WriteString(str, font, translate_color(color));
#elif DISPLAY_MODULE_LCD
    uint16_t ili_color = (color == DISPLAY_BLACK) ? ILI9341_BLACK : ILI9341_WHITE;
    uint16_t bg_color = (color == DISPLAY_BLACK) ? ILI9341_WHITE : ILI9341_BLACK;

    // Calculate centered position
    uint16_t str_width = strlen(str) * font.width;
    uint16_t x = (DISPLAY_WIDTH - str_width) / 2;

    ILI9341_WriteString(x, y, str, font, ili_color, bg_color);
#endif
}

void display_draw_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2, DisplayColor color) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_DrawRectangle((uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2, translate_color(color));
#elif DISPLAY_MODULE_LCD
    uint16_t ili_color = (color == DISPLAY_BLACK) ? ILI9341_BLACK : ILI9341_WHITE;
    uint16_t width = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;

    // Draw horizontal lines
    ILI9341_FillRectangle(x1, y1, width, 1, ili_color);
    ILI9341_FillRectangle(x1, y2, width, 1, ili_color);

    // Draw vertical lines
    ILI9341_FillRectangle(x1, y1, 1, height, ili_color);
    ILI9341_FillRectangle(x2, y1, 1, height, ili_color);
#endif
}

void display_fill_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2, DisplayColor color) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_FillRectangle((uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2, translate_color(color));
#elif DISPLAY_MODULE_LCD
    uint16_t ili_color = (color == DISPLAY_BLACK) ? ILI9341_BLACK : ILI9341_WHITE;
    uint16_t width = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;
    ILI9341_FillRectangle(x1, y1, width, height, ili_color);
#endif
}

void display_draw_border(void) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_DrawRectangle(1, 1, DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1, White);
#elif DISPLAY_MODULE_LCD
    display_draw_rectangle(1, 1, DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1, DISPLAY_WHITE);
#endif
}

void display_draw_border_at(coord_t x_offset, coord_t y_offset, coord_t dist_from_width, coord_t dist_from_height) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_DrawRectangle((uint8_t)x_offset, (uint8_t)y_offset,
                          (uint8_t)(DISPLAY_WIDTH-dist_from_width),
                          (uint8_t)(DISPLAY_HEIGHT-dist_from_height), White);
#elif DISPLAY_MODULE_LCD
    display_draw_rectangle(x_offset, y_offset,
                          DISPLAY_WIDTH-dist_from_width,
                          DISPLAY_HEIGHT-dist_from_height, DISPLAY_WHITE);
#endif
}

void display_draw_pixel(coord_t x, coord_t y, DisplayColor color) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_DrawPixel((uint8_t)x, (uint8_t)y, translate_color(color));
#elif DISPLAY_MODULE_LCD
    uint16_t ili_color = (color == DISPLAY_BLACK) ? ILI9341_BLACK : ILI9341_WHITE;
    ILI9341_DrawPixel(x, y, ili_color);
#endif
}

void display_draw_bitmap(coord_t x, coord_t y, const uint8_t* bitmap,
                        coord_t width, coord_t height, DisplayColor color) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_DrawBitmap((uint8_t)x, (uint8_t)y, bitmap, (uint8_t)width, (uint8_t)height, translate_color(color));
#elif DISPLAY_MODULE_LCD
    // For ILI9341, we would need to convert the bitmap format
    // This is a simplified implementation
    uint16_t fg_color = (color == DISPLAY_BLACK) ? ILI9341_BLACK : ILI9341_WHITE;
    uint16_t bg_color = (color == DISPLAY_BLACK) ? ILI9341_WHITE : ILI9341_BLACK;

    for (uint16_t i = 0; i < height; i++) {
        for (uint16_t j = 0; j < width; j++) {
            // Get the bit from bitmap
            uint8_t byte = bitmap[i * ((width + 7) / 8) + (j / 8)];
            uint8_t bit = byte & (1 << (7 - (j % 8)));

            if (bit) {
                ILI9341_DrawPixel(x + j, y + i, fg_color);
            } else {
                // Optional: uncomment if you want to draw background pixels
                // ILI9341_DrawPixel(x + j, y + i, bg_color);
            }
        }
    }
#endif
}
