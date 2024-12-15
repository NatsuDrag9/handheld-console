/*
 * dma_conf.c
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */


#include "System/Peripherals/dma_conf.h"

void MX_DMA_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}
