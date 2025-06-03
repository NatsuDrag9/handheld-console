/*
 * game_engine_conf.h
 *
 *  Created on: Jan 21, 2025
 *      Author: rohitimandi
 */

#ifndef INC_GAME_ENGINE_GAME_ENGINE_CONF_H_
#define INC_GAME_ENGINE_GAME_ENGINE_CONF_H_

// Frame rate options
#define FRAME_RATE_120FPS  8   // ~120 FPS (8.33ms between frames)
#define FRAME_RATE_100FPS  10  // ~100 FPS (10ms between frames)
#define FRAME_RATE_80FPS   12  // ~80 FPS (12.5ms between frames)
#define FRAME_RATE_60FPS   16  // ~60 FPS (16.67ms between frames)
#define FRAME_RATE_30FPS   33  // ~30 FPS (33.33ms between frames)
#define FRAME_RATE_20FPS   50  // ~20 FPS (50ms between frames)
#define FRAME_RATE_15FPS   66  // ~15 FPS (66.67ms between frames)
#define FRAME_RATE_10FPS   100 // ~10 FPS (100ms between frames)
#define FRAME_RATE_5FPS    200 // ~5 FPS (200ms between frames)


#if defined(DISPLAY_MODULE_LCD)
	#define FRAME_RATE         FRAME_RATE_60FPS  // Slower for LCD
#else
	#define FRAME_RATE         FRAME_RATE_60FPS  // Current frame time setting
#endif

#define GAME_OVER_MESSAGE_TIME 10000 // 10 sec
#define RETURN_MESSAGE_START_TIME 5000   // 5 sec - when to start countdown

// Button timing configuration
#define BUTTON_RESTART_MAX_DURATION     1200  // Short press ≤ 1.2 seconds for restart
#define BUTTON_MENU_MIN_DURATION        2000  // Minimum hold time (2 seconds) for main menu
#define BUTTON_MENU_MAX_DURATION        5000  // Maximum hold time (5 seconds) for main menu

// Audio
#define SAMPLE_COUNT 128
extern const uint16_t SINE_WAVE_TABLE[SAMPLE_COUNT]; // Pre-calculated sine wave samples (normalized to DAC range 0-4095)

// Default lives in a game
#define DEFAULT_LIVES 1

#endif /* INC_GAME_ENGINE_GAME_ENGINE_CONF_H_ */
