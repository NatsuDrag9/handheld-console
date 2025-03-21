/*
 * audio_driver.h
 *
 *  Created on: Mar 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_DRIVERS_AUDIO_DRIVER_H_
#define INC_CONSOLE_PERIPHERALS_DRIVERS_AUDIO_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

void audio_driver_init();
void audio_driver_write_dac(uint16_t value);
void audio_driver_set_volume(uint8_t volume); // 0-100%
void audio_driver_mute(bool mute);

extern DAC_HandleTypeDef hdac;

#endif /* INC_CONSOLE_PERIPHERALS_DRIVERS_AUDIO_DRIVER_H_ */
