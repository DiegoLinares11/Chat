# Nombre del ejecutable
TARGET = server

# Archivos fuente
SRCS = server.c server_threads.c user_manager.c message_handler.c

# Archivos objeto
OBJS = $(SRCS:.c=.o)

# Librerías necesarias
LIBS = -lwebsockets -ljson-c -pthread

# Comando para compilar
CC = gcc

# Flags de compilación
CFLAGS = -Wall -Wextra

# Regla por defecto: compilar todo
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# Compilar .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Ejecutar el servidor en el puerto 9000
run: $(TARGET)
	./$(TARGET) 9000

# Limpiar archivos .o y binario
clean:
	rm -f *.o $(TARGET)