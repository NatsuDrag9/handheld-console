/* USER CODE BEGIN Header */
///**
//  ******************************************************************************
//  * @file           : main.h
//  * @brief          : Header for main.c file.
//  *                   This file contains the common defines of the application.
//  ******************************************************************************
//  * @attention
//  *
//  * Copyright (c) 2024 STMicroelectronics.
//  * All rights reserved.
//  *
//  * This software is licensed under terms that can be found in the LICENSE file
//  * in the root directory of this software component.
//  * If no LICENSE file comes with this software, it is provided AS-IS.
//  *
//  ******************************************************************************
//  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifndef UNITY_TEST
#include "System/pin_definitions.h"
#include "System/system_conf.h"
#include <Console_Peripherals/console_peripherals_conf.h>
#include <Utils/misc_utils.h>
#include "Game_Engine/game_engine.h"
//#include "Game_Engine/Games/snake_game.h"
#include "Game_Engine/Games/Single_Player/snake_game.h"
#endif
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
