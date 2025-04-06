/*
 * audio.c
 *
 *  Created on: Mar 16, 2025
 *      Author: rohitimandi
 */


#include "Console_Peripherals/audio.h"
#include <math.h>

 // Audio state variables
static bool audio_playing = false;
static uint32_t audio_start_time = 0;
static uint32_t audio_duration = 0;
static uint16_t current_frequency = 0;
static SoundEffect current_sound = SOUND_NONE;
static bool sound_playing_started = false;

// Melody playback variables
static const Note* current_melody = NULL;
static uint8_t current_note_index = 0;
static uint8_t melody_length = 0;
static bool playing_melody = false;

const uint16_t sine_wave[SAMPLE_COUNT] = {
    2048, 2148, 2248, 2347, 2447, 2546, 2645, 2744,
    2842, 2940, 3038, 3135, 3231, 3327, 3422, 3516,
    3610, 3703, 3795, 3886, 3976, 4065, 4153, 4239,
    4325, 4409, 4492, 4573, 4653, 4731, 4808, 4883,
    4956, 5028, 5098, 5166, 5232, 5296, 5358, 5418,
    5476, 5532, 5585, 5637, 5686, 5733, 5777, 5820,
    5859, 5897, 5932, 5965, 5995, 6023, 6049, 6072,
    6093, 6112, 6128, 6142, 6153, 6162, 6168, 6172,
    6173, 6172, 6168, 6162, 6153, 6142, 6128, 6112,
    6093, 6072, 6049, 6023, 5995, 5965, 5932, 5897,
    5859, 5820, 5777, 5733, 5686, 5637, 5585, 5532,
    5476, 5418, 5358, 5296, 5232, 5166, 5098, 5028,
    4956, 4883, 4808, 4731, 4653, 4573, 4492, 4409,
    4325, 4239, 4153, 4065, 3976, 3886, 3795, 3703,
    3610, 3516, 3422, 3327, 3231, 3135, 3038, 2940,
    2842, 2744, 2645, 2546, 2447, 2347, 2248, 2148
};

void audio_init(void) {
    // Initialize to no sound
    audio_playing = false;

    // Initialize audio driver
    audio_driver_init();

    // Set default volume
    audio_driver_set_volume(10);
}

void audio_play_sound(SoundEffect effect) {
    // Stop any current playback
    audio_stop();

    // Select the appropriate sound effect
    switch (effect) {
    case SOUND_GAME_START:
        audio_play_melody(game_start_melody, game_start_melody_length);
        break;

    case SOUND_GAME_OVER:
        audio_play_melody(game_over_melody, game_over_melody_length);
        break;

    case SOUND_POINT_SCORED:
        audio_play_melody(point_scored_sound, point_scored_sound_length);
        break;

    case SOUND_COLLISION:
        audio_play_melody(collision_sound, collision_sound_length);
        break;

    case SOUND_MENU_SELECT:
        audio_play_melody(menu_select_sound, menu_select_sound_length);
        break;

    case SOUND_POWER_UP:
        audio_play_melody(power_up_sound, power_up_sound_length);
        break;

    case SOUND_NONE:
    default:
        // No sound
        break;
    }
}

void audio_stop(void) {
    audio_playing = false;
    playing_melody = false;
    current_melody = NULL;
    audio_duration = 0;

    audio_driver_stop();
}

void audio_set_volume(uint8_t volume) {
    audio_driver_set_volume(volume);
}

void audio_mute(bool mute) {
    audio_driver_mute(mute);
}

void audio_play_tone(uint16_t frequency, uint16_t duration_ms) {
    // Stop any current melody
    playing_melody = false;
    current_melody = NULL;

    // Set up for tone playback
    audio_playing = true;
    audio_start_time = get_current_ms();
    audio_duration = duration_ms;
    current_frequency = frequency;

    // Set the frequency in the audio driver
    audio_driver_set_frequency(frequency);
}

void audio_play_melody(const Note* notes, uint8_t note_count) {
    if (notes && note_count > 0) {
        playing_melody = true;
        current_melody = notes;
        melody_length = note_count;
        current_note_index = 0;

        // Start playing the first note
        audio_playing = true;
        audio_start_time = get_current_ms();
        current_frequency = current_melody[0].frequency;
        audio_duration = current_melody[0].duration;

        // Set the frequency in the audio driver
        audio_driver_set_frequency(current_frequency);
    }
}

void audio_play_sound_once(SoundEffect effect) {
    // Only start a new sound if we're not already playing this sound
    if (current_sound != effect || !sound_playing_started) {
        // Stop any currently playing sound
        audio_stop();

        // Start the new sound
        audio_play_sound(effect);

        // Update our tracking variables
        current_sound = effect;
        sound_playing_started = true;
    }
}


void audio_update(void) {
    // Only process if playing a sound
    if (!audio_playing) {
        sound_playing_started = false;  // Reset our flag when sound stops
        current_sound = SOUND_NONE;     // Clear current sound
        return;
    }

    uint32_t current_time = get_current_ms();

    // Check if current tone has finished
    if (current_time - audio_start_time >= audio_duration) {
        if (playing_melody) {
            // Move to next note in melody
            current_note_index++;

            if (current_note_index < melody_length) {
                // Play next note
                audio_start_time = current_time;
                current_frequency = current_melody[current_note_index].frequency;
                audio_duration = current_melody[current_note_index].duration;

                // Update frequency in the audio driver
                audio_driver_set_frequency(current_frequency);
            }
            else {
                // Melody finished
                audio_stop();
            }
        }
        else {
            // Single tone finished
            audio_stop();
        }
    }
}

bool audio_is_playing(void) {
    return audio_playing;
}

