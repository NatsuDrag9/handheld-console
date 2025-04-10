/*
 * dma_conf.c
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */


#include "System/Peripherals/dma_conf.h"

DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_dac1;
DMA_HandleTypeDef hdma_spi1_tx;

void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();  // For DAC
  __HAL_RCC_DMA2_CLK_ENABLE();  // For ADC

  // Configure the NVIC for DMA
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);  // For DAC
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);          // For DAC

  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);  // For ADC
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);          // For ADC

  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0); // For SPI
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
}
