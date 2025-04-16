CC = clang
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS =

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include
TEST_DIR = test

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_SRCS))

BINS = $(BIN_DIR)/router

.PHONY: all clean compiledb test

all: dirs $(BINS)

test: dirs $(TEST_BINS)
	@echo "Running tests..."
	@for test in $(TEST_BINS); do \
		echo "Running $$test"; \
		$$test || exit 1; \
		echo ""; \
	done
	@echo "All tests passed!"

dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -I$(INC_DIR) -c $< -o $@

# Main binary
$(BIN_DIR)/router: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

# Test binaries (exclude main.o to avoid duplicate main functions)
$(BIN_DIR)/test_%: $(TEST_DIR)/test_%.c $(filter-out $(OBJ_DIR)/main.o,$(OBJS))
	$(CC) $(CFLAGS) -I$(INC_DIR) $^ -o $@

# Generate compile_commands.json for language servers
compiledb:
	@echo "[" > compile_commands.json
	@for src in $(SRCS) $(TEST_SRCS); do \
		echo "  {" >> compile_commands.json; \
		echo "    \"directory\": \"$$(pwd)\"," >> compile_commands.json; \
		echo "    \"command\": \"$(CC) $(CFLAGS) -I$(INC_DIR) -c $$src -o obj/$$(basename $$src .c).o\"," >> compile_commands.json; \
		echo "    \"file\": \"$$src\"" >> compile_commands.json; \
		if [ "$$src" != "$$(echo $(SRCS) $(TEST_SRCS) | tr ' ' '\n' | tail -1)" ]; then \
			echo "  }," >> compile_commands.json; \
		else \
			echo "  }" >> compile_commands.json; \
		fi; \
	done
	@echo "]" >> compile_commands.json

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) compile_commands.json

-include $(DEPS)
