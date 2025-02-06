#include "unity.h"
#include "unity_fixture.h"
#include "Game_Engine/Games/snake_game.h"
#include "Mocks/Inc/mock_utils.h"
#include "Mocks/Inc/mock_display_driver.h"

static SnakeGameData* game_data;

TEST_GROUP(SnakeGame);

TEST_SETUP(SnakeGame) {
    mock_display_reset_state();
    mock_time_reset();
    mock_random_reset();
    game_data = (SnakeGameData*)snake_game_engine.game_data;
    snake_game_engine.init();
}

TEST_TEAR_DOWN(SnakeGame) {
    snake_game_engine.cleanup();
}

TEST(SnakeGame, InitializationSetsCorrectStartState) {
    TEST_ASSERT_EQUAL(DISPLAY_WIDTH / 2, game_data->head_x);
    TEST_ASSERT_EQUAL(DISPLAY_HEIGHT / 2, game_data->head_y);
    TEST_ASSERT_EQUAL(JS_DIR_RIGHT, game_data->direction);
    TEST_ASSERT_EQUAL(1, game_data->length);
    TEST_ASSERT_EQUAL(3, snake_game_engine.base_state.lives);
    TEST_ASSERT_EQUAL(0, snake_game_engine.base_state.score);
}

TEST(SnakeGame, BodyFollowsHead) {
    game_data->length = 2;
    uint8_t initial_body_x = game_data->head_x;
    uint8_t initial_body_y = game_data->head_y;

    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    mock_time_set_ms(SNAKE_SPEED + 1);
    snake_game_engine.update(js);

    TEST_ASSERT_EQUAL(initial_body_x, game_data->body[0].x);
    TEST_ASSERT_EQUAL(initial_body_y, game_data->body[0].y);
}

TEST(SnakeGame, DirectionChangeIgnoredIfNotNew) {
    game_data->direction = JS_DIR_RIGHT;
    JoystickStatus js = { .direction = JS_DIR_UP, .is_new = false };
    snake_game_engine.update(js);
    TEST_ASSERT_EQUAL(JS_DIR_RIGHT, game_data->direction);
}

TEST(SnakeGame, SnakeMovesSpriteWidthInDirectionOfMovement) {
    uint8_t initial_x = game_data->head_x;
    uint8_t initial_y = game_data->head_y;

    JoystickStatus js = {
        .direction = JS_DIR_RIGHT,
        .is_new = true
    };

    mock_time_set_ms(SNAKE_SPEED + 1);
    snake_game_engine.update(js);

    TEST_ASSERT_EQUAL(initial_x + SNAKE_SPRITE_SIZE, game_data->head_x);
    TEST_ASSERT_EQUAL(initial_y, game_data->head_y);
}

TEST(SnakeGame, CannotReverseDirection) {
    game_data->direction = JS_DIR_RIGHT;
    JoystickStatus js = { .direction = JS_DIR_LEFT, .is_new = true };
    snake_game_engine.update(js);
    TEST_ASSERT_EQUAL(JS_DIR_RIGHT, game_data->direction);
}

TEST(SnakeGame, SnakeWrapsHorizontally) {
    game_data->head_x = DISPLAY_WIDTH - SNAKE_SPRITE_SIZE;
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    mock_time_set_ms(SNAKE_SPEED + 1);
    snake_game_engine.update(js);
    TEST_ASSERT_EQUAL(BORDER_OFFSET, game_data->head_x);
}

TEST(SnakeGame, SnakeSpeedIncreases) {
    uint8_t initial_x = game_data->head_x;
    snake_game_engine.base_state.score = 200;
    mock_time_set_ms(SNAKE_SPEED - 180);  // Less than reduced speed threshold
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    snake_game_engine.update(js);
    TEST_ASSERT_NOT_EQUAL(initial_x, game_data->head_x);
}

TEST(SnakeGame, SnakeWrapsVertically) {
    game_data->head_y = DISPLAY_HEIGHT - SNAKE_SPRITE_SIZE;
    JoystickStatus js = { .direction = JS_DIR_DOWN, .is_new = true };
    mock_time_set_ms(SNAKE_SPEED + 1);
    snake_game_engine.update(js);
    TEST_ASSERT_EQUAL(GAME_AREA_TOP, game_data->head_y);
}

TEST(SnakeGame, MovementOnlyOccursAtCorrectInterval) {
    uint8_t initial_x = game_data->head_x;
    mock_time_set_ms(SNAKE_SPEED - 1);
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    snake_game_engine.update(js);
    TEST_ASSERT_EQUAL(initial_x, game_data->head_x);
}

TEST(SnakeGame, SnakeLengthLimitedToMaxSize) {
    for (int i = 0; i < 65; i++) {
        game_data->food.x = game_data->head_x + SNAKE_SPRITE_SIZE;
        game_data->food.y = game_data->head_y;
        mock_time_set_ms((i + 1) * SNAKE_SPEED + 1);
        JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
        snake_game_engine.update(js);
    }
    TEST_ASSERT_LESS_OR_EQUAL(64, game_data->length);
}


