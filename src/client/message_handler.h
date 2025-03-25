#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H


#include <ncurses.h>

// Procesa el mensaje recibido del servidor
void handle_server_message(WINDOW *chat_win, WINDOW *users_win, const char *json_str);

#endif

