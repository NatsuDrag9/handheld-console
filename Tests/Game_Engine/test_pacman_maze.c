#include "unity.h"
#include "unity_fixture.h"
#include "Game_Engine/Games/pacman_maze.h"
#include "Mocks/Inc/mock_utils.h"
#include "Mocks/Inc/mock_display_driver.h"

TEST_GROUP(PacmanGameMaze);

TEST_SETUP(PacmanGameMaze) {
}

TEST_TEAR_DOWN(PacmanGameMaze) {
}

// Test screen to maze X coordinate conversion
TEST(PacmanGameMaze, ScreenToMazeXValidInput) {
    // Given BORDER_OFFSET = 8, TILE_SIZE = 8
    // Test exact tile boundaries
    TEST_ASSERT_EQUAL_INT(0, screen_to_maze_x(8));   // First tile
    TEST_ASSERT_EQUAL_INT(0, screen_to_maze_x(15));  // Still in first tile
    TEST_ASSERT_EQUAL_INT(1, screen_to_maze_x(16));  // Second tile
    TEST_ASSERT_EQUAL_INT(1, screen_to_maze_x(23));  // Still in second tile
}

TEST(PacmanGameMaze, ScreenToMazeXEdgeCases) {
    // Test boundary conditions
    TEST_ASSERT_EQUAL_INT(0, screen_to_maze_x(7));  // Before border, should map to first tile
    TEST_ASSERT_EQUAL_INT(13, screen_to_maze_x(119)); // Last valid tile
}

// Test screen to maze Y coordinate conversion
TEST(PacmanGameMaze, ScreenToMazeYValidInput) {
    // Given GAME_AREA_TOP = 12, TILE_SIZE = 8
    // Test exact tile boundaries
    TEST_ASSERT_EQUAL_INT(0, screen_to_maze_y(12));  // First tile
    TEST_ASSERT_EQUAL_INT(0, screen_to_maze_y(19));  // Still in first tile
    TEST_ASSERT_EQUAL_INT(1, screen_to_maze_y(20));  // Second tile
    TEST_ASSERT_EQUAL_INT(1, screen_to_maze_y(27));  // Still in second tile
}

TEST(PacmanGameMaze, ScreenToMazeYEdgeCases) {
    // Test boundary conditions
    TEST_ASSERT_EQUAL_INT(0, screen_to_maze_y(11)); // Before game area, should map to first tile
    TEST_ASSERT_EQUAL_INT(5, screen_to_maze_y(52));  // Last valid tile
}

// Test maze to screen X coordinate conversion
TEST(PacmanGameMaze, MazeToScreenXValidInput) {
    // Test conversions for different maze positions
    TEST_ASSERT_EQUAL_UINT8(8, maze_to_screen_x(0));   // First column
    TEST_ASSERT_EQUAL_UINT8(16, maze_to_screen_x(1));  // Second column
    TEST_ASSERT_EQUAL_UINT8(24, maze_to_screen_x(2));  // Third column
    TEST_ASSERT_EQUAL_UINT8(120, maze_to_screen_x(14)); // Last column
}

// Test maze to screen Y coordinate conversion
TEST(PacmanGameMaze, MazeToScreenYValidInput) {
    // Test conversions for different maze rows
    TEST_ASSERT_EQUAL_UINT8(12, maze_to_screen_y(0));  // First row
    TEST_ASSERT_EQUAL_UINT8(20, maze_to_screen_y(1));  // Second row
    TEST_ASSERT_EQUAL_UINT8(28, maze_to_screen_y(2));  // Third row
    TEST_ASSERT_EQUAL_UINT8(36, maze_to_screen_y(3));  // Fourth row
    TEST_ASSERT_EQUAL_UINT8(44, maze_to_screen_y(4));  // Last row
    TEST_ASSERT_EQUAL_UINT8(52, maze_to_screen_y(5));  // Clamped to last row despite input exceeding MAZE_HEIGHT
}

// Test wall detection
TEST(PacmanGameMaze, IsWallValidPositions) {
    // Test known wall positions from MAZE_LAYOUT
    TEST_ASSERT_TRUE(is_wall(8, 12));    // Top-left corner (1,1)
    TEST_ASSERT_FALSE(is_wall(64, 12));  // Top-middle path (0)
    TEST_ASSERT_TRUE(is_wall(120, 12));  // Top-right corner (1)
}

TEST(PacmanGameMaze, IsWallOutOfBounds) {
    // Test out of bounds positions
    TEST_ASSERT_TRUE(is_wall(0, 0));     // Before game area
    TEST_ASSERT_TRUE(is_wall(128, 64));  // After game area
}

TEST(PacmanGameMaze, RoundTripConversion) {
    // Test that converting from screen to maze and back gives original value
    for (uint8_t x = 0; x < MAZE_WIDTH; x++) {
        uint8_t screen_x = maze_to_screen_x(x);
        int maze_x = screen_to_maze_x(screen_x);
        TEST_ASSERT_EQUAL_INT(x, maze_x);
    }

    for (uint8_t y = 0; y < MAZE_HEIGHT; y++) {
        uint8_t screen_y = maze_to_screen_y(y);
        int maze_y = screen_to_maze_y(screen_y);
        TEST_ASSERT_EQUAL_INT(y, maze_y);
    }
}

TEST_GROUP_RUNNER(PacmanGameMaze) {
    RUN_TEST_CASE(PacmanGameMaze, ScreenToMazeXValidInput);
    RUN_TEST_CASE(PacmanGameMaze, ScreenToMazeXEdgeCases);
    RUN_TEST_CASE(PacmanGameMaze, ScreenToMazeYValidInput);
    RUN_TEST_CASE(PacmanGameMaze, ScreenToMazeYEdgeCases);
    RUN_TEST_CASE(PacmanGameMaze, MazeToScreenXValidInput);
    RUN_TEST_CASE(PacmanGameMaze, MazeToScreenYValidInput);
    RUN_TEST_CASE(PacmanGameMaze, IsWallValidPositions);
    RUN_TEST_CASE(PacmanGameMaze, IsWallOutOfBounds);
    RUN_TEST_CASE(PacmanGameMaze, RoundTripConversion);
}