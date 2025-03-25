CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lwebsockets -ljson-c -lncurses -lpthread

SRC = client.c network.c message_handler.c ui.c
OBJ = $(SRC:.c=.o)
BIN = client_chat

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
