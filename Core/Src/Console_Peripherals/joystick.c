/*
 * joystick.c
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

// joystick.c
#ifndef SRC_PERIPHERALS_JOYSTICK_C_
#define SRC_PERIPHERALS_JOYSTICK_C_

#include "Console_Peripherals/joystick.h"
#include "stm32f4xx_hal.h"

/* Private variables */
static volatile uint16_t adc_buf[2];
static volatile uint8_t conversion_complete = 0;
static uint8_t last_direction = JS_DIR_CENTERED;
static volatile JoystickStatus joystick_status = {JS_DIR_CENTERED, 0, 0};

static uint8_t calculate_direction(void) {
    if (adc_buf[0] > 4095 || adc_buf[1] > 4095) {
        return JS_DIR_CENTERED;
    }

    int16_t x = adc_buf[1];
    int16_t y = adc_buf[0];

    int x_dir = X_DIR_CENTER;
    int y_dir = Y_DIR_CENTER;

    if (x < X_POS_THRES_LOW) {
        x_dir = X_DIR_LEFT;
    } else if (x > X_POS_THRES_HIGH) {
        x_dir = X_DIR_RIGHT;
    }

    if (y < Y_POS_THRES_LOW) {
        y_dir = Y_DIR_DOWN;
    } else if (y > Y_POS_THRES_HIGH) {
        y_dir = Y_DIR_UP;
    }

    if (x_dir == X_DIR_LEFT) {
        if (y_dir == Y_DIR_UP) return JS_DIR_LEFT_UP;
        if (y_dir == Y_DIR_DOWN) return JS_DIR_LEFT_DOWN;
        return JS_DIR_LEFT;
    }
    if (x_dir == X_DIR_RIGHT) {
        if (y_dir == Y_DIR_UP) return JS_DIR_RIGHT_UP;
        if (y_dir == Y_DIR_DOWN) return JS_DIR_RIGHT_DOWN;
        return JS_DIR_RIGHT;
    }
    if (y_dir == Y_DIR_UP) return JS_DIR_UP;
    if (y_dir == Y_DIR_DOWN) return JS_DIR_DOWN;

    return JS_DIR_CENTERED;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
        conversion_complete = 1;
        HAL_ADC_Stop_DMA(&hadc1);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        if(conversion_complete) {
            conversion_complete = 0;
            HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, 2);

            uint8_t current_dir = calculate_direction();
            uint8_t current_button = !HAL_GPIO_ReadPin(JS_Button_GPIO_Port, JS_Button_Pin);

            if(current_dir != last_direction || current_button != joystick_status.button) {
                last_direction = current_dir;
                __disable_irq();
                joystick_status.direction = current_dir;
                joystick_status.button = current_button;
                joystick_status.is_new = 1;
                __enable_irq();
            }
        }
    }
}

JoystickStatus joystick_get_status(void)
{
    JoystickStatus status;
    __disable_irq();
    status = joystick_status;
    joystick_status.is_new = 0;
    __enable_irq();
    return status;
}

void joystick_init(void)
{
    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, 2);
    if (status != HAL_OK) {
        for(int i = 0; i < 5; i++) {
            blink_error_led();
        }
        Error_Handler();
    }

    HAL_TIM_Base_Start_IT(&htim6);
}

#endif /* SRC_PERIPHERALS_JOYSTICK_C_ */
