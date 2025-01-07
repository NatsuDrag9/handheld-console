/*
 * sys_conf.c
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */


#include <System/system_conf.h>

void System_Init(void)
{
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
//  MX_USB_OTG_FS_PCD_Init();
  MX_TIM6_Init();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  blink_error_led();
  }
  /* USER CODE END Error_Handler_Debug */
}
