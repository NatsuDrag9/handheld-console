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

// Melody playback variables
static const Note* current_melody = NULL;
static uint8_t current_note_index = 0;
static uint8_t melody_length = 0;
static bool playing_melody = false;

// Static functions
static void generate_audio_samples(uint32_t current_time, uint32_t* last_sample_time, uint8_t* sample_index);
static void handle_tone_completion(uint32_t current_time);

// Sample sine wave data for audio generation
const uint16_t sine_wave[SAMPLE_COUNT] = {
    2048, 2447, 2831, 3185, 3495, 3750, 3939, 4056,
    4095, 4056, 3939, 3750, 3495, 3185, 2831, 2447,
    2048, 1648, 1264, 910, 600, 345, 156, 39,
    0, 39, 156, 345, 600, 910, 1264, 1648
};

void audio_init(void) {
    // Initialize to no sound
    audio_playing = false;

    // Initialize audio driver
    audio_driver_init();

    // Set default volume
    audio_driver_set_volume(10); // 80% volume
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

    // Set DAC to midpoint (silence)
    audio_driver_write_dac(2048);
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
    }
}

static void generate_audio_samples(uint32_t current_time, uint32_t* last_sample_time, uint8_t* sample_index) {
    // Skip if no frequency (silence)
    if (current_frequency == 0) {
        return;
    }

    // Calculate sample timing
    uint32_t cycle_time_us = 1000000 / current_frequency;
    uint32_t sample_interval_us = cycle_time_us / SAMPLE_COUNT;
    uint32_t sample_interval_ms = sample_interval_us / 1000;

    // Check if it's time for the next sample
    if (sample_interval_ms == 0) {
        sample_interval_ms = 1; // Ensure minimum interval of 1ms
    }

    if (current_time - *last_sample_time >= sample_interval_ms) {
        // Output the next sample
        audio_driver_write_dac(sine_wave[*sample_index]);

        // Move to next sample
        *sample_index = (*sample_index + 1) % SAMPLE_COUNT;
        *last_sample_time = current_time;
    }
}

static void handle_tone_completion(uint32_t current_time) {
    if (playing_melody) {
        // Move to next note in melody
        current_note_index++;

        if (current_note_index < melody_length) {
            // Play next note
            audio_start_time = current_time;
            current_frequency = current_melody[current_note_index].frequency;
            audio_duration = current_melody[current_note_index].duration;
        } else {
            // Melody finished
            audio_stop();
        }
    } else {
        // Single tone finished
        audio_stop();
    }
}


void audio_update(void) {
    static uint32_t last_sample_time = 0;
    static uint8_t sample_index = 0;

    // Only process if playing a sound
    if (!audio_playing) {
        return;
    }

    uint32_t current_time = get_current_ms();

    // Handle sample generation for current tone
    generate_audio_samples(current_time, &last_sample_time, &sample_index);

    // Check if current tone has finished
    if (current_time - audio_start_time >= audio_duration) {
        handle_tone_completion(current_time);
    }
}

bool audio_is_playing(void) {
    return audio_playing;
}
