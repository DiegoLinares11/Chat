#include <stdio.h>
#include <string.h>
#include "ui.h"

void init_ui() {
    printf("Cliente iniciado (modo consola)\n");
}

void destroy_ui() {
    printf("Cliente finalizado.\n");
}

void add_message_to_ui(const char *message, const char *sender) {
    if (sender)
        printf("[%s] %s\n", sender, message);
    else
        printf("%s\n", message);
}

void read_user_input(char *buffer, size_t size) {
    printf("> ");
    fflush(stdout);
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

void update_ui() {
    // Nada que hacer en modo consola
}

void show_help_message() {
    printf("Comandos disponibles:\n");
    printf("/help - Mostrar ayuda\n");
    printf("/list - Listar usuarios\n");
    printf("/info <usuario> - Información de un usuario\n");
    printf("/status <estado> - Cambiar estado (ACTIVO, OCUPADO, INACTIVO)\n");
    printf("/quit - Salir\n");
    printf("@usuario mensaje - Mensaje privado\n");
    printf("mensaje - Mensaje público (broadcast)\n");
}

void update_users_list(char **users, int count) {
    printf("Usuarios conectados:\n");
    for (int i = 0; i < count; i++) {
        printf("- %s\n", users[i]);
    }
}