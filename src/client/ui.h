#ifndef UI_H
#define UI_H

#include <ncurses.h>

void init_ui(WINDOW **chat_win, WINDOW **input_win, WINDOW **users_win);
void end_ui();
void get_input_line(WINDOW *win, char *buffer, int max_len);
void add_message_to_ui(WINDOW *win, const char *msg);
void clear_users_ui(WINDOW *win);
void add_user_to_ui(WINDOW *win, const char *user);

#endif