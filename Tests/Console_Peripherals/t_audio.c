/*
 * test_audio.c
 *
 *  Created on: Mar 22, 2025
 */

#include <stdint.h>
#include "../Unity/unity.h"
#include "../Unity/unity_fixture.h"
#include "Console_Peripherals/audio.h"
#include "Console_Peripherals/types.h"
#include "../Mocks/Inc/mock_audio_driver.h"
#include "../Mocks/Inc/mock_utils.h"
#include "Sounds/audio_sounds.h"

 // Add this in a suitable location in your test code

 // This function can inspect audio.c implementation details
void inspect_audio_module(void) {
    printf("\n==== Audio Module Inspection ====\n");

    // Inspect known audio module states
    printf("- audio_is_playing(): %d\n", audio_is_playing());

    // Try to infer the state of the static variables
    // We can't access them directly, but we can observe behavior

    // Test if audio_stop works properly
    audio_stop();
    printf("- After audio_stop(), audio_is_playing(): %d\n", audio_is_playing());

    // Test various time cases
    printf("\nTime behavior test:\n");
    audio_play_tone(440, 100); // Play 440Hz for 100ms
    printf("- Start time=%u, audio_is_playing()=%d\n", mock_time_get_ms(), audio_is_playing());

    mock_time_set_ms(50);
    audio_update();
    printf("- At t=50ms, audio_is_playing()=%d\n", audio_is_playing());

    mock_time_set_ms(101);
    audio_update();
    printf("- At t=101ms, audio_is_playing()=%d\n", audio_is_playing());

    audio_update();
    printf("- After second update at t=101ms, audio_is_playing()=%d\n", audio_is_playing());
}

TEST_GROUP(Audio);

TEST_SETUP(Audio) {
    // Reset all mock states before each test
    mock_audio_driver_reset();
    mock_time_reset();

    // Initialize the audio module
    audio_init();
    // Override the volume to 100% for consistent testing
    audio_set_volume(100);
}

TEST_TEAR_DOWN(Audio) {
    // No specific cleanup needed
}

TEST(Audio, InitialState) {
    // Verify initial state after initialization
    TEST_ASSERT_EQUAL(true, mock_driver_initialized);
    TEST_ASSERT_EQUAL(false, audio_is_playing());
    TEST_ASSERT_EQUAL(2048, mock_dac_value); // Should be midpoint for silence
}

TEST(Audio, VolumeControl) {
    // Test setting volume
    audio_set_volume(75);
    TEST_ASSERT_EQUAL(75, mock_volume);

    // Test setting volume above max (should clamp to 100)
    audio_set_volume(150);
    TEST_ASSERT_EQUAL(100, mock_volume);

    // Test setting volume to 0
    audio_set_volume(0);
    TEST_ASSERT_EQUAL(0, mock_volume);
}

TEST(Audio, MuteControl) {
    // Test muting
    audio_mute(true);
    TEST_ASSERT_EQUAL(true, mock_is_muted);
    TEST_ASSERT_EQUAL(0, mock_dac_value); // DAC value should be 0 when muted

    // Test unmuting
    audio_mute(false);
    TEST_ASSERT_EQUAL(false, mock_is_muted);
}

TEST(Audio, PlayTone) {
    // Test playing a simple tone
    audio_play_tone(440, 500); // Play 440Hz (A4) for 500ms

    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Update should maintain playing state before duration expires
    mock_time_set_ms(100);
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Advance time beyond duration
    mock_time_set_ms(600); // 600ms > 500ms duration
    audio_update();
    // Call update a second time to ensure state is completely processed
    audio_update();
    TEST_ASSERT_EQUAL(false, audio_is_playing());
}

