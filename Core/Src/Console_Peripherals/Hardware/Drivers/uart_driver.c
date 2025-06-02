/*
 * uart_driver.c
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 */

#include <Console_Peripherals/Hardware/Drivers/uart_driver.h>
#include "Utils/debug_conf.h"

 /* External variable declarations */
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

/* Private variables */
static void (*uart2_rx_callback)(uint8_t) = NULL;
static void (*uart3_rx_callback)(uint8_t) = NULL;
static uint8_t uart2_rx_buffer[1];
static uint8_t uart3_rx_buffer[1];

/* Helper function to get HAL UART handle based on port */
static UART_HandleTypeDef* UART_GetHandle(UART_Port port) {
    switch (port) {
    case UART_PORT_2:
        return &huart2;
    case UART_PORT_3:
        return &huart3;
    default:
        return NULL;
    }
}

/* UART Driver initialization */
UART_Status UART_Init(void) {
    /* UART2 and UART3 are already initialized in MX_USARTx_UART_Init functions */
    /* Add any additional driver layer initialization here if needed */
    return UART_OK;
}

/* Send a single byte */
UART_Status UART_SendByte(UART_Port port, uint8_t data) {
    UART_HandleTypeDef* huart = UART_GetHandle(port);

    if (huart == NULL) {
        return UART_ERROR;
    }

    if (HAL_UART_Transmit(huart, &data, 1, 100) != HAL_OK) {
        return UART_ERROR;
    }

    return UART_OK;
}

