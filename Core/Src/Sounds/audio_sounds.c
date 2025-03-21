/*
 * audio_sounds.c
 *
 *  Created on: Mar 16, 2025
 *      Author: rohitimandi
 */


#include "Sounds/audio_sounds.h"

// Game start melody - ascending three-note jingle
const Note game_start_melody[] = {
    {NOTE_C5, 100}, {NOTE_E5, 100}, {NOTE_G5, 200}
};
const uint8_t game_start_melody_length = sizeof(game_start_melody) / sizeof(Note);

// Game over melody - descending minor scale pattern
const Note game_over_melody[] = {
    {NOTE_A5, 200}, {NOTE_F5, 200}, {NOTE_D5, 300}, {NOTE_A4, 500}
};
const uint8_t game_over_melody_length = sizeof(game_over_melody) / sizeof(Note);

// Point scored sound - quick ascending interval
const Note point_scored_sound[] = {
    {NOTE_G4, 50}, {NOTE_C5, 100}
};
const uint8_t point_scored_sound_length = sizeof(point_scored_sound) / sizeof(Note);

// Collision sound - low thump
const Note collision_sound[] = {
    {NOTE_C4, 100}
};
const uint8_t collision_sound_length = sizeof(collision_sound) / sizeof(Note);

// Menu select sound - single mid-range beep
const Note menu_select_sound[] = {
    {NOTE_E5, 80}
};
const uint8_t menu_select_sound_length = sizeof(menu_select_sound) / sizeof(Note);

// Power up sound - quick ascending arpeggio
const Note power_up_sound[] = {
    {NOTE_C5, 80}, {NOTE_E5, 80}, {NOTE_G5, 150}
};
const uint8_t power_up_sound_length = sizeof(power_up_sound) / sizeof(Note);

// Victory melody - happy major scale pattern
const Note victory_melody[] = {
    {NOTE_C5, 100}, {NOTE_E5, 100}, {NOTE_G5, 100},
    {NOTE_C5, 200}, {NOTE_G5, 100}, {NOTE_C5, 300}
};
const uint8_t victory_melody_length = sizeof(victory_melody) / sizeof(Note);

// Level complete melody - fanfare pattern
const Note level_complete_melody[] = {
    {NOTE_G4, 100}, {NOTE_C5, 100}, {NOTE_E5, 100},
    {NOTE_G5, 200}, {NOTE_E5, 100}, {NOTE_G5, 300}
};
const uint8_t level_complete_melody_length = sizeof(level_complete_melody) / sizeof(Note);