TEST(Audio, StopSound) {
    // Start playing a tone
    audio_play_tone(440, 1000);
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Reset the timer to ensure we have a clean state
    mock_time_reset();

    // Stop the sound
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // Since we're using a mock, we need to directly check that the DAC value is set to midpoint
    // However, we should update to ensure any pending changes are processed
    audio_update();
    TEST_ASSERT_EQUAL(2048, mock_dac_value); // Should be midpoint for silence
}

TEST(Audio, PlayMelody) {
    // Test playing a melody (game start melody)
    audio_play_sound(SOUND_GAME_START);

    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // First note should be playing for 100ms
    mock_time_set_ms(50); // Middle of first note
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Advance time to end of first note
    mock_time_set_ms(110);
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Advance time to middle of second note
    mock_time_set_ms(160);
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Advance time to end of second note
    mock_time_set_ms(220);
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Advance time to middle of third note
    mock_time_set_ms(320);
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Advance time to end of melody
    mock_time_set_ms(450);
    audio_update();
    // Call update a second time to ensure state is completely processed
    audio_update();
    TEST_ASSERT_EQUAL(false, audio_is_playing());
}

TEST(Audio, SampleGeneration) {
    // Test that sample updates change DAC value
    audio_play_tone(1000, 1000); // Play 1kHz tone for 1 second

    // Record initial DAC value
    uint16_t initial_dac = mock_dac_value;

    // Advance time a small amount and update
    mock_time_set_ms(1); // 1ms later
    audio_update();

    // Since we're using a sine wave, the DAC value should change between updates
    // This test is simplified because we can't easily predict the exact DAC value
    // due to the sample rate calculation logic in generate_audio_samples
    TEST_ASSERT_NOT_EQUAL(initial_dac, mock_dac_value);
}

TEST(Audio, PlaySoundEffects) {
    // Test each sound effect - focus on start/stop behavior, not timing

    // SOUND_GAME_START
    audio_init();
    audio_set_volume(100);
    audio_play_sound(SOUND_GAME_START);
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Test that we can manually stop it
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // SOUND_GAME_OVER
    audio_init();
    audio_set_volume(100);
    audio_play_sound(SOUND_GAME_OVER);
    TEST_ASSERT_EQUAL(true, audio_is_playing());
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // SOUND_POINT_SCORED
    audio_init();
    audio_set_volume(100);
    audio_play_sound(SOUND_POINT_SCORED);
    TEST_ASSERT_EQUAL(true, audio_is_playing());
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // SOUND_COLLISION
    audio_init();
    audio_set_volume(100);
    audio_play_sound(SOUND_COLLISION);
    TEST_ASSERT_EQUAL(true, audio_is_playing());
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // SOUND_MENU_SELECT
    audio_init();
    audio_set_volume(100);
    audio_play_sound(SOUND_MENU_SELECT);
    TEST_ASSERT_EQUAL(true, audio_is_playing());
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // SOUND_POWER_UP
    audio_init();
    audio_set_volume(100);
    audio_play_sound(SOUND_POWER_UP);
    TEST_ASSERT_EQUAL(true, audio_is_playing());
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // SOUND_NONE
    audio_init();
    audio_set_volume(100);
    audio_play_sound(SOUND_NONE);
    TEST_ASSERT_EQUAL(false, audio_is_playing());
}

TEST(Audio, InterruptMelody) {
    // Start playing a melody
    audio_play_sound(SOUND_GAME_START);
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Start playing a different sound to interrupt it
    audio_play_sound(SOUND_POINT_SCORED);
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Verify different timing behavior for the new sound
    // Play through first note of point scored sound (50ms)
    mock_time_set_ms(40);
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Move to second note of point scored sound
    mock_time_set_ms(60);
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Complete the second note
    mock_time_set_ms(170);
    audio_update();
    // Call update a second time to ensure state is completely processed
    audio_update();
    TEST_ASSERT_EQUAL(false, audio_is_playing());
}

