/*
 * sys_conf.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_SYSTEM_CONF_H_
#define INC_SYSTEM_SYSTEM_CONF_H_

#include "stm32f4xx_hal.h"
#include "System/Clock/clock_conf.h"
#include "System/Peripherals/adc_conf.h"
#include "System/Peripherals/dma_conf.h"
#include "System/Peripherals/gpio_conf.h"
#include "System/Peripherals/uart_conf.h"
#include "System/Peripherals/usb_conf.h"
#include "System/Peripherals/timer_conf.h"
#include "System/Peripherals/spi_conf.h"
#include "Utils/debug_conf.h"

void System_Init(void);
void Error_Handler(void);

#endif /* INC_SYSTEM_SYSTEM_CONF_H_ */
