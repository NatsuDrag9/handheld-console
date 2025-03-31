#include "unity.h"
#include "unity_fixture.h"
#include "Game_Engine/Games/pacman_game.h"
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

    // Check game engine state
    TEST_ASSERT_EQUAL(3, pacman_game_engine.base_state.lives);
    TEST_ASSERT_EQUAL(0, pacman_game_engine.base_state.score);
    TEST_ASSERT_FALSE(pacman_game_engine.base_state.game_over);
    TEST_ASSERT_TRUE(pacman_game_engine.is_d_pad_game);

    // Check that ghosts are initialized
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        TEST_ASSERT_TRUE(game_data->ghosts[i].active);
    }

    // Check that dots are initialized
    TEST_ASSERT_TRUE(game_data->num_dots_remaining > 0);
}

TEST(PacmanGame, DirectionChangeWithDPad) {
    // Set initial direction
    game_data->curr_dir = DIR_RIGHT;
    game_data->next_dir = DIR_RIGHT;

    // Change direction with D-pad
    DPAD_STATUS dpad = { .direction = DPAD_DIR_UP, .is_new = 1 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Check new direction is set
    TEST_ASSERT_EQUAL(DIR_UP, game_data->next_dir);
}

TEST(PacmanGame, DirectionChangeIgnoredIfNotNew) {
    // Set initial direction
    game_data->curr_dir = DIR_RIGHT;
    game_data->next_dir = DIR_RIGHT;

    // Try to change direction with is_new=0
    DPAD_STATUS dpad = { .direction = DPAD_DIR_UP, .is_new = 0 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Direction should remain unchanged
    TEST_ASSERT_EQUAL(DIR_RIGHT, game_data->next_dir);
}

TEST(PacmanGame, PacmanMovesInCurrentDirection) {
    // Position before moving
    uint8_t initial_x = game_data->pacman_pos.x;
    uint8_t initial_y = game_data->pacman_pos.y;
    game_data->curr_dir = DIR_RIGHT;

    // Trigger movement
    mock_time_set_ms(PACMAN_SPEED + 1);
    DPAD_STATUS dpad = { .direction = 0, .is_new = 0 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Check new position
    TEST_ASSERT_EQUAL(initial_x + TILE_SIZE, game_data->pacman_pos.x);
    TEST_ASSERT_EQUAL(initial_y, game_data->pacman_pos.y);
}

TEST(PacmanGame, PacmanChangesDirectionWhenPossible) {
    // Use maze coordinates from your maze layout where there's definitely a path
    // This is an open path (non-wall) position
    game_data->pacman_pos.x = maze_to_screen_x(6);  // Position in an open corridor
    game_data->pacman_pos.y = maze_to_screen_y(1);  // Position in an open corridor
    game_data->curr_dir = DIR_RIGHT;
    game_data->next_dir = DIR_RIGHT;

    // Set next position to be open (not a wall)
    // This position would be above the current position
    mock_time_set_ms(PACMAN_SPEED + 1);

    // Test UP direction when there's space to move up
    DPAD_STATUS dpad = { .direction = DPAD_DIR_UP, .is_new = 1 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Verify next_dir changes immediately
    TEST_ASSERT_EQUAL(DIR_UP, game_data->next_dir);

    // Move enough times to ensure the direction change takes effect
    for (int i = 0; i < 3; i++) {
        mock_time_set_ms((i + 2) * PACMAN_SPEED + 1);
        pacman_game_engine.update_func.update_dpad(dpad);
    }

    // Now curr_dir should have changed too
    TEST_ASSERT_EQUAL(DIR_UP, game_data->curr_dir);
}

TEST(PacmanGame, PacmanStopsAtWalls) {
    // Use a known wall from the MAZE_LAYOUT
    // Based on the is_wall test, (8, 12) is a wall position

    // Position Pacman adjacent to a wall
    game_data->pacman_pos.x = 16; // maze_to_screen_x(1) = 16
    game_data->pacman_pos.y = 12; // maze_to_screen_y(0) = 12
    game_data->curr_dir = DIR_LEFT; // Try to move left into wall at (8, 12)

    // Record initial position
    uint8_t initial_x = game_data->pacman_pos.x;

    // Trigger movement
    mock_time_set_ms(PACMAN_SPEED + 1);
    DPAD_STATUS dpad = { .direction = DIR_LEFT, .is_new = 0 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Position should remain unchanged
    TEST_ASSERT_EQUAL(initial_x, game_data->pacman_pos.x);
}

TEST(PacmanGame, CollectingDotIncreasesScore) {
    // Position Pacman where a dot exists
    uint8_t i;
    for (i = 0; i < MAX_DOTS; i++) {
        if (game_data->dots[i].active && !game_data->dots[i].is_power_pellet) {
            break;
        }
    }

    if (i < MAX_DOTS) {
        // Position Pacman at the dot
        game_data->pacman_pos.x = game_data->dots[i].pos.x;
        game_data->pacman_pos.y = game_data->dots[i].pos.y;

        // Save initial values
        uint32_t initial_score = pacman_game_engine.base_state.score;
        uint8_t initial_dots = game_data->num_dots_remaining;

        // Trigger update to detect collision
        mock_time_set_ms(PACMAN_SPEED + 1);
        DPAD_STATUS dpad = { .direction = 0, .is_new = 0 };
        pacman_game_engine.update_func.update_dpad(dpad);

        // Check score increased and dot was collected
        TEST_ASSERT_EQUAL(initial_score + 10, pacman_game_engine.base_state.score);
        TEST_ASSERT_EQUAL(initial_dots - 1, game_data->num_dots_remaining);
        TEST_ASSERT_FALSE(game_data->dots[i].active);
    }
}

TEST(PacmanGame, CollectingPowerPelletActivatesPowerMode) {
    // Create a power pellet at a specific position
    uint8_t pellet_index = 0;

    // Make sure the dot is active and is a power pellet
    game_data->dots[pellet_index].active = true;
    game_data->dots[pellet_index].is_power_pellet = true;

    // Center of the tile at maze coordinates (1,1)
    uint8_t tile_x = maze_to_screen_x(1);
    uint8_t tile_y = maze_to_screen_y(1);

    game_data->dots[pellet_index].pos.x = tile_x;
    game_data->dots[pellet_index].pos.y = tile_y;

    // Position Pacman exactly at the same spot
    game_data->pacman_pos.x = tile_x;
    game_data->pacman_pos.y = tile_y;

    // Make sure there are dots remaining
    game_data->num_dots_remaining = 10;

    // Verify initial conditions
    TEST_ASSERT_TRUE(game_data->dots[pellet_index].active);
    TEST_ASSERT_TRUE(game_data->dots[pellet_index].is_power_pellet);
    TEST_ASSERT_EQUAL(tile_x, game_data->dots[pellet_index].pos.x);
    TEST_ASSERT_EQUAL(tile_y, game_data->dots[pellet_index].pos.y);
    TEST_ASSERT_EQUAL(tile_x, game_data->pacman_pos.x);
    TEST_ASSERT_EQUAL(tile_y, game_data->pacman_pos.y);

    // Calculate the expected distance - should be 0
    int dx = abs(game_data->pacman_pos.x - game_data->dots[pellet_index].pos.x);
    int dy = abs(game_data->pacman_pos.y - game_data->dots[pellet_index].pos.y);
    TEST_ASSERT_LESS_THAN(TILE_SIZE / 2, dx);
    TEST_ASSERT_LESS_THAN(TILE_SIZE / 2, dy);

    // Trigger the update to detect collision
    mock_time_set_ms(PACMAN_SPEED + 1);
    DPAD_STATUS dpad = { .direction = DPAD_DIR_RIGHT, .is_new = 0 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Check if the dot was consumed (active=false)
    TEST_ASSERT_FALSE(game_data->dots[pellet_index].active);

    // Now check if power mode was activated
    TEST_ASSERT_TRUE(game_data->power_pellet_active);

    // Check if ghosts were set to frightened mode
    for (uint8_t i = 0; i < NUM_GHOSTS; i++) {
        TEST_ASSERT_EQUAL(MODE_FRIGHTENED, game_data->ghosts[i].mode);
    }
}

TEST(PacmanGame, GhostCollisionReducesLives) {
    // Position a ghost at Pacman's location
    game_data->ghosts[0].pos.x = game_data->pacman_pos.x;
    game_data->ghosts[0].pos.y = game_data->pacman_pos.y;
    game_data->ghosts[0].mode = MODE_CHASE; // Ensure ghost is in chase mode

    // Save initial lives
    uint8_t initial_lives = pacman_game_engine.base_state.lives;

    // Trigger update to detect collision
    mock_time_set_ms(PACMAN_SPEED + 1);
    DPAD_STATUS dpad = { .direction = 0, .is_new = 0 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Check lives reduced
    TEST_ASSERT_EQUAL(initial_lives - 1, pacman_game_engine.base_state.lives);
}

TEST(PacmanGame, GhostCollisionInFrightenedModeIncreasesScore) {
    // Set up frightened ghost exactly at Pacman's position
    game_data->ghosts[0].active = true;
    game_data->ghosts[0].mode = MODE_FRIGHTENED;

    // Center of the tile at maze coordinates (2,2)
    uint8_t tile_x = maze_to_screen_x(2);
    uint8_t tile_y = maze_to_screen_y(2);

    // Position both exactly at the same coordinates
    game_data->pacman_pos.x = tile_x;
    game_data->pacman_pos.y = tile_y;
    game_data->ghosts[0].pos.x = tile_x;
    game_data->ghosts[0].pos.y = tile_y;

    // Save initial score
    uint32_t initial_score = pacman_game_engine.base_state.score;

    // Trigger update to detect collision
    mock_time_set_ms(PACMAN_SPEED + 1);
    DPAD_STATUS dpad = { .direction = DPAD_DIR_RIGHT, .is_new = 0 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Check score increased and ghost deactivated
    TEST_ASSERT_EQUAL(initial_score + 200, pacman_game_engine.base_state.score);
    TEST_ASSERT_FALSE(game_data->ghosts[0].active);
}

TEST(PacmanGame, NoLivesLeftEndsGame) {
    // Set lives to 1 so next hit will end game
    pacman_game_engine.base_state.lives = 1;

    // Position ghost at Pacman's location
    game_data->ghosts[0].pos.x = game_data->pacman_pos.x;
    game_data->ghosts[0].pos.y = game_data->pacman_pos.y;
    game_data->ghosts[0].mode = MODE_CHASE;

    // Trigger collision
    mock_time_set_ms(PACMAN_SPEED + 1);
    DPAD_STATUS dpad = { .direction = 0, .is_new = 0 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Check game over state
    TEST_ASSERT_EQUAL(0, pacman_game_engine.base_state.lives);
    TEST_ASSERT_TRUE(pacman_game_engine.base_state.game_over);
}

TEST(PacmanGame, MovementOnlyOccursAtCorrectInterval) {
    // Initial position
    uint8_t initial_x = game_data->pacman_pos.x;
    uint8_t initial_y = game_data->pacman_pos.y;

    // Set time just below movement threshold
    mock_time_set_ms(PACMAN_SPEED - 1);

    // Update game
    DPAD_STATUS dpad = { .direction = 0, .is_new = 0 };
    pacman_game_engine.update_func.update_dpad(dpad);

    // Position should not change
    TEST_ASSERT_EQUAL(initial_x, game_data->pacman_pos.x);
    TEST_ASSERT_EQUAL(initial_y, game_data->pacman_pos.y);
}

TEST(PacmanGame, GhostsChasePlayerInChaseMode) {
    // Position Pacman far from a ghost in an open area
    game_data->pacman_pos.x = maze_to_screen_x(8);
    game_data->pacman_pos.y = maze_to_screen_y(3);

    // Position ghost some distance away
    game_data->ghosts[0].pos.x = maze_to_screen_x(5);
    game_data->ghosts[0].pos.y = maze_to_screen_y(3);
    game_data->ghosts[0].mode = MODE_CHASE;
    game_data->ghosts[0].active = true;
    game_data->ghosts[0].dir = DIR_RIGHT;  // Initially moving right

    // Store initial position
    uint8_t initial_x = game_data->ghosts[0].pos.x;

    // Update game enough times for ghost to move
    for (int i = 0; i < 5; i++) {
        mock_time_set_ms((i + 1) * PACMAN_SPEED + 1);
        DPAD_STATUS dpad = { .direction = 0, .is_new = 0 };
        pacman_game_engine.update_func.update_dpad(dpad);
    }

    TEST_ASSERT_NOT_EQUAL(initial_x, game_data->ghosts[0].pos.x);
}

TEST_GROUP_RUNNER(PacmanGame) {
    RUN_TEST_CASE(PacmanGame, InitializationSetsCorrectStartState);
    RUN_TEST_CASE(PacmanGame, DirectionChangeWithDPad);
    RUN_TEST_CASE(PacmanGame, DirectionChangeIgnoredIfNotNew);
    RUN_TEST_CASE(PacmanGame, PacmanMovesInCurrentDirection);
    RUN_TEST_CASE(PacmanGame, PacmanChangesDirectionWhenPossible);
    RUN_TEST_CASE(PacmanGame, PacmanStopsAtWalls);
    RUN_TEST_CASE(PacmanGame, CollectingDotIncreasesScore);
    RUN_TEST_CASE(PacmanGame, CollectingPowerPelletActivatesPowerMode);
    RUN_TEST_CASE(PacmanGame, GhostCollisionReducesLives);
    RUN_TEST_CASE(PacmanGame, GhostCollisionInFrightenedModeIncreasesScore);
    RUN_TEST_CASE(PacmanGame, NoLivesLeftEndsGame);
    RUN_TEST_CASE(PacmanGame, MovementOnlyOccursAtCorrectInterval);
    RUN_TEST_CASE(PacmanGame, GhostsChasePlayerInChaseMode);
}