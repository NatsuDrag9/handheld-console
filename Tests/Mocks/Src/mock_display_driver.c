#include "../Inc/mock_display_driver.h"
#include <string.h>

// Define the font data and structure
const uint16_t Font7x10_data[] = {
    // Mock font data
    0x0000
};

FontDef Font_7x10 = {
    .FontWidth = 7,
    .FontHeight = 10,
    .data = Font7x10_data
};

// Static state variables
uint8_t display_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static DisplayColor current_color = DISPLAY_BLACK;
static uint8_t display_initialized = 0;
static uint8_t border_drawn = 0;
static uint8_t screen_updated = 0;
static uint8_t error_displayed = 0;
static uint8_t scrollbar_drawn = 0;
static uint8_t thumb_positions[MAX_MENU_ITEMS] = { 0 };
static uint8_t num_thumb_draws = 0;

void mock_display_reset_state(void) {
    memset(display_buffer, 0, sizeof(display_buffer));
    cursor_x = 0;
    cursor_y = 0;
    current_color = DISPLAY_BLACK;
    display_initialized = 0;
    border_drawn = 0;
    screen_updated = 0;
    error_displayed = 0;
    scrollbar_drawn = 0;
    memset(thumb_positions, 0, sizeof(thumb_positions));
    num_thumb_draws = 0;

}

void mock_display_get_state(MockDisplayState* state) {
    state->cursor_x = cursor_x;
    state->cursor_y = cursor_y;
    state->current_color = current_color;
    state->initialized = display_initialized;
    state->border_drawn = border_drawn;
    state->screen_updated = screen_updated;
    state->error_displayed = error_displayed;
    state->scrollbar_drawn = scrollbar_drawn;
    memcpy(state->thumb_positions, thumb_positions, sizeof(thumb_positions));
    state->num_thumb_draws = num_thumb_draws;
}

void mock_display_get_buffer(uint8_t* buffer, uint16_t size) {
    memcpy(buffer, display_buffer, size);
}

// Mock display driver functions
void display_init(void) {
    display_initialized = 1;
    display_clear();
}

void display_clear(void) {
    memset(display_buffer, 0, sizeof(display_buffer));
    current_color = DISPLAY_BLACK;
    cursor_x = 0;
    cursor_y = 0;
    border_drawn = 0;
    screen_updated++;
}

void display_update(void) {
    screen_updated++;
}

void display_set_cursor(uint8_t x, uint8_t y) {
    cursor_x = x;
    cursor_y = y;
}

void display_write_string(char* str, FontDef font, DisplayColor color) {
    current_color = color;
    cursor_x += strlen(str) * font.FontWidth;
    // Set some bits in the buffer to simulate text
    if (color == DISPLAY_WHITE) {
        uint16_t buffer_index = cursor_y * (DISPLAY_WIDTH / 8) + (cursor_x / 8);
        if (buffer_index < sizeof(display_buffer)) {
            display_buffer[buffer_index] = 0xFF;  // Set some bits
        }
    }
    screen_updated++;
}

void display_write_string_centered(char* str, FontDef font, uint8_t y, DisplayColor color) {
    uint8_t str_width = strlen(str) * font.FontWidth;
    cursor_x = (DISPLAY_WIDTH - str_width) / 2;
    cursor_y = y;
    current_color = color;
    screen_updated++;

    // Set some bits in the buffer to simulate text
    if (color == DISPLAY_WHITE) {
        uint16_t buffer_index = cursor_y * (DISPLAY_WIDTH / 8) + (cursor_x / 8);
        if (buffer_index < sizeof(display_buffer)) {
            display_buffer[buffer_index] = 0xFF;  // Set some bits
        }
    }

    // Check if this is an error message - the string is taken from oled_init() in oled.c file
    if (strcmp(str, "Menu Init Error") == 0) {
        error_displayed = 1;
    }
}

