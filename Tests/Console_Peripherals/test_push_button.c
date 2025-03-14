#include <stdint.h>
#include "../Unity/unity.h"
#include "../Unity/unity_fixture.h"
#include "Console_Peripherals/push_button.h"
#include "Console_Peripherals/Drivers/push_button_driver.h"
#include "../Mocks/Inc/mock_push_button_driver.h"

// Helper function for edge detection
static void detect_button1_press(uint8_t button_state, uint32_t initial_time, uint32_t debounced_time) {
    // Set initial state
    mock_pb1_state = button_state;

    // First check - Button change detected but not debounced
    mock_tick_count = initial_time;
    TEST_ASSERT_EQUAL(0, pb1_get_state());

    // Second check - After debounce time
    mock_tick_count = debounced_time;
    TEST_ASSERT_EQUAL(button_state, pb1_get_state());

    // Third check - Edge detected, should be 0
    TEST_ASSERT_EQUAL(0, pb1_get_state());
}

static void detect_button2_press(uint8_t button_state, uint32_t initial_time, uint32_t debounced_time) {
    // Set initial state
    mock_pb2_state = button_state;

    // First check - Button change detected but not debounced
    mock_tick_count = initial_time;
    TEST_ASSERT_EQUAL(0, pb2_get_state());

    // Second check - After debounce time
    mock_tick_count = debounced_time;
    TEST_ASSERT_EQUAL(button_state, pb2_get_state());

    // Third check - Edge detected, should be 0
    TEST_ASSERT_EQUAL(0, pb2_get_state());
}

static void set_dpad_left_state(uint8_t button_state) {
    mock_dpad_left_state = button_state;
}

static void set_dpad_right_state(uint8_t button_state) {
    mock_dpad_right_state = button_state;
}

static void set_dpad_up_state(uint8_t button_state) {
    mock_dpad_up_state = button_state;
}

static void set_dpad_down_state(uint8_t button_state) {
    mock_dpad_down_state = button_state;
}

TEST_GROUP(PushButton);

TEST_SETUP(PushButton) {
    mock_pb1_state = 0;
    mock_pb2_state = 0;
    mock_dpad_left_state = 0;
    mock_dpad_right_state = 0;
    mock_dpad_up_state = 0;
    mock_dpad_down_state = 0;
    mock_tick_count = 0;
    pb_init();
}

TEST_TEAR_DOWN(PushButton) {
    // No clean up
}

TEST(PushButton, InitialState) {
    TEST_ASSERT_EQUAL(0, pb1_get_state());
    TEST_ASSERT_EQUAL(0, pb2_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_left_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_right_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_up_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_down_get_state());
}

TEST(PushButton, SinglePress) {
    detect_button1_press(1, 150, 300);
}

TEST(PushButton, ButtonBounce) {
    // Initial press with bounce
    mock_pb1_state = 1;
    mock_tick_count = 10;
    TEST_ASSERT_EQUAL(0, pb1_get_state());

    // Bounce
    mock_pb1_state = 0;
    mock_tick_count = 20;
    TEST_ASSERT_EQUAL(0, pb1_get_state());

    // Stable press
    detect_button1_press(1, 150, 300);
}

TEST(PushButton, MultiplePress) {
    // First press
    detect_button1_press(1, 150, 300);

    // Release and wait for debounce
    detect_button1_press(0, 450, 600);

    // Second press
    detect_button2_press(1, 750, 900);
}

TEST(PushButton, TwoButtonsIndependent) {
    // Press both buttons simultaneously
    mock_pb1_state = 1;
    mock_pb2_state = 1;

    // Initial check for both
    mock_tick_count = 150;
    TEST_ASSERT_EQUAL(0, pb1_get_state());
    TEST_ASSERT_EQUAL(0, pb2_get_state());

    // After debounce time, both should trigger
    mock_tick_count = 300;
    TEST_ASSERT_EQUAL(1, pb1_get_state());
    TEST_ASSERT_EQUAL(1, pb2_get_state());

    // Edge detection should reset both
    TEST_ASSERT_EQUAL(0, pb1_get_state());
    TEST_ASSERT_EQUAL(0, pb2_get_state());

    // Release button 1 only
    mock_pb1_state = 0;

    // Process release for button 1
    mock_tick_count = 450;
    TEST_ASSERT_EQUAL(0, pb1_get_state());
    mock_tick_count = 600;
    TEST_ASSERT_EQUAL(0, pb1_get_state());  // Still 0 since it's a release

    // Button 2 should remain unchanged
    TEST_ASSERT_EQUAL(0, pb2_get_state());  // Already triggered and reset

    // Press button 1 again
    detect_button1_press(1, 750, 900);
    TEST_ASSERT_EQUAL(0, pb2_get_state());  // Button 2 should still remain unchanged
}

TEST(PushButton, QuickPressIgnored) {
    mock_pb1_state = 1;
    mock_tick_count = 10;
    TEST_ASSERT_EQUAL(0, pb1_get_state());

    mock_pb1_state = 0;
    mock_tick_count = 20;
    TEST_ASSERT_EQUAL(0, pb1_get_state());
}

