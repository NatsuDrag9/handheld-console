/*
 * joystick_callbacks.h
 *
 *  Created on: Dec 22, 2024
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVER_CALLBACKS_JOYSTICK_CALLBACKS_H_
#define INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVER_CALLBACKS_JOYSTICK_CALLBACKS_H_

#include "stm32f4xx_hal.h"

// HAL callbacks
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif /* INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVER_CALLBACKS_JOYSTICK_CALLBACKS_H_ */
