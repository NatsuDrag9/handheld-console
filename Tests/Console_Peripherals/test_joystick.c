#include "../Unity/unity.h"
#include "../Unity/unity_fixture.h"
#include "Console_Peripherals/joystick.h"
#include "Console_Peripherals/Drivers/joystick_driver.h" 

TEST_GROUP(Joystick);

TEST_SETUP(Joystick) {
    // Initialize joystick to a known state before each test
    joystick_driver_set_values(2048, 2048, 0);  // Center position, no button press
    joystick_init();
}

TEST_TEAR_DOWN(Joystick) {
    // No clean up
}

TEST(Joystick, CenterPosition) {
    uint8_t direction = calculate_direction(2048, 2048);  // Center values
    TEST_ASSERT_EQUAL(JS_DIR_CENTERED, direction);
}

TEST(Joystick, LeftDirection) {
    uint8_t direction = calculate_direction(1500, 2048);  // X below X_POS_THRES_LOW, Y centered
    TEST_ASSERT_EQUAL(JS_DIR_LEFT, direction);
}

TEST(Joystick, RightDirection) {
    uint8_t direction = calculate_direction(2700, 2048);  // X above X_POS_THRES_HIGH, Y centered
    TEST_ASSERT_EQUAL(JS_DIR_RIGHT, direction);
}

TEST(Joystick, UpDirection) {
    uint8_t direction = calculate_direction(2048, 2700);  // X centered, Y above Y_POS_THRES_HIGH
    TEST_ASSERT_EQUAL(JS_DIR_UP, direction);
}

TEST(Joystick, DownDirection) {
    uint8_t direction = calculate_direction(2048, 1500);  // X centered, Y below Y_POS_THRES_LOW
    TEST_ASSERT_EQUAL(JS_DIR_DOWN, direction);
}

TEST(Joystick, LeftUpDirection) {
    uint8_t direction = calculate_direction(1500, 2700);  // X below X_POS_THRES_LOW, Y above Y_POS_THRES_HIGH
    TEST_ASSERT_EQUAL(JS_DIR_LEFT_UP, direction);
}

TEST(Joystick, LeftDownDirection) {
    uint8_t direction = calculate_direction(1500, 1500);  // X below X_POS_THRES_LOW, Y below Y_POS_THRES_LOW
    TEST_ASSERT_EQUAL(JS_DIR_LEFT_DOWN, direction);
}

TEST(Joystick, RightUpDirection) {
    uint8_t direction = calculate_direction(2700, 2700);  // X above X_POS_THRES_HIGH, Y above Y_POS_THRES_HIGH
    TEST_ASSERT_EQUAL(JS_DIR_RIGHT_UP, direction);
}

TEST(Joystick, RightDownDirection) {
    uint8_t direction = calculate_direction(2700, 1500);  // X above X_POS_THRES_HIGH, Y below Y_POS_THRES_LOW
    TEST_ASSERT_EQUAL(JS_DIR_RIGHT_DOWN, direction);
}

TEST(Joystick, OutOfRangeHigh) {
    uint8_t direction = calculate_direction(4096, 4096);  // Both X and Y at max value
    TEST_ASSERT_EQUAL(JS_DIR_RIGHT_UP, direction);  // Should still register as right-up
}

TEST(Joystick, ThresholdTests) {
    // Test exact threshold values
    uint8_t direction;

    // Test lower X threshold
    direction = calculate_direction(X_POS_THRES_LOW, 2048);
    TEST_ASSERT_EQUAL(JS_DIR_LEFT, direction);

    // Test upper X threshold
    direction = calculate_direction(X_POS_THRES_HIGH, 2048);
    TEST_ASSERT_EQUAL(JS_DIR_RIGHT, direction);

    // Test lower Y threshold
    direction = calculate_direction(2048, Y_POS_THRES_LOW);
    TEST_ASSERT_EQUAL(JS_DIR_DOWN, direction);

    // Test upper Y threshold
    direction = calculate_direction(2048, Y_POS_THRES_HIGH);
    TEST_ASSERT_EQUAL(JS_DIR_UP, direction);
}

TEST(Joystick, StatusUpdateTest) {
    JoystickStatus status = joystick_get_status();
    TEST_ASSERT_EQUAL(JS_DIR_CENTERED, status.direction);  // Initial state should be centered
    TEST_ASSERT_EQUAL(0, status.is_new);  // Should not be new
}

TEST_GROUP_RUNNER(Joystick) {
    RUN_TEST_CASE(Joystick, CenterPosition);
    RUN_TEST_CASE(Joystick, LeftDirection);
    RUN_TEST_CASE(Joystick, RightDirection);
    RUN_TEST_CASE(Joystick, UpDirection);
    RUN_TEST_CASE(Joystick, DownDirection);
    RUN_TEST_CASE(Joystick, LeftUpDirection);
    RUN_TEST_CASE(Joystick, LeftDownDirection);
    RUN_TEST_CASE(Joystick, RightUpDirection);
    RUN_TEST_CASE(Joystick, RightDownDirection);
    RUN_TEST_CASE(Joystick, OutOfRangeHigh);
    RUN_TEST_CASE(Joystick, ThresholdTests);
    RUN_TEST_CASE(Joystick, StatusUpdateTest);
}
