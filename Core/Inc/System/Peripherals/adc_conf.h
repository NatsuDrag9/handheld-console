/*
 * adc_conf.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_PERIPHERALS_ADC_CONF_H_
#define INC_SYSTEM_PERIPHERALS_ADC_CONF_H_

#include "stm32f4xx_hal.h"
#include "System/system_conf.h"

extern ADC_HandleTypeDef hadc1;

void MX_ADC1_Init(void);

#endif /* INC_SYSTEM_PERIPHERALS_ADC_CONF_H_ */
