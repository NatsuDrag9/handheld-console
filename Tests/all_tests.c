#include "./Unity/unity_fixture.h"

static void RunAllTests(void) {
    RUN_TEST_GROUP(Joystick);
    RUN_TEST_GROUP(PushButton);
    RUN_TEST_GROUP(OLED);
}

int main(int argc, const char* argv[]) {
    return UnityMain(argc, argv, RunAllTests);
}
