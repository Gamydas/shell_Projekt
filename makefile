CC = gcc 
PRG = shell
SRC_DIR = src
BUILD_DIR = build
CFLAGS += -I lib/

OBJ = $(addprefix $(BUILD_DIR)/, main.o input.o str.o builtins.o shell.o tab.o parser.o err.o pipelining.o redirect.o exec.o history.o) 

$(PRG): $(OBJ)
	$(CC) -o $(PRG) $(OBJ)

debug: CFLAGS += -g -O0
debug: $(OBJ)
	$(CC) -o debug $(OBJ)

 
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -W -Wextra -Wall  -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
 

TEST_SRC = tests/test_history.c src/history.c src/str.c src/err.c
TEST_BIN = tests/test_history

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC)
	$(CC) $(CFLAGS) -W -Wextra -Wall -o $(TEST_BIN) $(TEST_SRC)

clean:
	rm -f $(OBJ) $(PRG) $(PRG)_debug $(TEST_BIN)

run:
	./$(PRG)

all: clean $(PRG) run

