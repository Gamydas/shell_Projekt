CC = gcc 
PRG = shell
OBJ = main.o input.o str.o shellcmd.o tools.o list.o


$(PRG): $(OBJ)
	$(CC) -o $(PRG) $(OBJ)

shell_debug: CFLAGS = -g -O0
shell_debug: $(OBJ)
	$(CC) -o $(PRG)_debug $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(PRG) $(PRG)_debug

run:
	./$(PRG)

all: clean $(PRG) run

