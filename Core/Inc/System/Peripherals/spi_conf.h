/*
 * spi_conf.h
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_PERIPHERALS_SPI_CONF_H_
#define INC_SYSTEM_PERIPHERALS_SPI_CONF_H_

#include "stm32f4xx_hal.h"
#include "System/system_conf.h"

extern SPI_HandleTypeDef hspi1;

void MX_SPI1_Init(void);

#endif /* INC_SYSTEM_PERIPHERALS_SPI_CONF_H_ */
