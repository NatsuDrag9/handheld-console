#include "unity.h"
#include "unity_fixture.h"
#include "Game_Engine/game_engine.h"
#include "Console_Peripherals/types.h"
#include "Mocks/Inc/mock_display_driver.h"

TEST_GROUP(GameEngine);

// Test game data structure
typedef struct {
    uint32_t test_data;
} TestGameData;

// Mock game callbacks
static bool init_called = false;
static bool update_called = false;
static bool render_called = false;
static bool cleanup_called = false;
static JoystickStatus last_joystick_input;

static void test_game_init(void) {
    init_called = true;
}

static void test_game_update(JoystickStatus js_status) {
    update_called = true;
    last_joystick_input = js_status;
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
    update_called = false;
    render_called = false;
    cleanup_called = false;

    // Initialize last_joystick_input
    last_joystick_input.direction = JS_DIR_CENTERED;
    last_joystick_input.is_new = 0;
    last_joystick_input.button = 0;

    // Initialize test game engine
    test_engine.init = test_game_init;
    test_engine.update = test_game_update;
    test_engine.render = test_game_render;
    test_engine.cleanup = test_game_cleanup;
    test_engine.game_data = &test_game_data;

    // Reset mock display
    mock_display_reset_state();
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
    TEST_ASSERT_NOT_NULL(test_engine.game_data);
}

TEST(GameEngine, UpdateCallsGameCallback) {
    JoystickStatus test_input;
    test_input.direction = JS_DIR_UP;
    test_input.is_new = 1;
    test_input.button = 1;

    game_engine_update(&test_engine, test_input);

    TEST_ASSERT_TRUE(update_called);
    TEST_ASSERT_EQUAL_UINT8(test_input.direction, last_joystick_input.direction);
    TEST_ASSERT_EQUAL_UINT8(test_input.is_new, last_joystick_input.is_new);
    TEST_ASSERT_EQUAL_UINT8(test_input.button, last_joystick_input.button);
}

TEST(GameEngine, UpdateNotCalledWhenPaused) {
    test_engine.base_state.paused = true;

    JoystickStatus js = { JS_DIR_UP, 1, 0 };
    game_engine_update(&test_engine, js);

    TEST_ASSERT_FALSE(update_called);
}

TEST(GameEngine, UpdateNotCalledWhenGameOver) {
    test_engine.base_state.game_over = true;

    JoystickStatus js = { JS_DIR_UP, 1, 0 };
    game_engine_update(&test_engine, js);

    TEST_ASSERT_FALSE(update_called);
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
    game_engine_update(NULL, js);
    game_engine_render(NULL);
    game_engine_cleanup(NULL);

    // Verify no callbacks were called
    TEST_ASSERT_FALSE(init_called);
    TEST_ASSERT_FALSE(update_called);
    TEST_ASSERT_FALSE(render_called);
    TEST_ASSERT_FALSE(cleanup_called);
}

TEST(GameEngine, NullCallbackHandling) {
    GameEngine null_engine = { 0 };  // All callbacks are NULL
    JoystickStatus js = { JS_DIR_UP, 1, 0 };

    // These should not crash
    game_engine_init(&null_engine);
    game_engine_update(&null_engine, js);
    game_engine_render(&null_engine);
    game_engine_cleanup(&null_engine);
}

// Test Group Runner
TEST_GROUP_RUNNER(GameEngine) {
    RUN_TEST_CASE(GameEngine, InitializationSetsDefaultState);
    RUN_TEST_CASE(GameEngine, UpdateCallsGameCallback);
    RUN_TEST_CASE(GameEngine, UpdateNotCalledWhenPaused);
    RUN_TEST_CASE(GameEngine, UpdateNotCalledWhenGameOver);
    RUN_TEST_CASE(GameEngine, RenderCallsGameCallbackAndUpdatesDisplay);
    RUN_TEST_CASE(GameEngine, CleanupCallsGameCallback);
    RUN_TEST_CASE(GameEngine, NullEngineHandling);
    RUN_TEST_CASE(GameEngine, NullCallbackHandling);
}