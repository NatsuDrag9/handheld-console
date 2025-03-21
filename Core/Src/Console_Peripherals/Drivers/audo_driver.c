/*
 * audio_driver.c
 *
 *  Created on: Mar 16, 2025
 *      Author: rohitimandi
 */

#include "Console_Peripherals/Drivers/audio_driver.h"


static uint8_t current_volume = 100;  // Default to 100% volume
static bool is_muted = false;

void audio_driver_init() {
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048); // Set to mid-range
}

void audio_driver_write_dac(uint16_t value) {
    if (is_muted) {
        // If muted, output zero regardless of input value
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
    } else {
        // Scale the input value by the current volume
        uint32_t scaled_value = (uint32_t)value * current_volume / 100;

        // Ensure the value is within valid DAC range (12-bit)
        if (scaled_value > 4095) {
            scaled_value = 4095;
        }

        // Write the scaled value to the DAC
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint16_t)scaled_value);
    }
}

void audio_driver_set_volume(uint8_t volume) {
    // Clamp volume to 0-100 range
    if (volume > 100) {
        volume = 100;
    }

    // Store the new volume level
    current_volume = volume;
}

void audio_driver_mute(bool mute) {
    is_muted = mute;

    // If muting, immediately set DAC to zero
    if (is_muted) {
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
    }
}
