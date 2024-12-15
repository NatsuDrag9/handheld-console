/*
 * uart_conf.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_PERIPHERALS_UART_CONF_H_
#define INC_SYSTEM_PERIPHERALS_UART_CONF_H_

#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart3;

void MX_USART3_UART_Init(void);

#endif /* INC_SYSTEM_PERIPHERALS_UART_CONF_H_ */
