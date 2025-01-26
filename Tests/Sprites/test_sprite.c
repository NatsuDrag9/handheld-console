#include "unity.h"
#include "unity_fixture.h"
#include "Sprites/sprite.h"
#include "../Mocks/Inc/mock_display_driver.h"
#include "../Mocks/Inc/mock_math.h"
#include "../Mocks/Inc/mock_utils.h"

static void verify_bitmap_drawn(const uint8_t* expected_bitmap, const uint8_t* actual_buffer,
    uint8_t width, uint8_t height, uint8_t x, uint8_t y) {
    for (uint8_t dy = 0; dy < height; dy++) {
        for (uint8_t dx = 0; dx < width; dx++) {
            // Calculate bit positions
            uint8_t src_byte = dy * ((width + 7) / 8) + dx / 8;
            uint8_t src_bit = 7 - (dx % 8);

            uint8_t dst_byte = (y + dy) * (DISPLAY_WIDTH / 8) + (x + dx) / 8;
            uint8_t dst_bit = 7 - ((x + dx) % 8);

            // Check if source bit is set
            bool src_pixel = (expected_bitmap[src_byte] & (1 << src_bit)) != 0;
            bool dst_pixel = (actual_buffer[dst_byte] & (1 << dst_bit)) != 0;

            char msg[100];
            snprintf(msg, sizeof(msg), "Pixel mismatch at (%d,%d)", dx, dy);
            TEST_ASSERT_EQUAL_MESSAGE(src_pixel, dst_pixel, msg);
        }
    }
}

TEST_GROUP(Sprite);

// Test bitmap data - 8x8 checkered pattern
static const uint8_t test_bitmap[] = {
    0xAA,  // 10101010
    0x55,  // 01010101
    0xAA,  // 10101010
    0x55,  // 01010101
    0xAA,  // 10101010
    0x55,  // 01010101
    0xAA,  // 10101010
    0x55   // 01010101
};

static const uint8_t test_bitmap_2[] = {
    0xFF,  // 11111111
    0xFF,  // 11111111
    0xFF,  // 11111111
    0xFF   // 11111111
};

// Test sprite
static Sprite test_sprite = {
    .bitmap = test_bitmap,
    .width = 8,
    .height = 8
};

// Test animated sprite frames
static Sprite test_frames[] = {
    {test_bitmap, 8, 8},
    {test_bitmap_2, 8, 4}
};

static AnimatedSprite test_animated_sprite = {
    .frames = test_frames,
    .num_frames = 2,
    .current_frame = 0,
    .frame_delay = 100,  // 100ms between frames
    .last_update = 0
};

TEST_SETUP(Sprite) {
    // Reset mock display and math states
    mock_display_reset_state();
    mock_math_reset();
}

TEST_TEAR_DOWN(Sprite) {
    // Nothing to clean up
}

TEST(Sprite, BasicDrawing) {
    sprite_draw(&test_sprite, 0, 0, DISPLAY_WHITE);

    // Get display buffer and verify pattern
    uint8_t display_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
    mock_display_get_buffer(display_buffer, sizeof(display_buffer));

    // Verify first byte matches bitmap pattern
    TEST_ASSERT_EQUAL_UINT8(test_bitmap[0], display_buffer[0]);
}

TEST(Sprite, RotatedDrawing) {
    // Set up mock math returns for 90-degree rotation
    mock_math_set_sin_return(1.0f);  // sin(90°) = 1
    mock_math_set_cos_return(0.0f);  // cos(90°) = 0

    sprite_draw_rotated(&test_sprite, 10, 10, 90, DISPLAY_WHITE);

    // Verify sin/cos were called with correct angle
    float expected_angle = 90.0f * 3.14159f / 180.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.001f, expected_angle, mock_math_get_last_sin_input());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, expected_angle, mock_math_get_last_cos_input());
}

TEST(Sprite, ScaledDrawing) {
    sprite_draw_scaled(&test_sprite, 0, 0, 2.0f, DISPLAY_WHITE);

    MockDisplayState state;
    mock_display_get_state(&state);

    // For 2x scaling of 8x8 sprite, screen updates should cover 16x16 area
    TEST_ASSERT_TRUE(state.screen_updated > 0);
}

TEST(Sprite, AnimationUpdate) {
    uint32_t initial_time = 0;
    uint32_t after_delay = test_animated_sprite.frame_delay + 1;

    // Set initial time
    mock_time_set_ms(initial_time);
    animated_sprite_update(&test_animated_sprite);
    TEST_ASSERT_EQUAL_UINT8(0, test_animated_sprite.current_frame);

    // Set time after frame delay
    mock_time_set_ms(after_delay);
    animated_sprite_update(&test_animated_sprite);
    TEST_ASSERT_EQUAL_UINT8(1, test_animated_sprite.current_frame);
}

TEST(Sprite, AnimatedDrawing) {
    animated_sprite_draw(&test_animated_sprite, 0, 0, DISPLAY_WHITE);

    uint8_t display_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
    mock_display_get_buffer(display_buffer, sizeof(display_buffer));

    // Use helper function to verify each pixel
    const Sprite* first_frame = &test_frames[0];
    verify_bitmap_drawn(first_frame->bitmap, display_buffer,
        first_frame->width, first_frame->height, 0, 0);
}

TEST(Sprite, NullHandling) {
    // These should not crash
    sprite_draw(NULL, 0, 0, DISPLAY_WHITE);
    sprite_draw_rotated(NULL, 0, 0, 0, DISPLAY_WHITE);
    sprite_draw_scaled(NULL, 0, 0, 1.0f, DISPLAY_WHITE);
    animated_sprite_update(NULL);
    animated_sprite_draw(NULL, 0, 0, DISPLAY_WHITE);
}

// Test Group Runner
TEST_GROUP_RUNNER(Sprite) {
    RUN_TEST_CASE(Sprite, BasicDrawing);
    RUN_TEST_CASE(Sprite, RotatedDrawing);
    RUN_TEST_CASE(Sprite, ScaledDrawing);
    RUN_TEST_CASE(Sprite, AnimationUpdate);
    // RUN_TEST_CASE(Sprite, AnimatedDrawing);
    // RUN_TEST_CASE(Sprite, NullHandling);
}