# Tests/Makefile
CC=gcc
CFLAGS=-I. \
       -I../Core/Inc \
       -I../Tests/Unity \
       -DUNITY_TEST \
       -DDEBUG \
       -Wall \
       -g3

# Source files
UNITY_SRCS=Unity/unity.c Unity/unity_fixture.c

TEST_SRCS=Console_Peripherals/test_joystick.c \
          all_tests.c \
          Mocks/mock_joystick_driver.c

SRC_FILES=../Core/Src/Console_Peripherals/joystick.c

# Object files
UNITY_OBJS=$(UNITY_SRCS:.c=.o)
TEST_OBJS=$(TEST_SRCS:.c=.o)
SRC_OBJS=$(SRC_FILES:.c=.o)

# Output executable
TARGET=run_tests

all: $(TARGET)

$(TARGET): $(UNITY_OBJS) $(TEST_OBJS) $(SRC_OBJS)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(UNITY_OBJS) $(TEST_OBJS) $(SRC_OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run