/*
 * joystick.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_JOYSTICK_H_
#define INC_CONSOLE_PERIPHERALS_JOYSTICK_H_

#include "main.h"
#include <stdint.h>

/* X and Y axis thresholds */
#define X_POS_THRES_LOW 1800	// Threshold for joystick left hold
#define X_POS_THRES_HIGH 2500	// Threshold for joystick right hold
#define Y_POS_THRES_LOW	1800	// Threshold for joystick bottom hold
#define Y_POS_THRES_HIGH 2500	// Threshold for joystick up hold

/* X and Y direction */
#define X_DIR_LEFT -1
#define X_DIR_RIGHT 1
#define X_DIR_CENTER 0
#define Y_DIR_UP 1
#define Y_DIR_DOWN -1
#define Y_DIR_CENTER 0

/* Joystick direction */
#define JS_DIR_LEFT_UP 1
#define JS_DIR_LEFT_DOWN 2
#define JS_DIR_LEFT 3
#define JS_DIR_RIGHT_UP 4
#define JS_DIR_RIGHT_DOWN 5
#define JS_DIR_RIGHT 6
#define JS_DIR_UP 7
#define JS_DIR_DOWN 8
#define JS_DIR_CENTERED 0

typedef struct {
    uint8_t direction;
    uint8_t is_new;
    uint8_t button;
} JoystickStatus;


void joystick_init(void);
JoystickStatus joystick_get_status();

#endif /* INC_CONSOLE_PERIPHERALS_JOYSTICK_H_ */
