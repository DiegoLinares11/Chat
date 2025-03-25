#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <json-c/json.h>

// Inicializar y finalizar interfaz
void init_ui(WINDOW **chat, WINDOW **input, WINDOW **users);
void end_ui(void);

// Mensajes
void add_message_to_ui(WINDOW *win, const char *msg);

// Entrada de usuario
void get_input_line(WINDOW *input_win, char *buffer, size_t size);

// Actualizar lista de usuarios
void update_user_list(WINDOW *users_win, struct json_object *user_array);

#endif