TEST(PushButton, LongPress) {
    // Initial press detection
    detect_button1_press(1, 150, 300);

    // Long press should not trigger again
    mock_tick_count = 1000;
    TEST_ASSERT_EQUAL(0, pb1_get_state());
}

// New tests for D-pad functionality
TEST(PushButton, DPadLeftState) {
    // Test low state
    set_dpad_left_state(0);
    TEST_ASSERT_EQUAL(0, dpad_pin_left_get_state());

    // Test high state
    set_dpad_left_state(1);
    TEST_ASSERT_EQUAL(1, dpad_pin_left_get_state());

    // Test toggling back to low
    set_dpad_left_state(0);
    TEST_ASSERT_EQUAL(0, dpad_pin_left_get_state());
}

TEST(PushButton, DPadRightState) {
    // Test low state
    set_dpad_right_state(0);
    TEST_ASSERT_EQUAL(0, dpad_pin_right_get_state());

    // Test high state
    set_dpad_right_state(1);
    TEST_ASSERT_EQUAL(1, dpad_pin_right_get_state());

    // Test toggling back to low
    set_dpad_right_state(0);
    TEST_ASSERT_EQUAL(0, dpad_pin_right_get_state());
}

TEST(PushButton, DPadUpState) {
    // Test low state
    set_dpad_up_state(0);
    TEST_ASSERT_EQUAL(0, dpad_pin_up_get_state());

    // Test high state
    set_dpad_up_state(1);
    TEST_ASSERT_EQUAL(1, dpad_pin_up_get_state());

    // Test toggling back to low
    set_dpad_up_state(0);
    TEST_ASSERT_EQUAL(0, dpad_pin_up_get_state());
}

TEST(PushButton, DPadDownState) {
    // Test low state
    set_dpad_down_state(0);
    TEST_ASSERT_EQUAL(0, dpad_pin_down_get_state());

    // Test high state
    set_dpad_down_state(1);
    TEST_ASSERT_EQUAL(1, dpad_pin_down_get_state());

    // Test toggling back to low
    set_dpad_down_state(0);
    TEST_ASSERT_EQUAL(0, dpad_pin_down_get_state());
}

TEST(PushButton, DPadIndependence) {
    // Test all directions at once
    set_dpad_left_state(1);
    set_dpad_right_state(0);
    set_dpad_up_state(1);
    set_dpad_down_state(0);

    // Verify each direction reports correct state independently
    TEST_ASSERT_EQUAL(1, dpad_pin_left_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_right_get_state());
    TEST_ASSERT_EQUAL(1, dpad_pin_up_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_down_get_state());

    // Change states
    set_dpad_left_state(0);
    set_dpad_right_state(1);
    set_dpad_up_state(0);
    set_dpad_down_state(1);

    // Verify states changed correctly
    TEST_ASSERT_EQUAL(0, dpad_pin_left_get_state());
    TEST_ASSERT_EQUAL(1, dpad_pin_right_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_up_get_state());
    TEST_ASSERT_EQUAL(1, dpad_pin_down_get_state());
}

TEST(PushButton, DPadAndButtonsIndependence) {
    // Set various states for all inputs
    mock_pb1_state = 1;
    mock_pb2_state = 0;
    set_dpad_left_state(1);
    set_dpad_right_state(0);
    set_dpad_up_state(1);
    set_dpad_down_state(0);

    // Check D-pad direct readings
    TEST_ASSERT_EQUAL(1, dpad_pin_left_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_right_get_state());
    TEST_ASSERT_EQUAL(1, dpad_pin_up_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_down_get_state());

    // Check debounced button readings (initially 0 due to debounce)
    mock_tick_count = 50;
    TEST_ASSERT_EQUAL(0, pb1_get_state());
    TEST_ASSERT_EQUAL(0, pb2_get_state());

    // After debounce time
    mock_tick_count = 200;
    TEST_ASSERT_EQUAL(1, pb1_get_state());
    TEST_ASSERT_EQUAL(0, pb2_get_state());

    // Edge detection should reset button 1
    TEST_ASSERT_EQUAL(0, pb1_get_state());

    // D-pad states should remain unchanged
    TEST_ASSERT_EQUAL(1, dpad_pin_left_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_right_get_state());
    TEST_ASSERT_EQUAL(1, dpad_pin_up_get_state());
    TEST_ASSERT_EQUAL(0, dpad_pin_down_get_state());
}

TEST_GROUP_RUNNER(PushButton) {
    RUN_TEST_CASE(PushButton, InitialState);
    RUN_TEST_CASE(PushButton, SinglePress);
    RUN_TEST_CASE(PushButton, ButtonBounce);
    RUN_TEST_CASE(PushButton, MultiplePress);
    RUN_TEST_CASE(PushButton, TwoButtonsIndependent);
    RUN_TEST_CASE(PushButton, QuickPressIgnored);
    RUN_TEST_CASE(PushButton, LongPress);
    RUN_TEST_CASE(PushButton, DPadLeftState);
    RUN_TEST_CASE(PushButton, DPadRightState);
    RUN_TEST_CASE(PushButton, DPadUpState);
    RUN_TEST_CASE(PushButton, DPadDownState);
    RUN_TEST_CASE(PushButton, DPadIndependence);
    RUN_TEST_CASE(PushButton, DPadAndButtonsIndependence);
}