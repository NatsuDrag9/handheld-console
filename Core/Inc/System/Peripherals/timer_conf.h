/*
 * timer_conf.h
 *
 *  Created on: Dec 15, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_PERIPHERALS_TIMER_CONF_H_
#define INC_SYSTEM_PERIPHERALS_TIMER_CONF_H_

#include "main.h"

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim4;

void MX_TIM6_Init(void);
void MX_TIM4_Init(void);

#endif /* INC_SYSTEM_PERIPHERALS_TIMER_CONF_H_ */