/* Send a buffer of data */
UART_Status UART_SendBuffer(UART_Port port, uint8_t* pData, uint16_t size, uint32_t timeout) {
    UART_HandleTypeDef* huart = UART_GetHandle(port);

    if (huart == NULL || pData == NULL || size == 0) {
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit(huart, pData, size, timeout);

    switch (status) {
    case HAL_OK:
        return UART_OK;
    case HAL_BUSY:
        return UART_BUSY;
    case HAL_TIMEOUT:
        return UART_TIMEOUT;
    default:
        return UART_ERROR;
    }
}

/* Send a null-terminated string */
UART_Status UART_SendString(UART_Port port, const char* str, uint32_t timeout) {
    if (str == NULL) {
        return UART_ERROR;
    }

    uint16_t length = strlen(str);
    return UART_SendBuffer(port, (uint8_t*)str, length, timeout);
}

/* Receive a single byte (blocking) */
UART_Status UART_ReceiveByte(UART_Port port, uint8_t* pData, uint32_t timeout) {
    UART_HandleTypeDef* huart = UART_GetHandle(port);

    if (huart == NULL || pData == NULL) {
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Receive(huart, pData, 1, timeout);

    switch (status) {
    case HAL_OK:
        return UART_OK;
    case HAL_BUSY:
        return UART_BUSY;
    case HAL_TIMEOUT:
        return UART_TIMEOUT;
    default:
        return UART_ERROR;
    }
}

/* Receive data into buffer */
UART_Status UART_ReceiveBuffer(UART_Port port, uint8_t* pData, uint16_t size, uint32_t timeout) {
    UART_HandleTypeDef* huart = UART_GetHandle(port);

    if (huart == NULL || pData == NULL || size == 0) {
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Receive(huart, pData, size, timeout);

    switch (status) {
    case HAL_OK:
        return UART_OK;
    case HAL_BUSY:
        return UART_BUSY;
    case HAL_TIMEOUT:
        return UART_TIMEOUT;
    default:
        return UART_ERROR;
    }
}

/* Check if data is available to read */
bool UART_IsDataAvailable(UART_Port port) {
    UART_HandleTypeDef* huart = UART_GetHandle(port);

    if (huart == NULL) {
        return false;
    }

    /* Check if RX register has data (RXNE flag is set) */
    if (port == UART_PORT_2) {
        return (USART2->SR & USART_SR_RXNE) != 0;
    }
    else if (port == UART_PORT_3) {
        return (USART3->SR & USART_SR_RXNE) != 0;
    }

    return false;
}

/* Flush the UART receive buffer */
UART_Status UART_FlushRxBuffer(UART_Port port) {
    UART_HandleTypeDef* huart = UART_GetHandle(port);

    if (huart == NULL) {
        return UART_ERROR;
    }

    /* Read and discard data until no more is available */
    uint8_t dummy;
    while (UART_IsDataAvailable(port)) {
        if (port == UART_PORT_2) {
            dummy = (uint8_t)(USART2->DR);
        }
        else if (port == UART_PORT_3) {
            dummy = (uint8_t)(USART3->DR);
        }
        /* Small delay to ensure stable reading */
        for (volatile int i = 0; i < 10; i++);
    }

    return UART_OK;
}

/* Enable UART receive interrupt */
UART_Status UART_EnableRxInterrupt(UART_Port port) {
    UART_HandleTypeDef* huart = UART_GetHandle(port);

    if (huart == NULL) {
        return UART_ERROR;
    }

    /* Start receiving in interrupt mode (1 byte at a time) */
    if (port == UART_PORT_2) {
        if (HAL_UART_Receive_IT(huart, uart2_rx_buffer, 1) != HAL_OK) {
            return UART_ERROR;
        }
    }
    else if (port == UART_PORT_3) {
        if (HAL_UART_Receive_IT(huart, uart3_rx_buffer, 1) != HAL_OK) {
            return UART_ERROR;
        }
    }

    return UART_OK;
}

/* Disable UART receive interrupt */
UART_Status UART_DisableRxInterrupt(UART_Port port) {
    UART_HandleTypeDef* huart = UART_GetHandle(port);

    if (huart == NULL) {
        return UART_ERROR;
    }

    /* Abort any ongoing receive operations */
    if (HAL_UART_AbortReceive_IT(huart) != HAL_OK) {
        return UART_ERROR;
    }

    return UART_OK;
}

/* Register callback function for UART receive */
UART_Status UART_RegisterRxCallback(UART_Port port, void (*callback)(uint8_t)) {
    if (port == UART_PORT_2) {
        uart2_rx_callback = callback;
    }
    else if (port == UART_PORT_3) {
        uart3_rx_callback = callback;
    }
    else {
        return UART_ERROR;
    }

    return UART_OK;
}

/* HAL UART Rx Complete callback - implement in this file to automatically restart reception */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    /* Check which UART triggered the callback */
    if (huart->Instance == USART2) {

        /* Call user callback if registered */
        if (uart2_rx_callback != NULL) {
            uart2_rx_callback(uart2_rx_buffer[0]);
        }
//        else {
//            DEBUG_PRINTF(false, "No user callback registered!");
//        }

        /* Restart reception in IT mode */
        HAL_StatusTypeDef restart_result = HAL_UART_Receive_IT(huart, uart2_rx_buffer, 1);
        if (restart_result != HAL_OK) {
        	// If restart fails, try to recover
			HAL_UART_AbortReceive_IT(huart);
        	HAL_UART_Receive_IT(huart, uart2_rx_buffer, 1);

//            DEBUG_PRINTF(false, "Failed to restart UART reception: %d\n", restart_result);
        }
    }
    else if (huart->Instance == USART3) {
        /* Call user callback if registered */
        if (uart3_rx_callback != NULL) {
            uart3_rx_callback(uart3_rx_buffer[0]);
        }

        /* Restart reception in IT mode */
        HAL_UART_Receive_IT(huart, uart3_rx_buffer, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    // Clear error flags and restart reception
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);

    if (huart->Instance == USART2) {
        // Restart reception
        HAL_UART_Receive_IT(huart, uart2_rx_buffer, 1);
    }

    if(huart->Instance == USART3) {
        // Restart reception
        HAL_UART_Receive_IT(huart, uart3_rx_buffer, 1);
    }
}
