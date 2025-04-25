/*
 * uart_driver.h
 *
 *  Created on: Apr 25, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_DRIVERS_UART_DRIVER_H_
#define INC_CONSOLE_PERIPHERALS_DRIVERS_UART_DRIVER_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "System/Peripherals/uart_conf.h"

/* UART Status Codes */
typedef enum {
    UART_OK = 0,
    UART_ERROR,
    UART_BUSY,
    UART_TIMEOUT
} UART_Status;

/* UART Port Selection */
typedef enum {
    UART_PORT_2 = 2,  // For ESP32 communication
    UART_PORT_3 = 3   // ST-Link Virtual COM port (reserved)
	// Add more UART ports here
} UART_Port;

/* Function Prototypes */

UART_Status UART_Init(void);
UART_Status UART_SendByte(UART_Port port, uint8_t data);
UART_Status UART_SendBuffer(UART_Port port, uint8_t *pData, uint16_t size, uint32_t timeout);
UART_Status UART_SendString(UART_Port port, const char *str, uint32_t timeout);
UART_Status UART_ReceiveByte(UART_Port port, uint8_t *pData, uint32_t timeout);
UART_Status UART_ReceiveBuffer(UART_Port port, uint8_t *pData, uint16_t size, uint32_t timeout);
bool UART_IsDataAvailable(UART_Port port);
UART_Status UART_FlushRxBuffer(UART_Port port);
UART_Status UART_EnableRxInterrupt(UART_Port port);
UART_Status UART_DisableRxInterrupt(UART_Port port);
UART_Status UART_RegisterRxCallback(UART_Port port, void (*callback)(uint8_t));

#endif /* INC_CONSOLE_PERIPHERALS_DRIVERS_UART_DRIVER_H_ */
