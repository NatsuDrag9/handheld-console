/*
 * mock_audio_driver.c
 *
 *  Created on: Mar 22, 2025
 */

#ifdef UNITY_TEST
#include <stdint.h>
#include <stdbool.h>
#include "Console_Peripherals/Drivers/audio_driver.h"
#include "../Inc/mock_audio_driver.h"

uint16_t mock_dac_value = 0;
uint8_t mock_volume = 100;
bool mock_is_muted = false;
bool mock_driver_initialized = false;


void audio_driver_init(void) {
    mock_driver_initialized = true;
    mock_dac_value = 2048; // Set to mid-range as per the real implementation
}

void audio_driver_write_dac(uint16_t value) {
    if (mock_is_muted) {
        mock_dac_value = 0;
    }
    else {
        // Scale the input value by the current volume
        uint32_t scaled_value = (uint32_t)value * mock_volume / 100;

        // Ensure the value is within valid DAC range (12-bit)
        if (scaled_value > 4095) {
            scaled_value = 4095;
        }

        mock_dac_value = (uint16_t)scaled_value;
    }
}

void audio_driver_set_volume(uint8_t volume) {
    // Clamp volume to 0-100 range
    if (volume > 100) {
        volume = 100;
    }

    mock_volume = volume;
}

void audio_driver_mute(bool mute) {
    mock_is_muted = mute;

    // If muting, immediately set DAC to zero
    if (mock_is_muted) {
        mock_dac_value = 0;
    }
}

void mock_audio_driver_reset(void) {
    mock_dac_value = 0;
    mock_volume = 100;
    mock_is_muted = false;
    mock_driver_initialized = false;
    mock_time_reset();
}

#endif /* UNITY_TEST */