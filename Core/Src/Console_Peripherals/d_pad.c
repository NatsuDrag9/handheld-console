/*
 * d_pad.c
 *
 *  Created on: Mar 7, 2025
 *      Author: rohitimandi
 */

#ifndef SRC_PERIPHERALS_D_PAD_C_
#define SRC_PERIPHERALS_D_PAD_C_

#include "Console_Peripherals/d_pad.h"
#include "Utils/debug_conf.h"

static uint8_t last_direction = 0;
static uint8_t d_pad_changed = 0;
static volatile DPAD_STATUS d_pad_status = { 0, 0 };

void update_d_pad_status(void) {
    // Read the D-pad states from push button interface
    uint8_t up_state = dpad_pin_up_get_state();
    uint8_t right_state = dpad_pin_right_get_state();
    uint8_t down_state = dpad_pin_down_get_state();
    uint8_t left_state = dpad_pin_left_get_state();

    uint8_t current_dir = 0;

    // Priority order in case multiple buttons are pressed
    if (up_state) {
        current_dir = DPAD_DIR_UP;
    } else if (right_state) {
        current_dir = DPAD_DIR_RIGHT;
    } else if (down_state) {
        current_dir = DPAD_DIR_DOWN;
    } else if (left_state) {
        current_dir = DPAD_DIR_LEFT;
    }

//    DEBUG_PRINTF(false, "Button states: U:%d R:%d D:%d L:%d Dir:%d Last:%d\n",
//                     up_state, right_state, down_state, left_state, current_dir, last_direction);


    if (current_dir != last_direction) {
            last_direction = current_dir;
            __disable_irq();
            d_pad_status.direction = current_dir;
            d_pad_status.is_new = 1;
            d_pad_changed = 1;
//            DEBUG_PRINTF(true, "*** SETTING is_new=1 for direction %d ***\n", current_dir);
            __enable_irq();
    }
}


DPAD_STATUS d_pad_get_status(void) {
    DPAD_STATUS status;
    __disable_irq();
    status = d_pad_status;
//    if (d_pad_status.is_new) {
//        DEBUG_PRINTF(true, "*** CLEARING is_new flag, was 1 for direction %d ***\n",
//                   d_pad_status.direction);
//    }
    d_pad_status.is_new = 0;
    __enable_irq();
    return status;
}

uint8_t d_pad_direction_changed(void) {
    __disable_irq();
    uint8_t changed = d_pad_changed;
    d_pad_changed = 0;  // Clear the flag
    __enable_irq();
    return changed;
}

#endif /* SRC_PERIPHERALS_D_PAD_C_ */
