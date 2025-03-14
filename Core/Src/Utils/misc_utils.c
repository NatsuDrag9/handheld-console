/*
 * misc_utils.c
 *
 *  Created on: Dec 8, 2024
 *      Author: rohitimandi
 */


#include <Utils/misc_utils.h>

static uint32_t random_seed = 1;

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

uint32_t get_current_ms(void) {
    return HAL_GetTick();
}

void init_random(void) {
	random_seed = HAL_GetTick();
}

uint32_t get_random(void) {
    random_seed = random_seed * 1103515245 + 12345;
    return random_seed;
}
