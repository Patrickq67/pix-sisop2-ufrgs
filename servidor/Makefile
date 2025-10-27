CC = gcc
CFLAGS = -Wall -g
SRC = src/servidor.c src/comunicacaoServidor.c
OBJ = $(SRC:.c=.o)
EXEC = servidor

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

clean:
	rm -f $(OBJ) $(EXEC)
