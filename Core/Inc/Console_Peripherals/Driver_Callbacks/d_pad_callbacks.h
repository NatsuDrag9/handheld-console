/*
 * d_pad_callbacks.h
 *
 *  Created on: Mar 7, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_DRIVER_CALLBACKS_D_PAD_CALLBACKS_H_
#define INC_CONSOLE_PERIPHERALS_DRIVER_CALLBACKS_D_PAD_CALLBACKS_H_

#include "stm32f4xx_hal.h"

// HAL EXTI callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif /* INC_CONSOLE_PERIPHERALS_DRIVER_CALLBACKS_D_PAD_CALLBACKS_H_ */