TEST(SnakeGame, FoodSpawnsInBoundsAfterCollection) {
    // Initial food position from init
    TEST_ASSERT_GREATER_OR_EQUAL(BORDER_OFFSET, game_data->food.x);
    TEST_ASSERT_LESS_THAN(DISPLAY_WIDTH - BORDER_OFFSET - SNAKE_SPRITE_SIZE, game_data->food.x);
    TEST_ASSERT_GREATER_OR_EQUAL(GAME_AREA_TOP, game_data->food.y);
    TEST_ASSERT_LESS_THAN(DISPLAY_HEIGHT - BORDER_OFFSET - SNAKE_SPRITE_SIZE, game_data->food.y);

    // Trigger food respawn through collection
    game_data->head_x = game_data->food.x - SNAKE_SPRITE_SIZE;
    game_data->head_y = game_data->food.y;

    mock_random_set_next_value(50);
    mock_time_set_ms(SNAKE_SPEED + 1);

    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    snake_game_engine.update(js);

    // Check new food position is also in bounds
    TEST_ASSERT_GREATER_OR_EQUAL(BORDER_OFFSET, game_data->food.x);
    TEST_ASSERT_LESS_THAN(DISPLAY_WIDTH - BORDER_OFFSET - SNAKE_SPRITE_SIZE, game_data->food.x);
    TEST_ASSERT_GREATER_OR_EQUAL(GAME_AREA_TOP, game_data->food.y);
    TEST_ASSERT_LESS_THAN(DISPLAY_HEIGHT - BORDER_OFFSET - SNAKE_SPRITE_SIZE, game_data->food.y);
}

TEST(SnakeGame, FoodRelocatesAfterCollection) {
    uint8_t old_food_x = game_data->food.x;
    uint8_t old_food_y = game_data->food.y;

    // Move snake to food
    game_data->head_x = old_food_x - SNAKE_SPRITE_SIZE;  // Position just before food
    game_data->head_y = old_food_y;

    mock_random_set_next_value(50);
    mock_time_set_ms(SNAKE_SPEED + 1);

    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    snake_game_engine.update(js);  // This movement should collect food

    mock_time_set_ms(SNAKE_SPEED * 2);  // Wait for another update
    snake_game_engine.update(js);

    TEST_ASSERT_NOT_EQUAL(old_food_x, game_data->food.x);
    TEST_ASSERT_NOT_EQUAL(old_food_y, game_data->food.y);
}

TEST(SnakeGame, SnakeCollectsFood) {
    // Need to wait for movement to update
    mock_time_set_ms(SNAKE_SPEED + 1);

    // Position the food where snake will collide
    game_data->food.x = game_data->head_x + SNAKE_SPRITE_SIZE;
    game_data->food.y = game_data->head_y;

    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    snake_game_engine.update(js);

    TEST_ASSERT_EQUAL(2, game_data->length);
    TEST_ASSERT_EQUAL(10, snake_game_engine.base_state.score);
}


TEST(SnakeGame, SnakeSelfCollisionReducesLives) {
    // Set up collision with body segment. Length must be sufficiently large for self collision to occur
    // Chose 10 based on experience
    game_data->length = 10;
    game_data->body[1].x = game_data->head_x + SNAKE_SPRITE_SIZE;
    game_data->body[1].y = game_data->head_y;

    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    mock_time_set_ms(SNAKE_SPEED + 1);
    snake_game_engine.update(js);

    TEST_ASSERT_EQUAL(2, snake_game_engine.base_state.lives);  // Should decrease from 3 to 2
}

TEST(SnakeGame, SelfCollisionWithNoLivesEndsGame) {
    snake_game_engine.base_state.lives = 1;
    game_data->length = 10;
    game_data->body[1].x = game_data->head_x + SNAKE_SPRITE_SIZE;
    game_data->body[1].y = game_data->head_y;
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    mock_time_set_ms(SNAKE_SPEED + 1);
    snake_game_engine.update(js);
    TEST_ASSERT_TRUE(snake_game_engine.base_state.game_over);
}

TEST_GROUP_RUNNER(SnakeGame) {
    RUN_TEST_CASE(SnakeGame, InitializationSetsCorrectStartState);
    RUN_TEST_CASE(SnakeGame, BodyFollowsHead);
    RUN_TEST_CASE(SnakeGame, SnakeMovesSpriteWidthInDirectionOfMovement);
    RUN_TEST_CASE(SnakeGame, DirectionChangeIgnoredIfNotNew);
    RUN_TEST_CASE(SnakeGame, CannotReverseDirection);
    RUN_TEST_CASE(SnakeGame, SnakeWrapsHorizontally);
    RUN_TEST_CASE(SnakeGame, SnakeWrapsVertically);
    RUN_TEST_CASE(SnakeGame, SnakeSpeedIncreases);
    RUN_TEST_CASE(SnakeGame, MovementOnlyOccursAtCorrectInterval);
    RUN_TEST_CASE(SnakeGame, SnakeLengthLimitedToMaxSize);
    RUN_TEST_CASE(SnakeGame, FoodRelocatesAfterCollection);
    RUN_TEST_CASE(SnakeGame, FoodSpawnsInBoundsAfterCollection);
    RUN_TEST_CASE(SnakeGame, SnakeCollectsFood);
    RUN_TEST_CASE(SnakeGame, SnakeSelfCollisionReducesLives);
    RUN_TEST_CASE(SnakeGame, SelfCollisionWithNoLivesEndsGame);
}