/*
 * joystick_callbacks.c
 *
 *  Created on: Dec 22, 2024
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Driver_Callbacks/joystick_callbacks.h"
#include "Console_Peripherals/Drivers/joystick_driver.h"
#include "Console_Peripherals/joystick.h"

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1) {
        joystick_driver_adc_callback(hadc);
        update_joystick_status();  // Call logic update after new ADC values
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        joystick_driver_tim_callback(htim);
    }
}
