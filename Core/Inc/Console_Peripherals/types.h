/*
 * types.h
 *
 *  Created on: Jan 8, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_TYPES_H_
#define INC_CONSOLE_PERIPHERALS_TYPES_H_

#include "stdint.h" // For uint8_t

 /* Joystick types and macros */

typedef struct {
    uint8_t direction;
    uint8_t is_new;
    uint8_t button;
} JoystickStatus;

// X and Y axis thresholds
#define X_POS_THRES_LOW 1800	// Threshold for joystick left hold
#define X_POS_THRES_HIGH 2500	// Threshold for joystick right hold
#define Y_POS_THRES_LOW	1800	// Threshold for joystick bottom hold
#define Y_POS_THRES_HIGH 2500	// Threshold for joystick up hold

// X and Y direction
#define X_DIR_LEFT -1
#define X_DIR_RIGHT 1
#define X_DIR_CENTER 0
#define Y_DIR_UP 1
#define Y_DIR_DOWN -1
#define Y_DIR_CENTER 0

// Joystick direction
#define JS_DIR_LEFT_UP 1
#define JS_DIR_LEFT_DOWN 2
#define JS_DIR_LEFT 3
#define JS_DIR_RIGHT_UP 4
#define JS_DIR_RIGHT_DOWN 5
#define JS_DIR_RIGHT 6
#define JS_DIR_UP 7
#define JS_DIR_DOWN 8
#define JS_DIR_CENTERED 0

/* D-PAD types and macros */
typedef struct {
	uint8_t direction;
	uint8_t is_new;
} DPAD_STATUS;

#define DPAD_DIR_UP 1
#define DPAD_DIR_RIGHT 2
#define DPAD_DIR_DOWN 3
#define DPAD_DIR_LEFT 4

/* Audio types */
// Melody note structure
typedef struct {
    uint16_t frequency;  // Frequency in Hz
    uint16_t duration;   // Duration in ms
} Note;

#endif /* INC_CONSOLE_PERIPHERALS_TYPES_H_ */
