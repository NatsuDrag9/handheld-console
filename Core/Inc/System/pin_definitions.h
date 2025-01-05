/*
 * pin_conf.h
 *
 *  Created on: Dec 6, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_PIN_DEFINITIONS_H_
#define INC_SYSTEM_PIN_DEFINITIONS_H_

#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC

/* MCO */
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH

/* LEDs */
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB

/* ST-LINK */
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD

/* USB */
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA

/* Debug */
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* Game Peripherals */
#define JS_Button_Pin GPIO_PIN_13
#define JS_Button_GPIO_Port GPIOF
#define PB_1_Pin GPIO_PIN_15
#define PB_2_Pin GPIO_PIN_14
#define PB_GPIO_Port GPIOF
#define OLED_CS_Port         GPIOD
#define OLED_CS_Pin          GPIO_PIN_14
#define OLED_DC_Port         GPIOD
#define OLED_DC_Pin          GPIO_PIN_15
#define OLED_Reset_Port      GPIOF
#define OLED_Reset_Pin       GPIO_PIN_12


#endif /* INC_SYSTEM_PIN_DEFINITIONS_H_ */
