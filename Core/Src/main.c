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
#include "main.h"

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
static bool initial_sound_played = false;

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

  /* USER CODE BEGIN SysInit */
  System_Init();
  /* USER CODE END SysInit */

  /* USER CODE BEGIN 2 */
  console_peripherals_init();
//  display_fill_white();
//  display_write_string("Hello World", Font_7x10, DISPLAY_BLACK);

//  oled_show_screen(SCREEN_WELCOME);
//  oled_show_screen(SCREEN_MENU);

  // Start with a simple tone test
//  audio_play_tone(440, 1000);  // 440 Hz (A4) for 1 second
//  audio_play_sound(SOUND_GAME_OVER);
//  audio_update();

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
//
//	  	 JoystickStatus js_status = joystick_get_status();
//	  	 if(js_status.is_new) {
//	  		 DEBUG_PRINTF(false, "JS direction: %d\n", js_status.direction);
//	  		 DEBUG_PRINTF(false, "JS button: %d\n", js_status.button);
//	  	 }
//	  	oled_menu_handle_input(js_status);
//	  	DEBUG_PRINTF(0, "Selected menu item: %s\n", oled_get_selected_menu_item().title);

//	  	if (oled_is_game_active()) {
//	  	        oled_run_game();
//
//	  	        add_delay(1);  // Control game speed
//	  	    } else {
//	  	        JoystickStatus js_status = joystick_get_status();
//	  	        oled_menu_handle_input(js_status);
//	  	    }

//	  	 if(pb1_get_state() == 1) {
//	  		 DEBUG_PRINTF(false, "PB1 is pressed\n");
//	  		 blink_led1();
//	  	 }
//
//	  	 if(pb2_get_state() == 1) {
//	  		 DEBUG_PRINTF(false, "PB2 is pressed\n");
//	  		 blink_led2();
//	  	 }

//	  	  	 if(dpad_pin_left_get_state() == 1) {
//	  	  		 DEBUG_PRINTF(false, "D-pad left is pressed\n");
//	  	  		 blink_led1();
//	  	  	 }
//
//	  	  	 if(dpad_pin_right_get_state() == 1) {
//	  	  		 DEBUG_PRINTF(false, "D-pad right is pressed\n");
//	  	  		 blink_led2();
//	  	  	 }
//
//		  	 if(dpad_pin_up_get_state() == 1) {
//		  	  		 DEBUG_PRINTF(false, "D-pad up is pressed\n");
//		  	  		 blink_led1();
//		  	  	 }
//
//		  	  	 if(dpad_pin_down_get_state() == 1) {
//		  	  		 DEBUG_PRINTF(false, "D-pad down is pressed\n");
//		  	  		 blink_led2();
//		  	  	 }


//    DPAD_STATUS d_pad_status = d_pad_get_status();
//    if (d_pad_status.is_new) {
//      DEBUG_PRINTF(false, "Dpad direction: %d\n", d_pad_status.direction);
//    }

//	  	audio_update();
//
//	    DPAD_STATUS d_pad_status = d_pad_get_status();
//	    if (d_pad_status.direction == DPAD_DIR_UP) {
//	      audio_play_sound(SOUND_GAME_START);
//
//	      DEBUG_PRINTF(false, "Dpad direction: %d\n", d_pad_status.direction);
//	    }
//	    else if (d_pad_status.direction == DPAD_DIR_RIGHT) {
//	    	audio_play_sound(SOUND_GAME_OVER);
////	    	 audio_update();
//	    	DEBUG_PRINTF(false, "Dpad direction: %d\n", d_pad_status.direction);
//	    }
//	    else if (d_pad_status.direction == DPAD_DIR_DOWN) {
//	    	audio_play_sound(SOUND_POINT_SCORED);
////	    	 audio_update();
//	    	DEBUG_PRINTF(false, "Dpad direction: %d\n", d_pad_status.direction);
//	    }
//	    else if (d_pad_status.direction == DPAD_DIR_LEFT) {
//	    	audio_play_sound(SOUND_COLLISION);
////	    	 audio_update();
//	    	DEBUG_PRINTF(false, "Dpad direction: %d\n", d_pad_status.direction);
//	    }
//	    if(pb1_get_state() == 1) {
//	    	audio_play_sound(SOUND_MENU_SELECT);
//	    	 audio_update();
//	    	DEBUG_PRINTF(false, "PB1 is pressed\n");
//	    }
//	    if(pb2_get_state() == 1) {
//	    	audio_play_sound(SOUND_POWER_UP);
////	    	 audio_update();
//	    	DEBUG_PRINTF(false, "PB2 is pressed\n");
//	    }

//	  audio_update();
//
//	    if (!initial_sound_played) {
//	        audio_play_sound_once(SOUND_MENU_SELECT);
//	        initial_sound_played = true;
//	    }



    //	  	 game_engine_update(&snake_game_engine, js_status);
    //	  	    game_engine_render(&snake_game_engine);
  }
  /* USER CODE END 3 */
}


