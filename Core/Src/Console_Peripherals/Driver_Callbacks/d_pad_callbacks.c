/*
 * d_pad_callbacks.c
 *
 *  Created on: Mar 7, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Driver_Callbacks/d_pad_callbacks.h"
#include "Console_Peripherals/d_pad.h"
#include "System/pin_definitions.h"
#include "Utils/debug_conf.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//	  DEBUG_PRINTF(false, "EXTI callback for pin: %x\n", GPIO_Pin);
    // Check if this is a D-pad button
    if(GPIO_Pin == D_PAD_Pin_Up ||
       GPIO_Pin == D_PAD_Pin_Down ||
       GPIO_Pin == D_PAD_Pin_Left ||
       GPIO_Pin == D_PAD_Pin_Right) {

        // Update D-pad status only when a button is pressed
        update_d_pad_status();
    }

    // Note: Other EXTI callbacks (if any) should be handled here or in other callback files
}
