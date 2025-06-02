/*
 * joystick_driver.h
 *
 *  Created on: Dec 22, 2024
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_JOYSTICK_DRIVER_H_
#define INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_JOYSTICK_DRIVER_H_

#include <stdint.h>

// Core driver interface
void joystick_driver_init(void);
void joystick_driver_start_conversion(void);
uint8_t joystick_driver_read_button(void);
void joystick_driver_get_adc_values(uint16_t* x, uint16_t* y);

#ifndef UNITY_TEST
    #include "stm32f4xx_hal.h"
    // HAL Callbacks
    void joystick_driver_tim_callback(TIM_HandleTypeDef *htim);
    void joystick_driver_adc_callback(ADC_HandleTypeDef* hadc);
#else
    // Test functions
    void joystick_driver_set_values(uint16_t x, uint16_t y, uint8_t button);

    // Test stubs for callbacks
   void joystick_driver_tim_callback(void* htim);  // Using void* to avoid HAL dependency
   void joystick_driver_adc_callback(void* hadc);
   void __disable_irq(void);
   void __enable_irq(void);
#endif

#endif /* INC_CONSOLE_PERIPHERALS_HARDWARE_DRIVERS_JOYSTICK_DRIVER_H_ */
