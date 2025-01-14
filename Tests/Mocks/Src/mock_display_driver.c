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