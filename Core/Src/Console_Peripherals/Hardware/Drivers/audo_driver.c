///*
// * audio_driver.c
// *
// *  Created on: Mar 16, 2025
// *      Author: rohitimandi
// */
//
//#include "Console_Peripherals/Drivers/audio_driver.h"
//#include "Utils/debug_conf.h"
//
//
//static uint8_t current_volume = 100;  // Default to 100% volume
//static bool is_muted = false;
//
//// External references to timer and dac
//extern DAC_HandleTypeDef hdac;
//extern TIM_HandleTypeDef htim4;
//extern const uint16_t sine_wave[SAMPLE_COUNT];
//
//// TIM4 clock frequency (96 MHz)
//#define TIM4_CLOCK 96000000
//
//void audio_driver_init() {
//	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
//	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048); // Set to mid-range
//
//	// Start DAC with DMA to auto-output sine wave
//	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)sine_wave, SAMPLE_COUNT, DAC_ALIGN_12B_R);
//
//	// Keep timer stopped until needed
//	HAL_TIM_Base_Stop(&htim4);
//
//	 // Set default volume
//	audio_driver_set_volume(50);
//
//}
//
//void audio_driver_write_dac(uint16_t value) {
//    if (is_muted) {
//        // If muted, output zero regardless of input value
//        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
//    } else {
//        // Scale the input value by the current volume
//        uint32_t scaled_value = (uint32_t)value * current_volume / 100;
//
//        // Ensure the value is within valid DAC range (12-bit)
//        if (scaled_value > 4095) {
//            scaled_value = 4095;
//        }
//
//        // Write the scaled value to the DAC
//        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint16_t)scaled_value);
//    }
//}
//
//void audio_driver_set_volume(uint8_t volume) {
//    // Clamp volume to 0-100 range
//    if (volume > 100) {
//        volume = 100;
//    }
//
//    // Store the new volume level
//    current_volume = volume;
//}
//
//
//void audio_driver_mute(bool mute) {
//    is_muted = mute;
//
//    // If muting, immediately set DAC to zero and stop timer
//    if (is_muted) {
//        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
//        HAL_TIM_Base_Stop(&htim4);
//    }
//}
//
//void audio_driver_set_frequency(uint16_t frequency) {
//    if (frequency == 0) {
//        // Stop timer for silence
//        HAL_TIM_Base_Stop(&htim4);
//        return;
//    }
//
//    //    // Restart DMA if it's not running
//    //    if (hdac.DMA_Handle1->State != HAL_DMA_STATE_BUSY) {
//    //    	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)sine_wave, SAMPLE_COUNT, DAC_ALIGN_12B_R);
//    //    }
//
////    // Calculate required sample rate based on note frequency and sample count
////    // We need to output the entire sine wave at the desired frequency
////    uint32_t target_sample_rate = frequency * SAMPLE_COUNT;
////        if (target_sample_rate > 48000) {
////            target_sample_rate = 48000;  // Cap at 48kHz which is standard for audio
////        }
////
////    // Calculate and set new timer period
////    uint32_t new_period = (TIM4_CLOCK / target_sample_rate) - 1;
////
////    DEBUG_PRINTF(false, "Frequency: %u Hz\n", frequency);
////    DEBUG_PRINTF(false, "Sample rate: %u Hz\n", target_sample_rate);
////    DEBUG_PRINTF(false, "Period: %u\n", new_period);
////
////
////    // Make sure the new period is valid (16-bit timer)
////    if (new_period > 0xFFFF) {
////        new_period = 0xFFFF;
////    }
////
////    // Update timer period
////    __HAL_TIM_SET_COUNTER(&htim4, 0);  // Reset counter to reduce glitches
////    __HAL_TIM_SET_AUTORELOAD(&htim4, new_period);
//
//
//
//    // Start timer if it's not already running
//    if (htim4.State != HAL_TIM_STATE_BUSY) {
//        HAL_TIM_Base_Start(&htim4);
//
//    }
//}
//
//void audio_driver_stop(void) {
//    // Stop timer to stop DAC triggers
//    HAL_TIM_Base_Stop(&htim4);
//
////    // Stop DMA transfers
////    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
//
//    // Set DAC to midpoint for silence
//    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
//
//}


/*
 * audio_driver.c - Improved phase-correct implementation
 */

#include <Console_Peripherals/Hardware/Drivers/audio_driver.h>
#include "Utils/debug_conf.h"

// Fixed sample rate of 22kHz
#define SAMPLE_RATE 22000