void display_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color) {
    current_color = color;
    // Check if this is a scrollbar background
    if (x1 == DISPLAY_WIDTH - 5 && y1 == MENU_START_Y) {
        scrollbar_drawn = 1;
    }
    screen_updated++;
}

void display_fill_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color) {
    current_color = color;
    // Check if this is a scrollbar thumb
    if (x1 == DISPLAY_WIDTH - 4) {
        if (num_thumb_draws < MAX_MENU_ITEMS) {
            thumb_positions[num_thumb_draws] = y1;
            num_thumb_draws++;
        }
    }
    screen_updated++;
}

void display_draw_border(void) {
    border_drawn = 1;
    current_color = DISPLAY_WHITE;
    screen_updated++;
}

void display_draw_border_at(uint8_t x_offset, uint8_t y_offset, uint8_t dist_from_width, uint8_t dist_from_height) {
    border_drawn = 1;
    current_color = DISPLAY_WHITE;

    // Set buffer bits to simulate border
    uint16_t x2 = DISPLAY_WIDTH - dist_from_width;
    uint16_t y2 = DISPLAY_HEIGHT - dist_from_height;

    // Draw border in buffer using existing pixel drawing function
    for (uint8_t x = x_offset; x <= x2; x++) {
        display_draw_pixel(x, y_offset, DISPLAY_WHITE);  // Top line
        display_draw_pixel(x, y2, DISPLAY_WHITE);        // Bottom line
    }
    for (uint8_t y = y_offset; y <= y2; y++) {
        display_draw_pixel(x_offset, y, DISPLAY_WHITE);  // Left line
        display_draw_pixel(x2, y, DISPLAY_WHITE);        // Right line
    }

    screen_updated++;
}

void display_draw_pixel(uint8_t x, uint8_t y, DisplayColor color) {
    // Bounds checking
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }

    // Calculate byte position in the buffer
    // Each byte represents 8 horizontal pixels
    uint16_t byte_pos = (y * (DISPLAY_WIDTH / 8)) + (x / 8);

    // Calculate bit position within the byte (7 - to account for MSB first)
    uint8_t bit_pos = 7 - (x % 8);

    if (byte_pos < sizeof(display_buffer)) {
        if (color == DISPLAY_WHITE) {
            // Set pixel
            display_buffer[byte_pos] |= (1 << bit_pos);
        }
        else {
            // Clear pixel
            display_buffer[byte_pos] &= ~(1 << bit_pos);
        }
    }

    current_color = color;
    screen_updated++;
}

void display_draw_bitmap(uint8_t x, uint8_t y, const uint8_t* bitmap, uint8_t width, uint8_t height, DisplayColor color) {
    // Bounds checking
    if (bitmap == NULL || x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }

    // Calculate bytes per row in the bitmap
    uint8_t bytes_per_row = (width + 7) / 8;

    // Iterate through each row
    for (uint8_t y_pos = 0; y_pos < height; y_pos++) {
        // Stop if we go beyond display height
        if ((y + y_pos) >= DISPLAY_HEIGHT) break;

        // Iterate through each byte in the row
        for (uint8_t byte_idx = 0; byte_idx < bytes_per_row; byte_idx++) {
            uint8_t byte = bitmap[y_pos * bytes_per_row + byte_idx];

            // Process each bit in the byte
            for (uint8_t bit_idx = 0; bit_idx < 8; bit_idx++) {
                // Stop if we go beyond bitmap width
                if ((byte_idx * 8 + bit_idx) >= width) break;

                // Calculate x position for this bit
                uint8_t x_pos = x + (byte_idx * 8 + bit_idx);

                // Stop if we go beyond display width
                if (x_pos >= DISPLAY_WIDTH) break;

                // Check if bit is set in bitmap
                if (byte & (0x80 >> bit_idx)) {
                    // Draw pixel using the existing function
                    display_draw_pixel(x_pos, y + y_pos, color);
                }
            }
        }
    }

    current_color = color;
    screen_updated++;
}