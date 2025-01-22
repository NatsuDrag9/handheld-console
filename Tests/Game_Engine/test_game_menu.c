#include "unity.h"
#include "unity_fixture.h"
#include "Game_Engine/game_menu.h"
#include <string.h>

TEST_GROUP(GameMenu);

// Test menu data
static MenuItem test_menu[] = {
    {"Test Snake", 0, SCREEN_GAME_SNAKE, 1},
    {"Test Game 2", 0, SCREEN_GAME_2, 1},
    {"Test Game 3", 0, SCREEN_GAME_3, 1},
    {"Test Game 4", 0, SCREEN_GAME_4, 1},
    {"Test Game 5", 0, SCREEN_GAME_5, 1},
};
static const uint8_t test_menu_size = sizeof(test_menu) / sizeof(test_menu[0]);

TEST_SETUP(GameMenu) {
    // Reset test menu to initial state
    for (uint8_t i = 0; i < test_menu_size; i++) {
        test_menu[i].selected = 0;
    }
    get_game_menu(&menu, &size);
}

TEST_TEAR_DOWN(GameMenu) {
    // No cleanup needed
}

TEST(GameMenu, MenuSizeIsCorrect) {
    TEST_ASSERT_EQUAL_UINT8(MAX_MENU_ITEMS, size);
    TEST_ASSERT_EQUAL_UINT8(test_menu_size, size);
}

TEST(GameMenu, FirstGameIsSnake) {
    TEST_ASSERT_EQUAL_STRING("Test Snake", menu[0].title);
    TEST_ASSERT_EQUAL_UINT8(SCREEN_GAME_SNAKE, menu[0].screen);
    TEST_ASSERT_TRUE(menu[0].is_game);
}

TEST(GameMenu, AllItemsHaveValidTitles) {
    for (uint8_t i = 0; i < size; i++) {
        TEST_ASSERT_NOT_NULL(menu[i].title);
        TEST_ASSERT_TRUE(strlen(menu[i].title) > 0);
    }
}

TEST(GameMenu, AllItemsAreInitiallyUnselected) {
    for (uint8_t i = 0; i < size; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, menu[i].selected);
    }
}

TEST(GameMenu, AllItemsAreGames) {
    for (uint8_t i = 0; i < size; i++) {
        TEST_ASSERT_TRUE(menu[i].is_game);
    }
}

TEST(GameMenu, ScreenTypesAreUnique) {
    // Check that each menu item has a unique screen type
    for (uint8_t i = 0; i < size; i++) {
        for (uint8_t j = i + 1; j < size; j++) {
            TEST_ASSERT_NOT_EQUAL(menu[i].screen, menu[j].screen);
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
    menu[1].selected = 1;
    TEST_ASSERT_EQUAL_UINT8(1, menu[1].selected);
    TEST_ASSERT_EQUAL_UINT8(0, menu[0].selected);  // Other items remain unselected
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