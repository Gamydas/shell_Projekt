CC = gcc 
PRG = shell
OBJ = main.o input.o str.o shellcmd.o


$(PRG): $(OBJ)
	$(CC) -o $(PRG) $(OBJ)

main.o: main.c
	$(CC) -c main.c

input.o: input.c
	$(CC) -c input.c

str.o: str.c
	$(CC) -c str.c

shellcmd.o: shellcmd.c
	$(CC) -c shellcmd.c

clean:
	rm -f $(OBJ) $(PRG)

run:
	./$(PRG)

all: clean $(PRG) run

