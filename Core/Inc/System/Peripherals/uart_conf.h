/*
 * uart_conf.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_PERIPHERALS_UART_CONF_H_
#define INC_SYSTEM_PERIPHERALS_UART_CONF_H_

#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3; // Default used by ST-Link Virtual COM port

void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void); // Default used by ST-Link Virtual COM port

#endif /* INC_SYSTEM_PERIPHERALS_UART_CONF_H_ */
