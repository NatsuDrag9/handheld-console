/*
 * display_driver.c
 *
 *  Created on: Jan 11, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Drivers/display_driver.h"

// Translate DisplayColor to SSD1306_COLOR
static SSD1306_COLOR translate_color(DisplayColor color) {
    return (color == DISPLAY_BLACK) ? Black : White;
}

void display_init(void) {
    ssd1306_Init();
}

void display_clear(void) {
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
}

void display_update(void) {
    ssd1306_UpdateScreen();
}

void display_set_cursor(uint8_t x, uint8_t y) {
    ssd1306_SetCursor(x, y);
}

/* Changes OLED background to white */
void display_fill_white(){
	  ssd1306_Fill(White);
	  ssd1306_UpdateScreen();
	  add_delay(100);
}

void display_write_string(char* str, FontDef font, DisplayColor color) {
    ssd1306_WriteString(str, font, translate_color(color));
}

void display_write_string_centered(char* str, FontDef font, uint8_t y, DisplayColor color) {
    uint8_t str_width = strlen(str) * font.FontWidth;
    uint8_t x = (DISPLAY_WIDTH - str_width) / 2;
    ssd1306_SetCursor(x, y);
    ssd1306_WriteString(str, font, translate_color(color));
}

void display_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color) {
    ssd1306_DrawRectangle(x1, y1, x2, y2, translate_color(color));
}

void display_fill_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color) {
    ssd1306_FillRectangle(x1, y1, x2, y2, translate_color(color));
}

void display_draw_border(void) {
    ssd1306_DrawRectangle(1, 1, DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1, White);
}

void display_draw_border_at(uint8_t x_offset, uint8_t y_offset, uint8_t dist_from_width, uint8_t dist_from_height) {
    ssd1306_DrawRectangle(x_offset, y_offset, DISPLAY_WIDTH-dist_from_width, DISPLAY_HEIGHT-dist_from_height, White);
}

void display_draw_pixel(uint8_t x, uint8_t y, DisplayColor color) {
    ssd1306_DrawPixel(x, y, color);
}

void display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t* bitmap,
                        uint8_t width, uint8_t height, DisplayColor color) {
    ssd1306_DrawBitmap(x, y, bitmap, width, height, color);
}
