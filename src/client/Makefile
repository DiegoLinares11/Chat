CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags libwebsockets json-c ncurses`
LDFLAGS = `pkg-config --libs libwebsockets json-c ncurses`

SRC = client.c \
      network.c \
      message_handler.c \
      user_input.c \
      ui.c

OBJ = $(SRC:.c=.o)
TARGET = chat_client

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)