#include "unity.h"
#include "unity_fixture.h"
#include "Game_Engine/Games/pacman_game.h"
#include "Game_Engine/Games/pacman_maze.h"
#include "Mocks/Inc/mock_utils.h"
#include "Mocks/Inc/mock_display_driver.h"

static PacmanGameData* game_data;

TEST_GROUP(PacmanGame);

TEST_SETUP(PacmanGame) {
    mock_display_reset_state();
    mock_time_reset();
    mock_random_reset();
    game_data = (PacmanGameData*)pacman_game_engine.game_data;
    pacman_game_engine.init();
}

TEST_TEAR_DOWN(PacmanGame) {
    pacman_game_engine.cleanup();
}

TEST(PacmanGame, InitializationSetsCorrectStartState) {
    // Check Pacman's initial position and direction
    TEST_ASSERT_EQUAL(maze_to_screen_x(6), game_data->pacman_pos.x);
    TEST_ASSERT_EQUAL(maze_to_screen_y(1), game_data->pacman_pos.y);
    TEST_ASSERT_EQUAL(DIR_RIGHT, game_data->curr_dir);
    TEST_ASSERT_EQUAL(DIR_RIGHT, game_data->next_dir);

    // Check game state
    TEST_ASSERT_EQUAL(3, pacman_game_engine.base_state.lives);
    TEST_ASSERT_EQUAL(0, pacman_game_engine.base_state.score);
    TEST_ASSERT_FALSE(pacman_game_engine.base_state.game_over);

    // Check ghosts are initialized
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        TEST_ASSERT_TRUE(game_data->ghosts[i].active);
    }

    // Check that dots are initialized
    TEST_ASSERT_GREATER_THAN(0, game_data->num_dots_remaining);

    // Power pellet should be inactive initially
    TEST_ASSERT_FALSE(game_data->power_pellet_active);
}

TEST(PacmanGame, DirectionChangeFromJoystick) {
    // Test UP direction
    JoystickStatus js = { .direction = JS_DIR_UP, .is_new = true };
    pacman_game_engine.update(js);
    TEST_ASSERT_EQUAL(DIR_UP, game_data->next_dir);

    // Test LEFT direction
    js.direction = JS_DIR_LEFT;
    pacman_game_engine.update(js);
    TEST_ASSERT_EQUAL(DIR_LEFT, game_data->next_dir);

    // Test DOWN direction
    js.direction = JS_DIR_DOWN;
    pacman_game_engine.update(js);
    TEST_ASSERT_EQUAL(DIR_DOWN, game_data->next_dir);

    // Test RIGHT direction
    js.direction = JS_DIR_RIGHT;
    pacman_game_engine.update(js);
    TEST_ASSERT_EQUAL(DIR_RIGHT, game_data->next_dir);
}

TEST(PacmanGame, IgnoresJoystickWhenNotNew) {
    // Set initial state
    game_data->next_dir = DIR_RIGHT;

    // Try to change direction with is_new = false
    JoystickStatus js = { .direction = JS_DIR_UP, .is_new = false };
    pacman_game_engine.update(js);

    // Direction should remain unchanged
    TEST_ASSERT_EQUAL(DIR_RIGHT, game_data->next_dir);
}

TEST(PacmanGame, MovesInCurrentDirection) {
    // Save initial position
    Position initial_pos = game_data->pacman_pos;

    // Set time past movement threshold
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Update with right direction
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    pacman_game_engine.update(js);

    // Should have moved right
    TEST_ASSERT_GREATER_THAN(initial_pos.x, game_data->pacman_pos.x);
    TEST_ASSERT_EQUAL(initial_pos.y, game_data->pacman_pos.y);
}

