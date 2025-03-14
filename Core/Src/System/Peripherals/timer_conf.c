/*
 * timer_conf.c
 *
 *  Created on: Dec 15, 2024
 *      Author: rohitimandi
 */

#include "System/Peripherals/timer_conf.h"

TIM_HandleTypeDef htim6;

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */
	__HAL_RCC_TIM6_CLK_ENABLE();
  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 95; // Since APB1 clock is at 96 MHz
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 9999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */
   HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  /* USER CODE END TIM6_Init 2 */

}
