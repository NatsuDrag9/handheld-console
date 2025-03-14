#ifndef MOCK_DISPLAY_DRIVER_H
#define MOCK_DISPLAY_DRIVER_H

#include <stdint.h>
#include "Console_Peripherals/oled.h"
#include "Console_Peripherals/Drivers/display_driver.h"

// Mock state tracking structure 
typedef struct {
    uint8_t cursor_x;
    uint8_t cursor_y;
    DisplayColor current_color;
    uint8_t initialized;
    uint8_t border_drawn;
    uint8_t screen_updated;
    uint8_t error_displayed;
    uint8_t scrollbar_drawn;
    uint8_t thumb_positions[MAX_MENU_ITEMS];  // Store each drawn position
    uint8_t num_thumb_draws;  // Count how many times thumb was drawn
} MockDisplayState;

// Declare the font data and definition as external
extern const uint16_t Font7x10_data[];
extern FontDef Font_7x10;

// Variables to track internal state
uint8_t display_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

// Helper functions to verify display state
void mock_display_reset_state(void);
void mock_display_get_state(MockDisplayState* state);
void mock_display_get_buffer(uint8_t* buffer, uint16_t size);

#endif // MOCK_DISPLAY_DRIVER_H