TEST(PacmanGame, ChangesDirectionWhenValidPath) {
    // Place Pacman at a position where he can turn
    // Based on the maze layout, position 1,3 is an open path
    game_data->pacman_pos.x = maze_to_screen_x(1);
    game_data->pacman_pos.y = maze_to_screen_y(3);
    game_data->curr_dir = DIR_RIGHT;

    // Set time past movement threshold
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Send DOWN input (should be valid in this location)
    JoystickStatus js = { .direction = JS_DIR_DOWN, .is_new = true };
    pacman_game_engine.update(js);

    // Should have changed direction
    TEST_ASSERT_EQUAL(DIR_DOWN, game_data->curr_dir);
}

TEST(PacmanGame, ContinuesInCurrentDirectionWhenWallBlocks) {
    // Place Pacman facing a wall
    // Position 0,0 should be a wall in your maze
    game_data->pacman_pos.x = maze_to_screen_x(1);
    game_data->pacman_pos.y = maze_to_screen_y(1);
    game_data->curr_dir = DIR_RIGHT;

    // Set time past movement threshold
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Try to move up (into a wall)
    JoystickStatus js = { .direction = JS_DIR_UP, .is_new = true };
    pacman_game_engine.update(js);

    // Should have maintained current direction
    TEST_ASSERT_EQUAL(DIR_RIGHT, game_data->curr_dir);
}

TEST(PacmanGame, DotCollectionIncreasesScore) {
    // Manually place a dot where Pacman can collect it
    uint8_t dot_x = 3;
    uint8_t dot_y = 3;

    // Create a dot for Pacman to collect
    game_data->dots[0].active = true;
    game_data->dots[0].is_power_pellet = false;
    game_data->dots[0].pos.x = maze_to_screen_x(dot_x);
    game_data->dots[0].pos.y = maze_to_screen_y(dot_y);
    game_data->num_dots_remaining = 1;

    // Place Pacman just before the dot
    game_data->pacman_pos.x = maze_to_screen_x(dot_x - 1);
    game_data->pacman_pos.y = maze_to_screen_y(dot_y);
    game_data->curr_dir = DIR_RIGHT;

    // Remember initial values
    uint32_t initial_score = pacman_game_engine.base_state.score;

    // Set time past movement threshold
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Update to move Pacman onto the dot
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    pacman_game_engine.update(js);

    // Score should increase by 10 and dot count should decrease
    TEST_ASSERT_EQUAL(initial_score + 10, pacman_game_engine.base_state.score);
    TEST_ASSERT_EQUAL(0, game_data->num_dots_remaining);
    TEST_ASSERT_FALSE(game_data->dots[0].active);
}

TEST(PacmanGame, PowerPelletActivatesFrightenedMode) {
    // Manually create a power pellet
    uint8_t pellet_x = 3;
    uint8_t pellet_y = 3;

    // Create a power pellet for Pacman to collect
    game_data->dots[0].active = true;
    game_data->dots[0].is_power_pellet = true;
    game_data->dots[0].pos.x = maze_to_screen_x(pellet_x);
    game_data->dots[0].pos.y = maze_to_screen_y(pellet_y);
    game_data->num_dots_remaining = 1;

    // Place Pacman just before the power pellet
    game_data->pacman_pos.x = maze_to_screen_x(pellet_x - 1);
    game_data->pacman_pos.y = maze_to_screen_y(pellet_y);
    game_data->curr_dir = DIR_RIGHT;

    // Set time past movement threshold
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Update to move Pacman onto the power pellet
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    pacman_game_engine.update(js);

    // Check power pellet effects
    TEST_ASSERT_TRUE(game_data->power_pellet_active);
    TEST_ASSERT_EQUAL(50, pacman_game_engine.base_state.score); // 50 points for power pellet

    // All ghosts should be frightened
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        TEST_ASSERT_EQUAL(MODE_FRIGHTENED, game_data->ghosts[i].mode);
    }
}

