/*
 * clock_conf.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_CLOCK_CLOCK_CONF_H_
#define INC_SYSTEM_CLOCK_CLOCK_CONF_H_

#include "main.h"

/* Exported functions prototypes */
void SystemClock_Config(void);

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#endif /* USE_FULL_ASSERT */

#endif /* INC_SYSTEM_CLOCK_CLOCK_CONF_H_ */
