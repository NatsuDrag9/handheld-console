#include "./Unity/unity_fixture.h"

static void RunAllTests(void) {
    RUN_TEST_GROUP(Joystick);
    RUN_TEST_GROUP(PushButton);
    RUN_TEST_GROUP(OLED);
    RUN_TEST_GROUP(GameMenu);
    RUN_TEST_GROUP(GameEngine);
    RUN_TEST_GROUP(Sprite);
}

int main(int argc, const char* argv[]) {
    return UnityMain(argc, argv, RunAllTests);
}
