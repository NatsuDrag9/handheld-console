/*
 * display_driver.c
 *
 *  Created on: Jan 11, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Drivers/display_driver.h"
#include "Utils/debug_conf.h"

#ifdef DISPLAY_MODULE_LCD
// Dirty rectangle tracking for LCD
static bool dirty_flag = false;               // Indicates if there are pending updates
static coord_t dirty_min_x = DISPLAY_WIDTH;   // Top-left X of dirty region
static coord_t dirty_min_y = DISPLAY_HEIGHT;  // Top-left Y of dirty region
static coord_t dirty_max_x = 0;               // Bottom-right X of dirty region
static coord_t dirty_max_y = 0;               // Bottom-right Y of dirty region

// Reset dirty region tracking
static void reset_dirty_region(void) {
    dirty_flag = false;
    dirty_min_x = DISPLAY_WIDTH;
    dirty_min_y = DISPLAY_HEIGHT;
    dirty_max_x = 0;
    dirty_max_y = 0;
}

// Update dirty region with new coordinates
static void update_dirty_region(coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
    // Ensure coordinates are in ascending order
    if (x2 < x1) {
        coord_t temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y2 < y1) {
        coord_t temp = y1;
        y1 = y2;
        y2 = temp;
    }

    // Update the dirty region
    if (x1 < dirty_min_x) dirty_min_x = x1;
    if (y1 < dirty_min_y) dirty_min_y = y1;
    if (x2 > dirty_max_x) dirty_max_x = x2;
    if (y2 > dirty_max_y) dirty_max_y = y2;

    dirty_flag = true;
}
#endif

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
    ILI9341_Init();
    reset_dirty_region();
#endif
}

void display_clear(void) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
#elif DISPLAY_MODULE_LCD
    ILI9341_FillScreen(ILI9341_BLACK);
    // Mark entire screen as clean since we just cleared it
    reset_dirty_region();
#endif
}

void display_update(void) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_UpdateScreen();
#elif DISPLAY_MODULE_LCD
    // The ILI9341 driver already updates the display immediately on each drawing operation
    // We just reset the dirty region tracking for the next frame
    reset_dirty_region();
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
    // Reset dirty region since we filled the whole screen
    reset_dirty_region();
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

    // Calculate the affected region
    coord_t str_width = strlen(str) * font.width;
    update_dirty_region(lcd_cursor_x, lcd_cursor_y,
                        lcd_cursor_x + str_width - 1,
                        lcd_cursor_y + font.height - 1);

    ILI9341_WriteString(lcd_cursor_x, lcd_cursor_y, str, font, ili_color, bg_color);

    // Update cursor position after writing
    lcd_cursor_x += str_width;
    // Check if we need to wrap to next line
    if (lcd_cursor_x >= DISPLAY_WIDTH) {
        lcd_cursor_x = 0;
        lcd_cursor_y += font.height;
    }
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

    // Update dirty region
    update_dirty_region(x, y, x + str_width - 1, y + font.height - 1);

    ILI9341_WriteString(x, y, str, font, ili_color, bg_color);
#endif
}

void display_draw_horizontal_line(coord_t x, coord_t y, coord_t length, DisplayColor color) {
#ifdef DISPLAY_MODULE_OLED
    for (coord_t i = 0; i < length; i++) {
        ssd1306_DrawPixel((uint8_t)(x + i), (uint8_t)y, translate_color(color));
    }
#elif DISPLAY_MODULE_LCD
    uint16_t ili_color = (color == DISPLAY_BLACK) ? ILI9341_BLACK : ILI9341_WHITE;

    // Update dirty region
    update_dirty_region(x, y, x + length - 1, y);

    // Draw a 1-pixel high rectangle as a horizontal line
    ILI9341_FillRectangle(x, y, length, 1, ili_color);
#endif
}


void display_draw_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2, DisplayColor color) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_DrawRectangle((uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2, translate_color(color));
#elif DISPLAY_MODULE_LCD
    uint16_t ili_color = (color == DISPLAY_BLACK) ? ILI9341_BLACK : ILI9341_WHITE;
    uint16_t width = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;

    // Update dirty region
    update_dirty_region(x1, y1, x2, y2);

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

    // Update dirty region
    update_dirty_region(x1, y1, x2, y2);

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

    // Update dirty region for single pixel
    update_dirty_region(x, y, x, y);

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

    // Update dirty region for the entire bitmap area
    update_dirty_region(x, y, x + width - 1, y + height - 1);

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

// New function to selectively clear a region (for sprite movement)
void display_clear_region(coord_t x, coord_t y, coord_t width, coord_t height) {
#ifdef DISPLAY_MODULE_OLED
    ssd1306_FillRectangle((uint8_t)x, (uint8_t)y,
                          (uint8_t)(x + width - 1),
                          (uint8_t)(y + height - 1), Black);
#elif DISPLAY_MODULE_LCD
    // Update dirty region
    update_dirty_region(x, y, x + width - 1, y + height - 1);

    ILI9341_FillRectangle(x, y, width, height, ILI9341_BLACK);
#endif
}

#ifdef DISPLAY_MODULE_LCD
// Get the current dirty rectangle state (LCD only)
bool display_is_dirty(void) {
    return dirty_flag;
}

// Get the current dirty rectangle bounds (LCD only)
void display_get_dirty_bounds(coord_t* min_x, coord_t* min_y, coord_t* max_x, coord_t* max_y) {
    if (min_x) *min_x = dirty_min_x;
    if (min_y) *min_y = dirty_min_y;
    if (max_x) *max_x = dirty_max_x;
    if (max_y) *max_y = dirty_max_y;
}
#endif
