// Tests/Mocks/Inc/stm32f4xx_hal.h 
#ifndef __STM32F4xx_HAL_H
#define __STM32F4xx_HAL_H

#include <stdint.h>

typedef struct {
    uint32_t dummy;
} ADC_HandleTypeDef;

typedef struct {
    uint32_t dummy;
} DMA_HandleTypeDef;

typedef struct {
    uint32_t dummy;
} UART_HandleTypeDef;

typedef struct {
    uint32_t dummy;
} PCD_HandleTypeDef;

typedef struct {
    uint32_t dummy;
} TIM_HandleTypeDef;

typedef struct {
    uint32_t dummy;
} SPI_HandleTypeDef;

// Add HAL status type that's commonly used
typedef enum {
    HAL_OK = 0x00,
    HAL_ERROR = 0x01,
    HAL_BUSY = 0x02,
    HAL_TIMEOUT = 0x03
} HAL_StatusTypeDef;

#endif