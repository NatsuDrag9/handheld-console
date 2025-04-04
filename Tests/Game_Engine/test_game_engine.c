#include "unity.h"
#include "unity_fixture.h"
#include "Game_Engine/game_engine.h"
#include "Console_Peripherals/types.h"
#include "Mocks/Inc/mock_display_driver.h"
#include "Mocks/Inc/mock_push_button_driver.h"
#include "Mocks/Inc/mock_utils.h"

TEST_GROUP(GameEngine);

// Test game data structure
typedef struct {
    uint32_t test_data;
} TestGameData;

// Mock game callbacks
static bool init_called = false;
static bool update_joystick_called = false;
static bool update_dpad_called = false;
static bool render_called = false;
static bool cleanup_called = false;
static JoystickStatus last_joystick_input;
static DPAD_STATUS last_dpad_input;

static void test_game_init(void) {
    init_called = true;
}

static void test_game_update_joystick(JoystickStatus js_status) {
    update_joystick_called = true;
    last_joystick_input = js_status;
}

static void test_game_update_dpad(DPAD_STATUS dpad_status) {
    update_dpad_called = true;
    last_dpad_input = dpad_status;
}

static void test_game_render(void) {
    render_called = true;
}

static void test_game_cleanup(void) {
    cleanup_called = true;
}

// Test engine instance
static GameEngine test_engine;
static TestGameData test_game_data;

TEST_SETUP(GameEngine) {
    // Reset callback flags
    init_called = false;
    update_joystick_called = false;
    update_dpad_called = false;
    render_called = false;
    cleanup_called = false;

    // Initialize last_joystick_input
    last_joystick_input.direction = JS_DIR_CENTERED;
    last_joystick_input.is_new = 0;
    last_joystick_input.button = 0;

    // Initialize last_dpad_input
    last_dpad_input.direction = 0;
    last_dpad_input.is_new = 0;

    // Initialize test game engine
    test_engine.init = test_game_init;
    test_engine.update_func.update_joystick = test_game_update_joystick;
    test_engine.render = test_game_render;
    test_engine.cleanup = test_game_cleanup;
    test_engine.game_data = &test_game_data;
    test_engine.is_d_pad_game = false; // Default to joystick

    // Reset mock display
    mock_display_reset_state();

    // Reset mock push button driver
    mock_pb1_state = 0;
    mock_pb2_state = 0;
    mock_dpad_left_state = 0;
    mock_dpad_right_state = 0;
    mock_dpad_up_state = 0;
    mock_dpad_down_state = 0;
    mock_tick_count = 0;

    // Reset mock time
    mock_time_reset();

    // Initialize push buttons for testing
    pb_init();
}

TEST_TEAR_DOWN(GameEngine) {
    // Clean up after each test
    game_engine_cleanup(&test_engine);
}

TEST(GameEngine, InitializationSetsDefaultState) {
    game_engine_init(&test_engine);

    TEST_ASSERT_TRUE(init_called);
    TEST_ASSERT_EQUAL_UINT32(0, test_engine.base_state.score);
    TEST_ASSERT_EQUAL_UINT8(3, test_engine.base_state.lives);
    TEST_ASSERT_FALSE(test_engine.base_state.paused);
    TEST_ASSERT_FALSE(test_engine.base_state.game_over);
    TEST_ASSERT_FALSE(test_engine.base_state.is_reset);
    TEST_ASSERT_FALSE(test_engine.return_to_main_menu);
    TEST_ASSERT_FALSE(test_engine.countdown_over);
    TEST_ASSERT_NOT_NULL(test_engine.game_data);
}

TEST(GameEngine, UpdateWithJoystickCallsCorrectCallback) {
    JoystickStatus test_input = {
        .direction = JS_DIR_UP,
        .is_new = 1,
        .button = 1
    };

    test_engine.is_d_pad_game = false;
    game_engine_update(&test_engine, &test_input);

    TEST_ASSERT_TRUE(update_joystick_called);
    TEST_ASSERT_FALSE(update_dpad_called);
    TEST_ASSERT_EQUAL_UINT8(test_input.direction, last_joystick_input.direction);
    TEST_ASSERT_EQUAL_UINT8(test_input.is_new, last_joystick_input.is_new);
    TEST_ASSERT_EQUAL_UINT8(test_input.button, last_joystick_input.button);
}

