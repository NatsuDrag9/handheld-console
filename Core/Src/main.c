/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile float angle = 0.0f;      // Made volatile to prevent optimization
volatile float result_sin = 0.0f;
volatile float result_cos = 0.0f;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */

  /* USER CODE BEGIN SysInit */
  System_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  console_peripherals_init();
  oled_show_screen(SCREEN_WELCOME);
  oled_show_screen(SCREEN_MENU);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // Compute sine and cosine using FPU
//	  float rad = angle * 3.14159f / 180.0f;
//	      result_sin = sinf(rad);
//	      result_cos = cosf(rad);
//
//	      // Complex floating point operation to ensure FPU usage
//	      float complex_calc = (sinf(rad) * cosf(rad)) / (1.0f + rad);
//
//	      // Set a breakpoint here to check FPU registers
//	      __asm("NOP");  // Add NOP to prevent optimization
//
//	      angle += 22.5f;  // Use floating point increment
//	      if(angle >= 360.0f) {
//	          angle = 0.0f;
//	      }
//
//	      DEBUG_PRINTF(1, "Angle=%.1f, Sin=%.3f, Cos=%.3f\r\n",
//	                  angle, result_sin, result_cos);

//	  	 JoystickStatus js_status = joystick_get_status();
//	  	 if(js_status.is_new) {
//	  		 DEBUG_PRINTF("JS direction: %d\n", js_status.direction);
//	  		 DEBUG_PRINTF("JS button: %d\n", js_status.button);
//	  	 }
//	  	oled_menu_handle_input(js_status);
//	  	DEBUG_PRINTF(0, "Selected menu item: %s\n", oled_get_selected_menu_item().title);

	  	if (oled_is_game_active()) {
	  	        oled_run_game();

	  	        add_delay(50);  // Control game speed
	  	    } else {
	  	        JoystickStatus js_status = joystick_get_status();
	  	        oled_menu_handle_input(js_status);
	  	    }

//	  	 if(pb1_get_state() == 1) {
//	  		 DEBUG_PRINTF("PB1 is pressed\n");
//	  		 blink_led1();
//	  	 }
//
//	  	 if(pb2_get_state() == 1) {
//	  		 DEBUG_PRINTF("PB2 is pressed\n");
//	  		 blink_led2();
//	  	 }

//	  	 game_engine_update(&snake_game_engine, js_status);
//	  	    game_engine_render(&snake_game_engine);
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

