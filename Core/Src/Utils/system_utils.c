/*
 * system_utils.c
 *
 *  Created on: Dec 8, 2024
 *      Author: rohitimandi
 */


#include "Utils/system_utils.h"


void add_delay(uint32_t value) {
	HAL_Delay(value);
}

void blink_led1() {
	HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
//	add_delay(1000);
}

void blink_led2() {
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
//	add_delay(1000);
}

void blink_error_led() {
	HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
	add_delay(1000);
}