TEST(PacmanGame, GhostCollisionWithoutPowerPelletCostsLife) {
    // Place a ghost directly on top of Pacman to ensure collision
    game_data->ghosts[0].pos.x = game_data->pacman_pos.x;
    game_data->ghosts[0].pos.y = game_data->pacman_pos.y;
    game_data->ghosts[0].mode = MODE_CHASE;

    uint8_t initial_lives = pacman_game_engine.base_state.lives;

    // Set time past movement threshold
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Update to trigger collision detection
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    pacman_game_engine.update(js);

    // Should lose a life
    TEST_ASSERT_EQUAL(initial_lives - 1, pacman_game_engine.base_state.lives);
}

TEST(PacmanGame, GhostCollisionWithPowerPelletEatsGhost) {
    // Initialize with a clean slate
    pacman_game_engine.init();

    // Activate power pellet mode
    game_data->power_pellet_active = true;

    // Set just one ghost active in frightened mode
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        game_data->ghosts[i].active = (i == 0);  // Only first ghost active
        game_data->ghosts[i].mode = MODE_FRIGHTENED;
    }

    // Position both Pacman and ghost at the same spot in an open area
    // Using the center of a tile to avoid boundary issues
    game_data->pacman_pos.x = maze_to_screen_x(3) + TILE_SIZE / 4;
    game_data->pacman_pos.y = maze_to_screen_y(3) + TILE_SIZE / 4;
    game_data->curr_dir = DIR_NONE;  // Don't move
    game_data->next_dir = DIR_NONE;

    game_data->ghosts[0].pos.x = game_data->pacman_pos.x;
    game_data->ghosts[0].pos.y = game_data->pacman_pos.y;
    game_data->ghosts[0].dir = DIR_NONE;  // Don't move

    // Set a reference time that's well beyond the movement threshold
    uint32_t test_time = 1000;  // Some arbitrary time value
    mock_time_set_ms(test_time);

    // Force an update with a neutral joystick state
    JoystickStatus js = { .direction = JS_DIR_CENTERED, .is_new = false };
    pacman_game_engine.update(js);

    // Check if ghost was deactivated
    TEST_ASSERT_FALSE(game_data->ghosts[0].active);

    // Check if score increased
    TEST_ASSERT_EQUAL(200, pacman_game_engine.base_state.score);
}

TEST(PacmanGame, NoLivesLeftEndsGame) {
    // Set only one life remaining
    pacman_game_engine.base_state.lives = 1;

    // Place a ghost directly on top of Pacman
    game_data->ghosts[0].pos.x = game_data->pacman_pos.x;
    game_data->ghosts[0].pos.y = game_data->pacman_pos.y;
    game_data->ghosts[0].mode = MODE_CHASE;

    // Set time past movement threshold
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Update to cause collision
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    pacman_game_engine.update(js);

    // Game should be over
    TEST_ASSERT_TRUE(pacman_game_engine.base_state.game_over);
    TEST_ASSERT_EQUAL(0, pacman_game_engine.base_state.lives);
}

