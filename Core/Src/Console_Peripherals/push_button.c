/*
 * push_button.c
 *
 *  Created on: Dec 28, 2024
 *      Author: rohitimandi
 */

#include "Console_Peripherals/push_button.h"

// Debounce configuration
#define DEBOUNCE_DELAY_MS    100

// Button instances
static Button_TypeDef button1 = { 0 };
static Button_TypeDef button2 = { 0 };
static Button_TypeDef d_pad_left = { 0 };
static Button_TypeDef d_pad_right = { 0 };
static Button_TypeDef d_pad_up = { 0 };
static Button_TypeDef d_pad_down = { 0 };

void pb_init(void) {
    button1.current_state = 0;
    button1.last_reading = 0;
    button1.last_debounce_time = 0;
    button1.pressed = 0;

    button2.current_state = 0;
    button2.last_reading = 0;
    button2.last_debounce_time = 0;
    button2.pressed = 0;

    d_pad_left.current_state = 0;
    d_pad_left.last_reading = 0;
    d_pad_left.last_debounce_time = 0;
    d_pad_left.pressed = 0;

    d_pad_right.current_state = 0;
    d_pad_right.last_reading = 0;
    d_pad_right.last_debounce_time = 0;
    d_pad_right.pressed = 0;

    d_pad_up.current_state = 0;
    d_pad_up.last_reading = 0;
    d_pad_up.last_debounce_time = 0;
    d_pad_up.pressed = 0;

    d_pad_down.current_state = 0;
    d_pad_down.last_reading = 0;
    d_pad_down.last_debounce_time = 0;
    d_pad_down.pressed = 0;
}

void process_button(Button_TypeDef* button, uint8_t current_reading, uint32_t current_time) {
    button->pressed = 0;  // Reset pressed flag

    // If the switch reading changed, due to noise or pressing
    if (current_reading != button->last_reading) {
        // Reset the debouncing timer
        button->last_debounce_time = current_time;
    }

    // If enough time has passed, check if the state has changed
    if ((current_time - button->last_debounce_time) > DEBOUNCE_DELAY_MS) {
        // If the button state has changed
        if (current_reading != button->current_state) {
            button->current_state = current_reading;

            // Set pressed flag only on rising edge (button press)
            if (button->current_state == 1) {
                button->pressed = 1;
            }
        }
    }

    button->last_reading = current_reading;
}

uint8_t pb1_get_state(void) {
    process_button(&button1, PB_Driver_ReadPin1(), PB_Driver_GetTick());
    return button1.pressed;
}

uint8_t pb2_get_state(void) {
    process_button(&button2, PB_Driver_ReadPin2(), PB_Driver_GetTick());
    return button2.pressed;
}

uint8_t dpad_pin_left_get_state(void) {
    process_button(&d_pad_left, read_dpad_pin_left(), PB_Driver_GetTick());
    return d_pad_left.pressed;
}

uint8_t dpad_pin_right_get_state(void) {
    process_button(&d_pad_right, read_dpad_pin_right(), PB_Driver_GetTick());
    return d_pad_right.pressed;
}

uint8_t dpad_pin_up_get_state(void) {
    process_button(&d_pad_up, read_dpad_pin_up(), PB_Driver_GetTick());
    return d_pad_up.pressed;
}

uint8_t dpad_pin_down_get_state(void) {
    process_button(&d_pad_down, read_dpad_pin_down(), PB_Driver_GetTick());
    return d_pad_down.pressed;
}