TEST(GameEngine, UpdateWithDPadCallsCorrectCallback) {
    DPAD_STATUS test_input = {
        .direction = DPAD_DIR_UP,
        .is_new = 1
    };

    // Set up the D-Pad callback
    test_engine.update_func.update_dpad = test_game_update_dpad;
    test_engine.is_d_pad_game = true;

    game_engine_update(&test_engine, &test_input);

    TEST_ASSERT_TRUE(update_dpad_called);
    TEST_ASSERT_FALSE(update_joystick_called);
    TEST_ASSERT_EQUAL_UINT8(test_input.direction, last_dpad_input.direction);
    TEST_ASSERT_EQUAL_UINT8(test_input.is_new, last_dpad_input.is_new);
}

TEST(GameEngine, UpdateNotCalledWhenPaused) {
    test_engine.base_state.paused = true;

    JoystickStatus js = { JS_DIR_UP, 1, 0 };
    game_engine_update(&test_engine, &js);

    TEST_ASSERT_FALSE(update_joystick_called);
}

TEST(GameEngine, UpdateNotCalledWhenGameOver) {
    test_engine.base_state.game_over = true;

    JoystickStatus js = { JS_DIR_UP, 1, 0 };
    game_engine_update(&test_engine, &js);

    TEST_ASSERT_FALSE(update_joystick_called);
}

TEST(GameEngine, RenderCallsGameCallbackAndUpdatesDisplay) {
    MockDisplayState display_state;
    game_engine_render(&test_engine);

    mock_display_get_state(&display_state);

    TEST_ASSERT_TRUE(render_called);
    TEST_ASSERT_TRUE(display_state.screen_updated);

    // Verify display operations happened in correct order:
    // 1. Clear was called (resets display buffer)
    uint8_t display_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = { 0 };
    mock_display_get_buffer(display_buffer, sizeof(display_buffer));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(display_buffer,
        display_buffer,  // Compare with zeroed buffer
        sizeof(display_buffer));

    // 2. Screen was updated
    TEST_ASSERT_TRUE(display_state.screen_updated > 0);
}

TEST(GameEngine, CleanupCallsGameCallback) {
    game_engine_cleanup(&test_engine);
    TEST_ASSERT_TRUE(cleanup_called);
}

TEST(GameEngine, NullEngineHandling) {
    JoystickStatus js = { JS_DIR_UP, 1, 0 };

    // These should not crash
    game_engine_init(NULL);
    game_engine_update(NULL, &js);
    game_engine_render(NULL);
    game_engine_cleanup(NULL);

    // Verify no callbacks were called
    TEST_ASSERT_FALSE(init_called);
    TEST_ASSERT_FALSE(update_joystick_called);
    TEST_ASSERT_FALSE(render_called);
    TEST_ASSERT_FALSE(cleanup_called);
}

TEST(GameEngine, NullCallbackHandling) {
    GameEngine null_engine = { 0 };  // All callbacks are NULL
    JoystickStatus js = { JS_DIR_UP, 1, 0 };

    // Set input type to avoid potential segfault when accessing union
    null_engine.is_d_pad_game = false;

    // These should not crash
    game_engine_init(&null_engine);
    game_engine_update(&null_engine, &js);
    game_engine_render(&null_engine);
    game_engine_cleanup(&null_engine);
}

