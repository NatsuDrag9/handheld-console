/*
 * oled.h
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_OLED_H_
#define INC_CONSOLE_PERIPHERALS_OLED_H_

#include <stdbool.h>
#include "Console_Peripherals/Drivers/ssd1306.h"
#include "Utils/system_utils.h"

/* OLED function prototype */
void oled_init();
void oled_fillWhite();
void oled_display_string(char* str, FontDef Font, SSD1306_COLOR color);

// Add more as the game will be developed
//void oled_display();

//  ssd1306_Init();
//  HAL_Delay(20);

//  ssd1306_Fill(White);
//  ssd1306_UpdateScreen();
//  HAL_Delay(100);

//  ssd1306_SetCursor(20, 20);
//  ssd1306_WriteString("Hello", Font_7x10, White);
//  ssd1306_UpdateScreen();
//  HAL_Delay(1000);

#endif /* INC_CONSOLE_PERIPHERALS_OLED_H_ */
