// Tests/all_tests.c
#include "./Unity/unity_fixture.h"

static void RunAllTests(void) {
    RUN_TEST_GROUP(Joystick);
}

int main(int argc, const char* argv[]) {
    return UnityMain(argc, argv, RunAllTests);
}
