/*
 * misc_utils.h
 *
 *  Created on: Dec 8, 2024
 *      Author: rohitimandi
 */

#ifndef INC_UTILS_MISC_UTILS_H_
#define INC_UTILS_MISC_UTILS_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "System/pin_definitions.h"

void blink_led1();
void blink_led2();
void blink_error_led();
void add_delay(uint32_t);
uint32_t get_current_ms(void);

void init_random(void);
uint32_t get_random(void);

#endif /* INC_UTILS_MISC_UTILS_H_ */
