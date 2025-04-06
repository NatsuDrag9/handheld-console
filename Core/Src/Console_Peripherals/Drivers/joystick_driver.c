/*
 * joystick_driver.c
 *
 *  Created on: Dec 22, 2024
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Drivers/joystick_driver.h"
#include "Console_Peripherals/joystick.h"

// Private variables for hardware interaction
static volatile uint16_t adc_buf[2];
static volatile uint8_t conversion_complete = 0;

#ifndef UNITY_TEST

#include "stm32f4xx_hal.h"

void joystick_driver_start_conversion(void) {
    if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, 2) != HAL_OK) {
    	Error_Handler();
    }
}

uint8_t joystick_driver_read_button(void) {
    return !HAL_GPIO_ReadPin(JS_Button_GPIO_Port, JS_Button_Pin);
}

void joystick_driver_get_adc_values(uint16_t* x, uint16_t* y) {
    *x = adc_buf[1];
    *y = adc_buf[0];
}

void joystick_driver_tim_callback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM6 && conversion_complete) {
        conversion_complete = 0;
        joystick_driver_start_conversion();
    }
}

void joystick_driver_adc_callback(ADC_HandleTypeDef* hadc) {
    if(hadc->Instance == ADC1) {
        conversion_complete = 1;
        HAL_ADC_Stop_DMA(&hadc1);
    }
}

void joystick_driver_init(void) {
	joystick_driver_start_conversion();

    if(HAL_TIM_Base_Start_IT(&htim6) != HAL_OK) {
            Error_Handler();
    }

    if(HAL_TIM_Base_Start_IT(&htim4) != HAL_OK) {
               Error_Handler();
       }
}

#endif