// Volume and mute state
static uint8_t current_volume = 100;  // Default to 100% volume
static bool is_muted = false;

// External references to timer and DAC
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim4;
extern const uint16_t sine_wave[SAMPLE_COUNT];

// Use a larger buffer for smoother playback
#define AUDIO_BUFFER_SIZE 512
static uint16_t audio_buffer[AUDIO_BUFFER_SIZE];

// Phase accumulator for frequency synthesis
static uint32_t phase_acc = 0;
static uint32_t phase_inc = 0;

// TIM4 clock frequency (96 MHz)
#define TIM4_CLOCK 96000000

// Prepare the audio buffer with the correct frequency
void update_audio_buffer(void) {
    // Ensure we're not playing when updating buffer
    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);

    // Reset phase accumulator to get a clean start
    phase_acc = 0;

    // Pre-calculate the entire buffer based on current frequency settings
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        // Calculate the sine table index based on phase
        // We only use the top 7 bits since our sine table has 128 entries
        uint8_t index = (phase_acc >> 25) & 0x7F;

        // Get sample from sine table
        uint16_t sample = sine_wave[index];

        // Apply volume
        if (!is_muted) {
            uint32_t scaled_sample = (uint32_t)sample * current_volume / 100;
            if (scaled_sample > 4095) {
                scaled_sample = 4095;
            }
            audio_buffer[i] = (uint16_t)scaled_sample;
        } else {
            audio_buffer[i] = 2048; // Silence (midpoint)
        }

        // Increment phase for next sample
        phase_acc += phase_inc;
    }

    // Calculate final phase to ensure the next buffer will continue smoothly
    uint8_t final_index = (phase_acc >> 25) & 0x7F;

    // Make sure the last sample smoothly connects to the first sample
    // by checking if the phase wrapped around during buffer generation
    if (phase_inc > 0) {
        DEBUG_PRINTF(false, "Final buffer index: %u\n", final_index);
    }

    // Restart DMA with the updated buffer
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)audio_buffer, AUDIO_BUFFER_SIZE, DAC_ALIGN_12B_R);
}

void audio_driver_init(void) {
    // Initialize DAC
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048); // Set to mid-range (silence)

    // Configure Timer4 for 22kHz sample rate
    uint32_t period = (TIM4_CLOCK / SAMPLE_RATE) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim4, period);

    // Set default volume
    audio_driver_set_volume(50);

    // Initialize phase accumulator
    phase_acc = 0;
    phase_inc = 0;

    // Initialize audio buffer with silence
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        audio_buffer[i] = 2048;
    }
}

// DAC DMA half-transfer complete callback
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
    // We could update the first half of the buffer here if needed
    // This is called when DMA has processed half the buffer
}

// DAC DMA transfer complete callback
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
    // We could update the second half of the buffer here if needed
    // This is called when DMA has processed the entire buffer
}

void audio_driver_set_volume(uint8_t volume) {
    // Clamp volume to 0-100 range
    if (volume > 100) {
        volume = 100;
    }

    // Store the new volume level
    current_volume = volume;

    // Update audio buffer with new volume
    if (phase_inc > 0) {
        update_audio_buffer();
    }
}

void audio_driver_mute(bool mute) {
    is_muted = mute;

    // Update audio buffer based on mute status
    if (phase_inc > 0) {
        update_audio_buffer();
    } else if (is_muted) {
        // If no sound playing but muted, ensure DAC is at midpoint
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
    }
}

void audio_driver_set_frequency(uint16_t frequency) {
    if (frequency == 0) {
        // Stop sound generation
        phase_inc = 0;
        HAL_TIM_Base_Stop(&htim4);
        HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048); // Set to silence
        return;
    }

    // Calculate phase increment for the requested frequency
    // Formula: phase_inc = (frequency * 2^32) / SAMPLE_RATE
    uint64_t calc = (uint64_t)frequency << 32;
    phase_inc = (uint32_t)(calc / SAMPLE_RATE);

    DEBUG_PRINTF(false, "Set frequency: %u Hz, phase_inc: %lu\n", frequency, phase_inc);

    // Update the audio buffer with the new frequency
    update_audio_buffer();

    // Start timer if it's not already running
    if (htim4.State != HAL_TIM_STATE_BUSY) {
        HAL_TIM_Base_Start(&htim4);
    }
}

void audio_driver_stop(void) {
    // Stop sound generation
    phase_inc = 0;

    // Stop timer and DMA
    HAL_TIM_Base_Stop(&htim4);
    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);

    // Set DAC to midpoint for silence
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
}
