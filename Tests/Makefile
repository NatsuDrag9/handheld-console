# Tests/Makefile
CC=gcc
CFLAGS=-I. \
       -I../Core/Inc \
       -I../Tests/Unity \
       -I../Tests/Mocks/Inc \
       -DUNITY_TEST \
       -DDEBUG \
       -Wall \
       -g3

# Source files
UNITY_SRCS=Unity/unity.c Unity/unity_fixture.c

# TEST_SRCS=Console_Peripherals/test_joystick.c \
#           Console_Peripherals/test_push_button.c \
#           all_tests.c

# MOCK_SRCS=Mocks/Src/mock_joystick_driver.c \
#           Mocks/Src/mock_push_button_driver.c

TEST_SRCS=$(wildcard Console_Peripherals/test_*.c) \
       all_tests.c

MOCK_SRCS=$(wildcard Mocks/Src/mock_*.c)

SRC_FILES=../Core/Src/Console_Peripherals/joystick.c \
          ../Core/Src/Console_Peripherals/push_button.c

# Object files
UNITY_OBJS=$(UNITY_SRCS:.c=.o)
TEST_OBJS=$(TEST_SRCS:.c=.o)
MOCK_OBJS=$(MOCK_SRCS:.c=.o)
SRC_OBJS=$(SRC_FILES:.c=.o)

# Output executable
TARGET=run_tests

all: $(TARGET)

$(TARGET): $(UNITY_OBJS) $(TEST_OBJS) $(MOCK_OBJS) $(SRC_OBJS)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(UNITY_OBJS) $(TEST_OBJS) $(MOCK_OBJS) $(SRC_OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run