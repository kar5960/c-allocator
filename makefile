# Compiler and Flags
CC       := gcc
CFLAGS   := -Wall -Wextra -g -O2
INCLUDES := -Iinclude

# Directory Structure
SRC_DIR  := src
TEST_DIR := tests
OBJ_DIR  := obj
BIN      := test_runner

# Source & Object Files
SRCS     := $(SRC_DIR)/core.c $(SRC_DIR)/strategy.c $(TEST_DIR)/test.c
OBJS     := $(SRCS:%.c=$(OBJ_DIR)/%.o)

# Default target
all: $(BIN)

# Link object files into final executable
$(BIN): $(OBJS)
	@echo "Linking $(BIN)..."
	$(CC) $(CFLAGS) $(OBJS) -o $@
	@echo "Build complete!"

# Pattern rule: Compile each .c file into a .o object file
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run test suite
test: $(BIN)
	@echo "Running tests..."
	./$(BIN)

# Run tests under Valgrind
valgrind: $(BIN)
	valgrind --leak-check=full ./$(BIN)

# Run tests under strace for system calls
strace: $(BIN)
	strace -e trace=brk,mmap,munmap ./$(BIN)

# Clean build artifacts
clean:
	@echo "Cleaning up build artifacts..."
	rm -rf $(OBJ_DIR) $(BIN)

# Declare non-file targets
.PHONY: all test valgrind strace clean