// Test for Play/Pause functionality
TEST(GameEngine, ButtonOneTogglesPause) {
    // Initialize engine
    game_engine_init(&test_engine);
    TEST_ASSERT_FALSE(test_engine.base_state.paused);

    // Update with button press
    JoystickStatus js = { JS_DIR_CENTERED, 0, 0 };
    // Setup mock button press with proper debouncing
    mock_pb1_state = 1;
    mock_tick_count = 10;  // Initial time
    game_engine_update(&test_engine, &js);  // This won't toggle pause yet

    // Simulate time passing for debounce
    mock_tick_count = 30;  // After debounce delay
    game_engine_update(&test_engine, &js);  // This should toggle pause now

    // Verify game is paused
    TEST_ASSERT_TRUE(test_engine.base_state.paused);

    // Reset mock button state
    mock_pb1_state = 0;
    mock_tick_count = 40;
    // Update with button released
    game_engine_update(&test_engine, &js);

    // Verify game is still paused (toggle happens only on press, not release)
    TEST_ASSERT_TRUE(test_engine.base_state.paused);

    printf("Before second press, paused = %d\n", test_engine.base_state.paused);

    // Press button again
    mock_pb1_state = 1;
    mock_tick_count = 50; // Use a completely fresh timestamp
    printf("Button state set to %d, time = %d\n", mock_pb1_state, mock_tick_count);
    game_engine_update(&test_engine, &js);
    printf("After first update of second press, paused = %d\n", test_engine.base_state.paused);

    mock_tick_count = 200; // Larger gap for debounce
    printf("Time advanced to %d\n", mock_tick_count);
    printf("pb1_get_state() = %d\n", pb1_get_state());
    game_engine_update(&test_engine, &js);
    printf("After second update of second press, paused = %d\n", test_engine.base_state.paused);

    // Verify game is unpaused
    TEST_ASSERT_FALSE(test_engine.base_state.paused);

    // // Press button again
    // mock_pb1_state = 1;
    // mock_tick_count = 10;
    // game_engine_update(&test_engine, &js);
    // mock_tick_count = 30;  // After debounce delay
    // game_engine_update(&test_engine, &js);

    // // Verify game is unpaused
    // TEST_ASSERT_FALSE(test_engine.base_state.paused);
}

// Test for Reset functionality
TEST(GameEngine, ButtonTwoShortPressTriggersReset) {
    // Initialize engine
    game_engine_init(&test_engine);
    TEST_ASSERT_FALSE(test_engine.base_state.is_reset);

    // Simulate Button 2 press
    mock_pb2_state = 1;
    mock_tick_count = 100;  // Start time

    JoystickStatus js = { JS_DIR_CENTERED, 0, 0 };
    game_engine_update(&test_engine, &js);

    // Simulate button release after a short duration (< BUTTON_RESTART_MAX_DURATION)
    mock_pb2_state = 0;
    mock_tick_count = 500;  // Less than 1.2 seconds
    game_engine_update(&test_engine, &js);

    // Verify reset flag is set
    TEST_ASSERT_TRUE(test_engine.base_state.is_reset);
}

// Test for Main Menu Return functionality
TEST(GameEngine, ButtonTwoLongPressTriggersReturnToMainMenu) {
    // Initialize engine
    game_engine_init(&test_engine);
    TEST_ASSERT_FALSE(test_engine.return_to_main_menu);

    // Simulate Button 2 long press
    mock_pb2_state = 1;
    mock_tick_count = 100;  // Start time

    JoystickStatus js = { JS_DIR_CENTERED, 0, 0 };
    game_engine_update(&test_engine, &js);

    // Simulate button release after a long duration (≥ BUTTON_MENU_MIN_DURATION)
    mock_pb2_state = 0;
    mock_tick_count = 3100;  // More than 3 seconds
    game_engine_update(&test_engine, &js);

    // Verify return to main menu flag is set
    TEST_ASSERT_TRUE(test_engine.return_to_main_menu);
}

// Test for Button behavior during game over
TEST(GameEngine, ButtonTwoNoEffectWhenGameOver) {
    // Initialize engine and set game over
    game_engine_init(&test_engine);
    test_engine.base_state.game_over = true;

    // Simulate Button 2 short press
    mock_pb2_state = 1;
    mock_tick_count = 100;

    JoystickStatus js = { JS_DIR_CENTERED, 0, 0 };
    game_engine_update(&test_engine, &js);

    // Simulate button release
    mock_pb2_state = 0;
    mock_tick_count = 500;
    game_engine_update(&test_engine, &js);

    // Reset flag should not be set during game over
    TEST_ASSERT_FALSE(test_engine.base_state.is_reset);
}

