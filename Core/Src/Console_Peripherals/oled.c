/*
 * oled.c
 *
 *  Created on: Jan 5, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/oled.h"

/* Initializes SPI and the OLED */
void oled_init(){
	ssd1306_Init();
	add_delay(10);
}

/* Changes OLED background to white */
void oled_fillWhite(){
	  ssd1306_Fill(White);
	  ssd1306_UpdateScreen();
	  add_delay(100);
}

/* Displays a string on the screen */
void oled_display_string(char* str, FontDef Font, SSD1306_COLOR color) {
	  ssd1306_SetCursor(20, 20);
	  ssd1306_WriteString(str, Font, color);
	  ssd1306_UpdateScreen();
	  add_delay(500);
}
