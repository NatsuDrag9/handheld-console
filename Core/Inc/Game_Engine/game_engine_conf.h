/*
 * game_engine_conf.h
 *
 *  Created on: Jan 21, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAME_ENGINE_CONF_H_
#define INC_GAME_ENGINE_GAME_ENGINE_CONF_H_

// Frame rate
#define FRAME_RATE_30FPS   33  // ~30 FPS
#define FRAME_RATE_60FPS   16  // ~60 FPS
#define FRAME_RATE         FRAME_RATE_60FPS  // Current frame time setting
#define GAME_OVER_MESSAGE_TIME 10000 // 10 sec
#define RETURN_MESSAGE_START_TIME 5000   // 5 sec - when to start countdown

// Button timing configuration
#define BUTTON_RESTART_MAX_DURATION     1200  // Short press ≤ 1.2 seconds for restart
#define BUTTON_MENU_MIN_DURATION        2000  // Minimum hold time (2 seconds) for main menu
#define BUTTON_MENU_MAX_DURATION        5000  // Maximum hold time (5 seconds) for main menu

// Audio
#define SAMPLE_COUNT 32
extern const uint16_t SINE_WAVE_TABLE[SAMPLE_COUNT]; // Pre-calculated sine wave samples (normalized to DAC range 0-4095)

#endif /* INC_GAME_ENGINE_GAME_ENGINE_CONF_H_ */