TEST(Audio, NoSoundWhenFrequencyZero) {
    // Create a custom zero-frequency note
    Note zero_freq_note = { 0, 100 }; // 0Hz for 100ms

    // Play the zero-frequency note
    audio_play_melody(&zero_freq_note, 1);

    // Audio should be playing (because the module is in playing state)
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // But the DAC value should remain at the midpoint (no sample generation for f=0)
    uint16_t initial_dac = mock_dac_value;
    mock_time_set_ms(50);
    audio_update();
    // For zero frequency, DAC value should not change since no waveform generation occurs
    TEST_ASSERT_EQUAL(initial_dac, mock_dac_value);
}


TEST(Audio, PlaySpecialMelodies) {
    // Test victory melody
    audio_init();
    audio_set_volume(100);
    audio_play_melody(victory_melody, victory_melody_length);
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Test that we can manually stop it
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // Test level complete melody
    audio_init();
    audio_set_volume(100);
    audio_play_melody(level_complete_melody, level_complete_melody_length);
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Test that we can manually stop it
    audio_stop();
    TEST_ASSERT_EQUAL(false, audio_is_playing());

    // Test a simple duration test with a single note
    // This tests the automatic completion functionality
    audio_init();
    audio_set_volume(100);

    Note single_note = { 440, 100 }; // 440Hz for 100ms
    audio_play_melody(&single_note, 1);
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Just before end
    mock_time_set_ms(99);
    audio_update();
    TEST_ASSERT_EQUAL(true, audio_is_playing());

    // Just after end - should stop automatically
    mock_time_set_ms(101);
    audio_update();
    TEST_ASSERT_EQUAL(false, audio_is_playing());
}

TEST(Audio, DiagnosticInspection) {
    printf("\n\n==== DIAGNOSTIC: Audio Module Inspection ====\n");

    // Run general inspection
    inspect_audio_module();

    // Test specific behavior of handle_tone_completion
    printf("\nTesting melody completion handling:\n");

    // Completely reset state
    mock_audio_driver_reset();
    mock_time_reset();
    audio_init();
    audio_set_volume(100);

    // Create a simple test melody with just one note
    Note test_note = { 440, 100 }; // 440Hz for 100ms

    printf("- Playing single-note melody\n");
    audio_play_melody(&test_note, 1);
    printf("- After play_melody: is_playing=%d\n", audio_is_playing());

    // Time just before completion
    mock_time_set_ms(99);
    audio_update();
    printf("- At t=99ms: is_playing=%d\n", audio_is_playing());

    // Time just after completion
    mock_time_set_ms(101);
    printf("- Setting time to t=101ms\n");
    audio_update();
    printf("- After update at t=101ms: is_playing=%d\n", audio_is_playing());

    // Multiple updates to ensure state is processed
    audio_update();
    printf("- After second update: is_playing=%d\n", audio_is_playing());
    audio_update();
    printf("- After third update: is_playing=%d\n", audio_is_playing());

    // Test state after stopping manually
    audio_stop();
    printf("- After manual stop: is_playing=%d\n", audio_is_playing());
    TEST_ASSERT_EQUAL(false, audio_is_playing());
}

TEST_GROUP_RUNNER(Audio) {
    RUN_TEST_CASE(Audio, InitialState);
    RUN_TEST_CASE(Audio, VolumeControl);
    RUN_TEST_CASE(Audio, MuteControl);
    RUN_TEST_CASE(Audio, PlayTone);
    RUN_TEST_CASE(Audio, StopSound);
    RUN_TEST_CASE(Audio, PlayMelody);
    RUN_TEST_CASE(Audio, SampleGeneration);
    RUN_TEST_CASE(Audio, PlaySoundEffects);
    RUN_TEST_CASE(Audio, InterruptMelody);
    RUN_TEST_CASE(Audio, NoSoundWhenFrequencyZero);
    RUN_TEST_CASE(Audio, PlaySpecialMelodies);
    // RUN_TEST_CASE(Audio, DiagnosticInspection);
}