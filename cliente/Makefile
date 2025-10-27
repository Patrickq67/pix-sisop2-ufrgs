CC = gcc
CFLAGS = -Wall -g
SRC = src/cliente.c src/comunicacaoCliente.c
OBJ = $(SRC:.c=.o)
EXEC = cliente

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

clean:
	rm -f $(OBJ) $(EXEC)