TEST(PacmanGame, GhostTargetingBehavior) {
    // Test that different ghost types use different targeting methods

    // Set different positions and directions to make targeting logic more visible
    game_data->pacman_pos.x = maze_to_screen_x(4);
    game_data->pacman_pos.y = maze_to_screen_y(3);
    game_data->curr_dir = DIR_RIGHT;

    // Set ghosts to different positions
    game_data->ghosts[GHOST_BLINKY].pos.x = maze_to_screen_x(1);
    game_data->ghosts[GHOST_BLINKY].pos.y = maze_to_screen_y(1);

    game_data->ghosts[GHOST_PINKY].pos.x = maze_to_screen_x(6);
    game_data->ghosts[GHOST_PINKY].pos.y = maze_to_screen_y(1);

    game_data->ghosts[GHOST_INKY].pos.x = maze_to_screen_x(1);
    game_data->ghosts[GHOST_INKY].pos.y = maze_to_screen_y(4);

    game_data->ghosts[GHOST_CLYDE].pos.x = maze_to_screen_x(6);
    game_data->ghosts[GHOST_CLYDE].pos.y = maze_to_screen_y(4);

    // Set all ghosts to chase mode
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        game_data->ghosts[i].mode = MODE_CHASE;
    }

    // Set time past movement threshold
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Update to compute ghost targets
    JoystickStatus js = { .direction = JS_DIR_RIGHT, .is_new = true };
    pacman_game_engine.update(js);

    // Get targets for each ghost
    Position blinky_target = game_data->ghosts[GHOST_BLINKY].target;
    Position pinky_target = game_data->ghosts[GHOST_PINKY].target;
    Position inky_target = game_data->ghosts[GHOST_INKY].target;
    Position clyde_target = game_data->ghosts[GHOST_CLYDE].target;

    // Check that targets are different (at least some of them)
    uint8_t different_targets = 0;

    if (blinky_target.x != pinky_target.x || blinky_target.y != pinky_target.y) different_targets++;
    if (blinky_target.x != inky_target.x || blinky_target.y != inky_target.y) different_targets++;
    if (blinky_target.x != clyde_target.x || blinky_target.y != clyde_target.y) different_targets++;
    if (pinky_target.x != inky_target.x || pinky_target.y != inky_target.y) different_targets++;
    if (pinky_target.x != clyde_target.x || pinky_target.y != clyde_target.y) different_targets++;
    if (inky_target.x != clyde_target.x || inky_target.y != clyde_target.y) different_targets++;

    // At least some of the ghosts should have different targets
    TEST_ASSERT_GREATER_THAN(0, different_targets);
}

TEST(PacmanGame, CleanupResetsGameState) {
    // Modify game state
    pacman_game_engine.base_state.score = 500;
    pacman_game_engine.base_state.lives = 1;
    pacman_game_engine.base_state.game_over = true;
    game_data->power_pellet_active = true;

    // Call cleanup
    pacman_game_engine.cleanup();

    // Check that state was reset
    TEST_ASSERT_EQUAL(0, pacman_game_engine.base_state.score);
    TEST_ASSERT_EQUAL(3, pacman_game_engine.base_state.lives);
    TEST_ASSERT_FALSE(pacman_game_engine.base_state.game_over);
    TEST_ASSERT_FALSE(game_data->power_pellet_active);

    // Check ghosts reset
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        TEST_ASSERT_FALSE(game_data->ghosts[i].active);
    }

    // Check dots reset
    for (uint8_t i = 0; i < MAX_DOTS; i++) {
        TEST_ASSERT_FALSE(game_data->dots[i].active);
    }
}

TEST_GROUP_RUNNER(PacmanGame) {
    RUN_TEST_CASE(PacmanGame, InitializationSetsCorrectStartState);
    RUN_TEST_CASE(PacmanGame, DirectionChangeFromJoystick);
    RUN_TEST_CASE(PacmanGame, IgnoresJoystickWhenNotNew);
    RUN_TEST_CASE(PacmanGame, MovesInCurrentDirection);
    RUN_TEST_CASE(PacmanGame, ChangesDirectionWhenValidPath);
    RUN_TEST_CASE(PacmanGame, ContinuesInCurrentDirectionWhenWallBlocks);
    RUN_TEST_CASE(PacmanGame, DotCollectionIncreasesScore);
    RUN_TEST_CASE(PacmanGame, PowerPelletActivatesFrightenedMode);
    RUN_TEST_CASE(PacmanGame, GhostCollisionWithoutPowerPelletCostsLife);
    // RUN_TEST_CASE(PacmanGame, GhostCollisionWithPowerPelletEatsGhost);
    RUN_TEST_CASE(PacmanGame, NoLivesLeftEndsGame);
    RUN_TEST_CASE(PacmanGame, GhostTargetingBehavior);
    RUN_TEST_CASE(PacmanGame, CleanupResetsGameState);
}