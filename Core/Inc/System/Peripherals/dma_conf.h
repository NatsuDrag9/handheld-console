/*
 * dma_conf.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_PERIPHERALS_DMA_CONF_H_
#define INC_SYSTEM_PERIPHERALS_DMA_CONF_H_

#include "stm32f4xx_hal.h"

extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_dac1;

void MX_DMA_Init(void);

#endif /* INC_SYSTEM_PERIPHERALS_DMA_CONF_H_ */
