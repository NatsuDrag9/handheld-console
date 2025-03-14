#include "../Unity/unity.h"
#include "../Unity/unity_fixture.h"
#include "../Mocks/Inc/mock_display_driver.h"
#include "Console_Peripherals/oled.h"

TEST_GROUP(OLED);

static MenuItem test_menu[] = {
    {"Test 1", 0},
    {"Test 2", 0},
    {"Test 3", 0}
};

static MockDisplayState display_state;

TEST_SETUP(OLED) {
    mock_display_reset_state();
}

TEST_TEAR_DOWN(OLED) {
    // Clean up test_menu selections
    for (int i = 0; i < 3; i++) {
        test_menu[i].selected = 0;
    }
}

TEST(OLED, InitializationTest) {
    oled_init(test_menu, 3);
    mock_display_get_state(&display_state);
    TEST_ASSERT_EQUAL(1, display_state.initialized);
    TEST_ASSERT_EQUAL(DISPLAY_BLACK, display_state.current_color);
}

TEST(OLED, InitializationWithNullMenuTest) {
    oled_init(NULL, 0);
    mock_display_get_state(&display_state);

    // Check that an error message was displayed
    TEST_ASSERT_EQUAL(1, display_state.error_displayed);
}

TEST(OLED, InitializationWithLargeMenuTest) {
    MenuItem large_menu[10];
    for (int i = 0; i < 10; i++) {
        large_menu[i].title = "Large Menu Item";
        large_menu[i].selected = 0;
    }

    oled_init(large_menu, 10);

    TEST_ASSERT_EQUAL_UINT8(5, oled_get_current_menu_size());
}

