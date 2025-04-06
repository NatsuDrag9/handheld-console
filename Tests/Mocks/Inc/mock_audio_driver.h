#ifndef MOCK_AUDIO_DRIVER_H_
#define MOCK_AUDIO_DRIVER_H_

// Define SAMPLE_COUNT if not defined in audio.h
// #define SAMPLE_COUNT 128

#include <stdint.h>
#include <stdbool.h>
#include "mock_utils.h"

// Variables to control mock behavior
extern uint16_t mock_dac_value;
extern uint8_t mock_volume;
extern bool mock_is_muted;
extern bool mock_driver_initialized;

// Function to reset mocks
void mock_audio_driver_reset(void);

#endif /* MOCK_AUDIO_DRIVER_H_ */