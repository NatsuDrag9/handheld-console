/*
 * usb_conf.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_PERIPHERALS_USB_CONF_H_
#define INC_SYSTEM_PERIPHERALS_USB_CONF_H_

#include "main.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

void MX_USB_OTG_FS_PCD_Init(void);


#endif /* INC_SYSTEM_PERIPHERALS_USB_CONF_H_ */
