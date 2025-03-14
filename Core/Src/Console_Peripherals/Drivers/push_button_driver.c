/*
 * push_button_driver.c
 *
 *  Created on: Dec 28, 2024
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Drivers/push_button_driver.h"

uint8_t PB_Driver_ReadPin1(void) {
    return (HAL_GPIO_ReadPin(PB_GPIO_Port, PB_1_Pin) == GPIO_PIN_SET) ? 1 : 0;
}

uint8_t PB_Driver_ReadPin2(void) {
    return (HAL_GPIO_ReadPin(PB_GPIO_Port, PB_2_Pin) == GPIO_PIN_SET) ? 1 : 0;
}

uint8_t read_dpad_pin_left(void) {
    return (HAL_GPIO_ReadPin(D_PAD_GPIO_Port_2, D_PAD_Pin_Left) == GPIO_PIN_SET) ? 1 : 0;
}

uint8_t read_dpad_pin_right(void) {
    return (HAL_GPIO_ReadPin(D_PAD_GPIO_Port_1, D_PAD_Pin_Right) == GPIO_PIN_SET) ? 1 : 0;
}

uint8_t read_dpad_pin_up(void) {
    return (HAL_GPIO_ReadPin(D_PAD_GPIO_Port_1, D_PAD_Pin_Up) == GPIO_PIN_SET) ? 1 : 0;
}

uint8_t read_dpad_pin_down(void) {
    return (HAL_GPIO_ReadPin(D_PAD_GPIO_Port_1, D_PAD_Pin_Down) == GPIO_PIN_SET) ? 1 : 0;
}

uint32_t PB_Driver_GetTick(void) {
    return HAL_GetTick();
}
