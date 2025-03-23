#ifndef UI_H
#define UI_H

void init_ui();
void destroy_ui();

void add_message_to_ui(const char *message, const char *sender);
void read_user_input(char *buffer, size_t size);
void update_ui();
void show_help_message();
void update_users_list(char **users, int count);

#endif