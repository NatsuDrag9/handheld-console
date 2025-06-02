/*
 * joystick.c
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

 // joystick.c
#ifndef SRC_PERIPHERALS_JOYSTICK_C_
#define SRC_PERIPHERALS_JOYSTICK_C_

#include <Console_Peripherals/Hardware/Drivers/joystick_driver.h>
#include <Console_Peripherals/Hardware/joystick.h>

static uint8_t last_direction = JS_DIR_CENTERED;
static volatile JoystickStatus joystick_status = { JS_DIR_CENTERED, 0, 0 };

uint8_t calculate_direction(uint16_t x, uint16_t y) {
    // Clamp out-of-range values
    x = (x > 4095) ? 4095 : x;
    y = (y > 4095) ? 4095 : y;

    int x_dir = X_DIR_CENTER;
    int y_dir = Y_DIR_CENTER;

    if (x <= X_POS_THRES_LOW) {
        x_dir = X_DIR_LEFT;
    }
    else if (x >= X_POS_THRES_HIGH) {
        x_dir = X_DIR_RIGHT;
    }

    if (y <= Y_POS_THRES_LOW) {
        y_dir = Y_DIR_DOWN;
    }
    else if (y >= Y_POS_THRES_HIGH) {
        y_dir = Y_DIR_UP;
    }

    if (x_dir == X_DIR_LEFT) {
        if (y_dir == Y_DIR_UP) return JS_DIR_LEFT_UP;
        if (y_dir == Y_DIR_DOWN) return JS_DIR_LEFT_DOWN;
        return JS_DIR_LEFT;
    }
    if (x_dir == X_DIR_RIGHT) {
        if (y_dir == Y_DIR_UP) return JS_DIR_RIGHT_UP;
        if (y_dir == Y_DIR_DOWN) return JS_DIR_RIGHT_DOWN;
        return JS_DIR_RIGHT;
    }
    if (y_dir == Y_DIR_UP) return JS_DIR_UP;
    if (y_dir == Y_DIR_DOWN) return JS_DIR_DOWN;

    return JS_DIR_CENTERED;
}

void update_joystick_status(void) {
    uint16_t x, y;
    joystick_driver_get_adc_values(&x, &y);

    uint8_t current_dir = calculate_direction(x, y);
    uint8_t current_button = joystick_driver_read_button();

    if (current_dir != last_direction || current_button != joystick_status.button) {
        last_direction = current_dir;
        __disable_irq();
        joystick_status.direction = current_dir;
        joystick_status.button = current_button;
        joystick_status.is_new = 1;
        __enable_irq();
    }
}

JoystickStatus joystick_get_status(void) {
    JoystickStatus status;
    __disable_irq();
    status = joystick_status;
    joystick_status.is_new = 0;
    __enable_irq();
    return status;
}

void joystick_init(void) {
    joystick_driver_init();
}

#endif /* SRC_PERIPHERALS_JOYSTICK_C_ */
