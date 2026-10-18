# ==============================================================================
# Makefile - SRM IST Campus Navigation & Route Planner
# Compiler: GCC (C11 standard, strict warnings, zero warnings policy)
# ==============================================================================

CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -g
LDFLAGS = 

MODULE_SRCS = graph.c heap.c trie.c hash.c history.c algo.c file.c
MAIN_SRC    = main.c
TEST_SRC    = tests/test_main.c

MODULE_OBJS = $(MODULE_SRCS:.c=.o)
MAIN_OBJ    = $(MAIN_SRC:.c=.o)

TARGET      = campus_nav.exe
TEST_TARGET = test_runner.exe

.PHONY: all clean test sanitize

all: $(TARGET)

$(TARGET): $(MODULE_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(MODULE_OBJS)
	@if not exist tests mkdir tests
	$(CC) $(CFLAGS) -I. $(MODULE_OBJS) $(TEST_SRC) -o $(TEST_TARGET)
	./$(TEST_TARGET)

sanitize: CFLAGS += -fsanitize=address,undefined
sanitize: clean $(TARGET)

clean:
	@if exist *.o del /Q *.o
	@if exist $(TARGET) del /Q $(TARGET)
	@if exist $(TEST_TARGET) del /Q $(TEST_TARGET)