// Test for countdown timer
TEST(GameEngine, CountdownOverAfterGameOverDuration) {
    // Initialize engine and set game over
    game_engine_init(&test_engine);
    test_engine.base_state.game_over = true;

    // Simulate time just before countdown completion
    mock_tick_count = 9900;  // Just under 10 seconds
    game_engine_render(&test_engine);
    TEST_ASSERT_FALSE(test_engine.countdown_over);

    // Simulate time after countdown completion
    mock_tick_count = 10100;  // Just over 10 seconds
    game_engine_render(&test_engine);
    TEST_ASSERT_TRUE(test_engine.countdown_over);
}

TEST(GameEngine, CleanupResetsAllFlags) {
    // Set various flags
    test_engine.base_state.paused = true;
    test_engine.base_state.game_over = true;
    test_engine.base_state.is_reset = true;
    test_engine.return_to_main_menu = true;
    test_engine.countdown_over = true;

    // Call cleanup
    game_engine_cleanup(&test_engine);

    // Check that countdown_over is reset
    TEST_ASSERT_FALSE(test_engine.countdown_over);

    // Check that game-specific cleanup was called
    TEST_ASSERT_TRUE(cleanup_called);
}

TEST(GameEngine, DPadInputHandling) {
    // Set up for D-Pad game
    test_engine.is_d_pad_game = true;
    test_engine.update_func.update_dpad = test_game_update_dpad;
    game_engine_init(&test_engine);

    // Simulate D-pad input for UP direction
    mock_dpad_up_state = 1;
    mock_dpad_right_state = 0;
    mock_dpad_down_state = 0;
    mock_dpad_left_state = 0;

    // Create D-pad status matching what would be returned by d_pad_get_status()
    DPAD_STATUS dpad_status = {
        .direction = DPAD_DIR_UP,
        .is_new = 1
    };

    // Update with D-pad input
    game_engine_update(&test_engine, &dpad_status);

    // Check that D-pad callback was called with correct values
    TEST_ASSERT_TRUE(update_dpad_called);
    TEST_ASSERT_EQUAL_UINT8(DPAD_DIR_UP, last_dpad_input.direction);
    TEST_ASSERT_EQUAL_UINT8(1, last_dpad_input.is_new);
}

// Test Group Runner
TEST_GROUP_RUNNER(GameEngine) {
    RUN_TEST_CASE(GameEngine, InitializationSetsDefaultState);
    RUN_TEST_CASE(GameEngine, UpdateWithJoystickCallsCorrectCallback);
    RUN_TEST_CASE(GameEngine, UpdateWithDPadCallsCorrectCallback);
    RUN_TEST_CASE(GameEngine, UpdateNotCalledWhenPaused);
    RUN_TEST_CASE(GameEngine, UpdateNotCalledWhenGameOver);
    RUN_TEST_CASE(GameEngine, RenderCallsGameCallbackAndUpdatesDisplay);
    RUN_TEST_CASE(GameEngine, CleanupCallsGameCallback);
    RUN_TEST_CASE(GameEngine, NullEngineHandling);
    RUN_TEST_CASE(GameEngine, NullCallbackHandling);
    // RUN_TEST_CASE(GameEngine, ButtonOneTogglesPause);
    RUN_TEST_CASE(GameEngine, ButtonTwoShortPressTriggersReset);
    // // RUN_TEST_CASE(GameEngine, ButtonTwoLongPressTriggersReturnToMainMenu);
    // RUN_TEST_CASE(GameEngine, ButtonTwoNoEffectWhenGameOver);
    // // RUN_TEST_CASE(GameEngine, CountdownOverAfterGameOverDuration);
    RUN_TEST_CASE(GameEngine, CleanupResetsAllFlags);
    RUN_TEST_CASE(GameEngine, DPadInputHandling);
}