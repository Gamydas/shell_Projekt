CC = gcc 
PRG = shell
SRC_DIR = src
BUILD_DIR = build
CFLAGS += -I lib/

OBJ = $(addprefix $(BUILD_DIR)/, main.o input.o str.o builtins.o shell.o tab.o parser.o err.o pipelining.o redirect.o exec.o)

$(PRG): $(OBJ)
	$(CC) -o $(PRG) $(OBJ)

debug: CFLAGS += -g -O0
debug: $(OBJ)
	$(CC) -o debug $(OBJ)

 
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -W -Wextra -Wall -Werror -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
 

clean:
	rm -f $(OBJ) $(PRG) $(PRG)_debug

run:
	./$(PRG)

all: clean $(PRG) run

