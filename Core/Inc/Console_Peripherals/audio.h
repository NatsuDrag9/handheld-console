/*
 * audio.h
 *
 *  Created on: Mar 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_CONSOLE_PERIPHERALS_AUDIO_H_
#define INC_CONSOLE_PERIPHERALS_AUDIO_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "Game_Engine/game_engine_conf.h"
#include "Console_Peripherals/Drivers/audio_driver.h"
#include "Sounds/audio_sounds.h"
#include "Console_Peripherals/types.h"

 // Sound types
typedef enum {
    SOUND_NONE = 0,
    SOUND_GAME_START,
    SOUND_GAME_OVER,
    SOUND_POINT_SCORED,
    SOUND_COLLISION,
    SOUND_MENU_SELECT,
    SOUND_POWER_UP
} SoundEffect;

void audio_init(void);
void audio_play_sound(SoundEffect effect);
void audio_stop(void);
void audio_set_volume(uint8_t volume);
void audio_mute(bool mute);
void audio_play_tone(uint16_t frequency, uint16_t duration_ms); // Play a custom frequency for a specified duration
void audio_play_melody(const Note* notes, uint8_t note_count); // Play a melody (sequence of notes)
void audio_update(void); // Update function to be called in main loop
bool audio_is_playing(void); // Check if audio is currently playing
void audio_play_sound_once(SoundEffect effect); // Plays the sound only once until the sound is restarted manually

#endif /* INC_CONSOLE_PERIPHERALS_AUDIO_H_ */
