#include "unity.h"
#include "unity_fixture.h"
#include "Game_Engine/game_menu.h"
#include <string.h>

TEST_GROUP(GameMenu);

// Test menu data
static MenuItem test_menu[] = {
    {"Test Snake", 0, SCREEN_GAME_SNAKE, 1},
    {"Test Game 2", 0, SCREEN_GAME_PACMAN, 1},
    {"Test Game 3", 0, SCREEN_GAME_3, 1},
    {"Test Game 4", 0, SCREEN_GAME_4, 1},
    {"Test Game 5", 0, SCREEN_GAME_5, 1},
};
static const uint8_t test_menu_size = sizeof(test_menu) / sizeof(test_menu[0]);

// Variables to store menu returned by get_game_menu
static MenuItem* menu_ptr;
static uint8_t menu_size;

TEST_SETUP(GameMenu) {
    // Reset test menu to initial state
    for (uint8_t i = 0; i < test_menu_size; i++) {
        test_menu[i].selected = 0;
    }
    get_game_menu(&menu_ptr, &menu_size);
}

TEST_TEAR_DOWN(GameMenu) {
    // No cleanup needed
}

TEST(GameMenu, MenuSizeIsCorrect) {
    TEST_ASSERT_EQUAL_UINT8(MAX_MENU_ITEMS, menu_size);
    TEST_ASSERT_EQUAL_UINT8(test_menu_size, menu_size);
}

TEST(GameMenu, FirstGameIsSnake) {
    TEST_ASSERT_EQUAL_STRING("Snake Game", menu_ptr[0].title);
    TEST_ASSERT_EQUAL_UINT8(SCREEN_GAME_SNAKE, menu_ptr[0].screen);
    TEST_ASSERT_TRUE(menu_ptr[0].is_game);
}

TEST(GameMenu, AllItemsHaveValidTitles) {
    for (uint8_t i = 0; i < menu_size; i++) {
        TEST_ASSERT_NOT_NULL(menu_ptr[i].title);
        TEST_ASSERT_TRUE(strlen(menu_ptr[i].title) > 0);
    }
}

TEST(GameMenu, AllItemsAreInitiallyUnselected) {
    for (uint8_t i = 0; i < menu_size; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, menu_ptr[i].selected);
    }
}

TEST(GameMenu, AllItemsAreGames) {
    for (uint8_t i = 0; i < menu_size; i++) {
        TEST_ASSERT_TRUE(menu_ptr[i].is_game);
    }
}

TEST(GameMenu, ScreenTypesAreUnique) {
    // Check that each menu item has a unique screen type
    for (uint8_t i = 0; i < menu_size; i++) {
        for (uint8_t j = i + 1; j < menu_size; j++) {
            TEST_ASSERT_NOT_EQUAL(menu_ptr[i].screen, menu_ptr[j].screen);
        }
    }
}

TEST(GameMenu, NullPointerHandling) {
    MenuItem* null_menu = NULL;
    uint8_t null_size = 0;

    // Should not crash with NULL pointers
    get_game_menu(NULL, NULL);
    get_game_menu(&null_menu, NULL);
    get_game_menu(NULL, &null_size);

    TEST_ASSERT_NULL(null_menu);
    TEST_ASSERT_EQUAL_UINT8(0, null_size);
}

TEST(GameMenu, CanSelectMenuItem) {
    // Select second menu item
    menu_ptr[1].selected = 1;
    TEST_ASSERT_EQUAL_UINT8(1, menu_ptr[1].selected);
    TEST_ASSERT_EQUAL_UINT8(0, menu_ptr[0].selected);  // Other items remain unselected
}

// Test Group Runner
TEST_GROUP_RUNNER(GameMenu) {
    RUN_TEST_CASE(GameMenu, MenuSizeIsCorrect);
    RUN_TEST_CASE(GameMenu, FirstGameIsSnake);
    RUN_TEST_CASE(GameMenu, AllItemsHaveValidTitles);
    RUN_TEST_CASE(GameMenu, AllItemsAreInitiallyUnselected);
    RUN_TEST_CASE(GameMenu, AllItemsAreGames);
    RUN_TEST_CASE(GameMenu, ScreenTypesAreUnique);
    RUN_TEST_CASE(GameMenu, NullPointerHandling);
    RUN_TEST_CASE(GameMenu, CanSelectMenuItem);
}