TEST(OLED, ClearScreenTest) {
    // First draw something on the screen
    oled_show_screen(SCREEN_WELCOME);
    uint8_t buffer_before[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
    mock_display_get_buffer(buffer_before, sizeof(buffer_before));

    // Verify screen has content
    bool has_content = false;
    for (uint16_t i = 0; i < sizeof(buffer_before); i++) {
        if (buffer_before[i] != 0x00) {
            has_content = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_content);

    // Now clear the screen
    oled_clear_screen();

    // Verify buffer is cleared
    uint8_t buffer_after[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
    mock_display_get_buffer(buffer_after, sizeof(buffer_after));
    for (uint16_t i = 0; i < sizeof(buffer_after); i++) {
        TEST_ASSERT_EQUAL_UINT8(0x00, buffer_after[i]);
    }

    // Verify display state
    mock_display_get_state(&display_state);
    TEST_ASSERT_EQUAL(DISPLAY_BLACK, display_state.current_color);
    TEST_ASSERT_EQUAL(0, display_state.cursor_x);
    TEST_ASSERT_EQUAL(0, display_state.cursor_y);
}

TEST(OLED, ShowWelcomeScreenTest) {
    // Initialize OLED first
    oled_init(test_menu, 3);

    // Get initial state
    mock_display_get_state(&display_state);
    uint8_t initial_updates = display_state.screen_updated;

    // Show welcome screen
    oled_show_screen(SCREEN_WELCOME);
    mock_display_get_state(&display_state);

    // Verify screen structure
    TEST_ASSERT_EQUAL(1, display_state.border_drawn);
    TEST_ASSERT_TRUE(display_state.screen_updated > initial_updates);

    // Verify welcome messages are centered - checking a range
    TEST_ASSERT_GREATER_OR_EQUAL(15, display_state.cursor_x);
    TEST_ASSERT_LESS_OR_EQUAL(DISPLAY_WIDTH - 15, display_state.cursor_x);
}

TEST(OLED, ShowMenuTest) {
    // Initialize OLED first
    oled_init(test_menu, 3);

    // Get initial state
    mock_display_get_state(&display_state);
    uint8_t initial_updates = display_state.screen_updated;

    // Show menu
    oled_show_menu(test_menu, 3);
    mock_display_get_state(&display_state);

    // Verify basic screen structure
    TEST_ASSERT_EQUAL(1, display_state.border_drawn);
    TEST_ASSERT_TRUE(display_state.screen_updated > initial_updates);

    // Verify menu title is centered
    TEST_ASSERT_GREATER_OR_EQUAL(20, display_state.cursor_x);
    TEST_ASSERT_LESS_OR_EQUAL(DISPLAY_WIDTH - 20, display_state.cursor_x);

    // Verify initial cursor position
    TEST_ASSERT_EQUAL(0, oled_get_current_cursor_item());

    // Verify correct number of items displayed
    TEST_ASSERT_EQUAL(3, oled_get_current_menu_size());

    // Verify display properties
    TEST_ASSERT_EQUAL(DISPLAY_WHITE, display_state.current_color);

    // Verify no scrollbar for short menu
    TEST_ASSERT_EQUAL(0, display_state.scrollbar_drawn);
}

TEST(OLED, MenuNavigationDownTest) {
    // Initialize menu
    oled_init(test_menu, 3);
    oled_show_menu(test_menu, 3);
    mock_display_get_state(&display_state);
    uint8_t initial_updates = display_state.screen_updated;

    // Get initial cursor position
    uint8_t initial_cursor = oled_get_current_cursor_item();
    TEST_ASSERT_EQUAL(0, initial_cursor);  // Should start at first item

    // Test down navigation
    JoystickStatus js_down = { JS_DIR_DOWN, 1, 0 };
    oled_menu_handle_input(js_down);
    mock_display_get_state(&display_state);

    // Verify screen was updated
    TEST_ASSERT_TRUE(display_state.screen_updated > initial_updates);

    // Verify cursor moved down
    TEST_ASSERT_EQUAL(1, oled_get_current_cursor_item());

    // Verify cursor is displayed at new position
    // The '>' cursor should now be at the second menu item position
    TEST_ASSERT_GREATER_THAN(initial_cursor, display_state.cursor_y);
}

TEST(OLED, MenuNavigationUpTest) {
    // Initialize menu first
    oled_init(test_menu, 3);
    oled_show_menu(test_menu, 3);

    // First move down
    JoystickStatus js_down = { JS_DIR_DOWN, 1, 0 };
    oled_menu_handle_input(js_down);

    mock_display_get_state(&display_state);
    uint8_t initial_updates = display_state.screen_updated;
    uint8_t initial_cursor = oled_get_current_cursor_item();

    // Then test up navigation
    JoystickStatus js_up = { JS_DIR_UP, 1, 0 };
    oled_menu_handle_input(js_up);
    mock_display_get_state(&display_state);

    // Verify screen was updated
    TEST_ASSERT_TRUE(display_state.screen_updated > initial_updates);

    // Verify cursor moved up
    TEST_ASSERT_LESS_THAN(initial_cursor, oled_get_current_cursor_item());
}

TEST(OLED, MenuSelectionTest) {
    // Initialize menu first
    oled_init(test_menu, 3);

    // Initially no item should be selected
    MenuItem selected = oled_get_selected_menu_item();
    TEST_ASSERT_NULL(selected.title);
    TEST_ASSERT_EQUAL(0, selected.selected);

    // Simulate button press to select first item
    JoystickStatus js_select = { JS_DIR_CENTERED, 1, 1 };
    oled_menu_handle_input(js_select);

    // Get selected item and verify
    selected = oled_get_selected_menu_item();
    TEST_ASSERT_EQUAL_STRING("Test 1", selected.title);
    TEST_ASSERT_EQUAL(1, selected.selected);

    // Move down and select second item
    JoystickStatus js_down = { JS_DIR_DOWN, 1, 0 };
    oled_menu_handle_input(js_down);
    js_select.direction = JS_DIR_CENTERED;
    oled_menu_handle_input(js_select);

    // Verify second item is now selected
    selected = oled_get_selected_menu_item();
    TEST_ASSERT_EQUAL_STRING("Test 2", selected.title);
    TEST_ASSERT_EQUAL(1, selected.selected);
}

TEST(OLED, NoMenuUpdateOnOldInput) {
    // Initialize menu
    oled_init(test_menu, 3);
    oled_show_menu(test_menu, 3);
    mock_display_get_state(&display_state);
    uint8_t initial_updates = display_state.screen_updated;

    // Try with old input
    JoystickStatus js_old = { JS_DIR_DOWN, 0, 0 };  // is_new = 0
    oled_menu_handle_input(js_old);

    // Get updated state
    mock_display_get_state(&display_state);

    // Screen updates should remain the same
    TEST_ASSERT_EQUAL(initial_updates, display_state.screen_updated);

    // Get selected item to verify no change
    MenuItem selected = oled_get_selected_menu_item();
    TEST_ASSERT_NULL(selected.title);
}

TEST(OLED, ScrollbarVisibilityTest) {
    // Test menu shorter than VISIBLE_ITEMS - shouldn't show scrollbar
    MenuItem short_menu[] = {
        {"Item 1", 0},
        {"Item 2", 0}
    };

    oled_init(short_menu, 2);
    oled_show_menu(short_menu, 2);
    mock_display_get_state(&display_state);
    TEST_ASSERT_EQUAL(0, display_state.scrollbar_drawn);

    // Test menu longer than VISIBLE_ITEMS - should show scrollbar
    MenuItem long_menu[] = {
        {"Item 1", 0},
        {"Item 2", 0},
        {"Item 3", 0},
        {"Item 4", 0},
        {"Item 5", 0}
    };

    oled_init(long_menu, 5);
    oled_show_menu(long_menu, 5);
    mock_display_get_state(&display_state);
    TEST_ASSERT_EQUAL(1, display_state.scrollbar_drawn);
}

TEST(OLED, ScrollbarPositionUpdateTest) {
    MenuItem long_menu[] = {
        {"Item 1", 0},
        {"Item 2", 0},
        {"Item 3", 0},
        {"Item 4", 0},
        {"Item 5", 0}
    };

    oled_init(long_menu, 5);

    // Get initial position
    oled_show_menu(long_menu, 5);
    mock_display_get_state(&display_state);
    uint8_t initial_position = display_state.thumb_positions[0];

    // Do multiple scroll downs
    for (int i = 0; i < 3; i++) {
        JoystickStatus js_down = { JS_DIR_DOWN, 1, 0 };
        oled_menu_handle_input(js_down);
        mock_display_get_state(&display_state);
    }

    uint8_t final_position = display_state.thumb_positions[display_state.num_thumb_draws - 1];

    // Test that final position is greater than initial position
    TEST_ASSERT_GREATER_THAN(initial_position, final_position);

    // Or test explicitly 
    TEST_ASSERT_EQUAL(25, initial_position);
    TEST_ASSERT_EQUAL(32, final_position);
}

TEST_GROUP_RUNNER(OLED) {
    RUN_TEST_CASE(OLED, InitializationTest);
    RUN_TEST_CASE(OLED, InitializationWithNullMenuTest);
    RUN_TEST_CASE(OLED, InitializationWithLargeMenuTest);
    RUN_TEST_CASE(OLED, ClearScreenTest);
    RUN_TEST_CASE(OLED, ShowWelcomeScreenTest);
    RUN_TEST_CASE(OLED, ShowMenuTest);
    RUN_TEST_CASE(OLED, MenuNavigationDownTest);
    RUN_TEST_CASE(OLED, MenuNavigationUpTest);
    RUN_TEST_CASE(OLED, MenuSelectionTest);
    RUN_TEST_CASE(OLED, NoMenuUpdateOnOldInput);
    RUN_TEST_CASE(OLED, ScrollbarVisibilityTest);
    RUN_TEST_CASE(OLED, ScrollbarPositionUpdateTest);
}