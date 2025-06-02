/*
 * audio_driver.h
 *
 *  Created on: Mar 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_AUDIO_DRIVER_H_
#define INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_AUDIO_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "Game_Engine/game_engine_conf.h"

void audio_driver_init();
void audio_driver_write_dac(uint16_t value);
void audio_driver_set_volume(uint8_t volume); // 0-100%
void audio_driver_mute(bool mute);
void audio_driver_set_frequency(uint16_t frequency); // New function to set note frequency
void audio_driver_stop(void); // Explicit stop function


#ifndef UNITY_TEST
extern DAC_HandleTypeDef hdac;
#endif

#endif /* INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_AUDIO_DRIVER_H_ */
