/*
 * audio_sounds.h
 *
 *  Created on: Mar 16, 2025
 *      Author: rohitimandi
 */

#ifndef INC_SOUNDS_AUDIO_SOUNDS_H_
#define INC_SOUNDS_AUDIO_SOUNDS_H_

#include "Console_Peripherals/types.h"
#include "Utils/misc_utils.h"

// Note frequency definitions (based on A4 = 440 Hz)
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988

// Melodies and sound effects
extern const Note game_start_melody[];
extern const uint8_t game_start_melody_length;

extern const Note game_over_melody[];
extern const uint8_t game_over_melody_length;

extern const Note point_scored_sound[];
extern const uint8_t point_scored_sound_length;

extern const Note collision_sound[];
extern const uint8_t collision_sound_length;

extern const Note menu_select_sound[];
extern const uint8_t menu_select_sound_length;

extern const Note power_up_sound[];
extern const uint8_t power_up_sound_length;

// Special melodies
extern const Note victory_melody[];
extern const uint8_t victory_melody_length;

extern const Note level_complete_melody[];
extern const uint8_t level_complete_melody_length;

#endif /* INC_SOUNDS_AUDIO_SOUNDS_H_ */
