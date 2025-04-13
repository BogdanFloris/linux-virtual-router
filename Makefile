CC = clang
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS =

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

BINS = $(BIN_DIR)/router

.PHONY: all clean compiledb

all: dirs $(BINS)

dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -I$(INC_DIR) -c $< -o $@

$(BIN_DIR)/router: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

# Generate compile_commands.json for language servers
compiledb:
	@echo "[" > compile_commands.json
	@for src in $(SRCS); do \
		echo "  {" >> compile_commands.json; \
		echo "    \"directory\": \"$$(pwd)\"," >> compile_commands.json; \
		echo "    \"command\": \"$(CC) $(CFLAGS) -I$(INC_DIR) -c $$src -o $${src/$(SRC_DIR)/$(OBJ_DIR)/.c/.o}\"," >> compile_commands.json; \
		echo "    \"file\": \"$$src\"" >> compile_commands.json; \
		echo "  }$(if $(filter-out $$src,$(lastword $(SRCS))),,)" >> compile_commands.json; \
	done
	@echo "]" >> compile_commands.json

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) compile_commands.json

-include $(DEPS)