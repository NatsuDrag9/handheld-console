/*
 * dac_conf.c
 *
 *  Created on: Mar 16, 2025
 *      Author: rohitimandi
 */

#ifndef SRC_SYSTEM_PERIPHERALS_DAC_CONF_C_
#define SRC_SYSTEM_PERIPHERALS_DAC_CONF_C_

#include "System/Peripherals/dac_conf.h"

DAC_HandleTypeDef hdac;

void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */
  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}



#endif /* SRC_SYSTEM_PERIPHERALS_DAC_CONF_C_ */
