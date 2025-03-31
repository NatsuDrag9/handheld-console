/*
 * d_pad.h
 *
 *  Created on: Mar 7, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_D_PAD_H_
#define INC_CONSOLE_PERIPHERALS_D_PAD_H_

#include "System/system_conf.h"
#include <stdint.h>
#include "Console_Peripherals/types.h"
#include "Console_Peripherals/push_button.h"

void update_d_pad_status(void); // Made public for callbacks to access
DPAD_STATUS d_pad_get_status(void);
uint8_t d_pad_direction_changed(void);

#endif /* INC_CONSOLE_PERIPHERALS_D_PAD_H_ */
