#include <stdint.h>
#include "../Unity/unity.h"
#include "../Unity/unity_fixture.h"
#include "Console_Peripherals/d_pad.h"
#include "Console_Peripherals/push_button.h"
#include "Console_Peripherals/Drivers/push_button_driver.h"
#include "Console_Peripherals/types.h"
#include "../Mocks/Inc/mock_push_button_driver.h"

// Helper function that sets mock d-pad states
static void set_dpad_direction(uint8_t up, uint8_t right, uint8_t down, uint8_t left) {
    mock_dpad_up_state = up;
    mock_dpad_right_state = right;
    mock_dpad_down_state = down;
    mock_dpad_left_state = left;
}

TEST_GROUP(DPad);

TEST_SETUP(DPad) {
    // Initialize mock states
    mock_dpad_up_state = 0;
    mock_dpad_right_state = 0;
    mock_dpad_down_state = 0;
    mock_dpad_left_state = 0;

    // Reset d-pad status (call update with no buttons pressed)
    update_d_pad_status();
    // Clear both status and changed flags
    d_pad_get_status();
    d_pad_direction_changed();
}

TEST_TEAR_DOWN(DPad) {
    // No clean up needed
}

TEST(DPad, InitialState) {
    // Verify initial state (no buttons pressed)
    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(0, status.direction);
    TEST_ASSERT_EQUAL(0, status.is_new);
}

TEST(DPad, UpDirection) {
    // Set up direction
    set_dpad_direction(1, 0, 0, 0);

    // Update the d-pad status
    update_d_pad_status();

    // Check status
    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_UP, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Check that is_new flag is cleared after reading
    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_UP, status.direction);
    TEST_ASSERT_EQUAL(0, status.is_new);
}

TEST(DPad, RightDirection) {
    set_dpad_direction(0, 1, 0, 0);
    update_d_pad_status();

    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_RIGHT, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Check that is_new flag is cleared after reading
    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_RIGHT, status.direction);
    TEST_ASSERT_EQUAL(0, status.is_new);
}

TEST(DPad, DownDirection) {
    set_dpad_direction(0, 0, 1, 0);
    update_d_pad_status();

    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_DOWN, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Check that is_new flag is cleared after reading
    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_DOWN, status.direction);
    TEST_ASSERT_EQUAL(0, status.is_new);
}

TEST(DPad, LeftDirection) {
    set_dpad_direction(0, 0, 0, 1);
    update_d_pad_status();

    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_LEFT, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Check that is_new flag is cleared after reading
    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_LEFT, status.direction);
    TEST_ASSERT_EQUAL(0, status.is_new);
}

TEST(DPad, DirectionPriority) {
    // Test priority when multiple buttons are pressed (up has highest priority)
    set_dpad_direction(1, 1, 1, 1);
    update_d_pad_status();

    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_UP, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Test priority: right over down and left
    set_dpad_direction(0, 1, 1, 1);
    update_d_pad_status();

    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_RIGHT, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Test priority: down over left
    set_dpad_direction(0, 0, 1, 1);
    update_d_pad_status();

    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_DOWN, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);
}

TEST(DPad, DirectionChange) {
    // Start with up direction
    set_dpad_direction(1, 0, 0, 0);
    update_d_pad_status();

    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_UP, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Change to right direction
    set_dpad_direction(0, 1, 0, 0);
    update_d_pad_status();

    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_RIGHT, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Change to no direction (released)
    set_dpad_direction(0, 0, 0, 0);
    update_d_pad_status();

    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(0, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);
}

TEST(DPad, NoChangeInDirection) {
    // Set initial direction
    set_dpad_direction(1, 0, 0, 0);
    update_d_pad_status();

    // Read to clear is_new flag
    d_pad_get_status();

    // Update with same direction
    update_d_pad_status();

    // Check that is_new remains cleared since direction didn't change
    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_UP, status.direction);
    TEST_ASSERT_EQUAL(0, status.is_new);
}

TEST(DPad, DirectionChangedFlag) {
    // Initially, no change should be detected
    TEST_ASSERT_EQUAL(0, d_pad_direction_changed());

    // Set initial direction to UP
    set_dpad_direction(1, 0, 0, 0);
    update_d_pad_status();
    TEST_ASSERT_EQUAL(1, d_pad_direction_changed());

    // Calling it again should return 0 (flag should be cleared)
    TEST_ASSERT_EQUAL(0, d_pad_direction_changed());

    // Change direction to RIGHT
    set_dpad_direction(0, 1, 0, 0);
    update_d_pad_status();
    TEST_ASSERT_EQUAL(1, d_pad_direction_changed());
}

TEST(DPad, GetStatusClearsIsNewOnly) {
    // Set initial direction
    set_dpad_direction(1, 0, 0, 0);
    update_d_pad_status();

    // First read should have is_new=1
    DPAD_STATUS status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_UP, status.direction);
    TEST_ASSERT_EQUAL(1, status.is_new);

    // Second read should have is_new=0 but unchanged direction
    status = d_pad_get_status();
    TEST_ASSERT_EQUAL(DPAD_DIR_UP, status.direction);
    TEST_ASSERT_EQUAL(0, status.is_new);

    // Direction changed flag should be independent of get_status
    TEST_ASSERT_EQUAL(1, d_pad_direction_changed());
    TEST_ASSERT_EQUAL(0, d_pad_direction_changed());
}


TEST_GROUP_RUNNER(DPad) {
    RUN_TEST_CASE(DPad, InitialState);
    RUN_TEST_CASE(DPad, UpDirection);
    RUN_TEST_CASE(DPad, RightDirection);
    RUN_TEST_CASE(DPad, DownDirection);
    RUN_TEST_CASE(DPad, LeftDirection);
    RUN_TEST_CASE(DPad, DirectionPriority);
    RUN_TEST_CASE(DPad, DirectionChange);
    RUN_TEST_CASE(DPad, NoChangeInDirection);
    RUN_TEST_CASE(DPad, DirectionChangedFlag);
    RUN_TEST_CASE(DPad, GetStatusClearsIsNewOnly);
}