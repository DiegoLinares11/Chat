#include <ncurses.h>
#include <string.h>
#include "ui.h"

#define MAX_CHAT_LINES 100
#define MAX_LINE_LENGTH 256

static WINDOW *chat_win, *input_win, *users_win;
static char chat_lines[MAX_CHAT_LINES][MAX_LINE_LENGTH];
static int chat_line_count = 0;

// Inicializar interfaz
void init_ui(WINDOW **chat, WINDOW **input, WINDOW **users) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int users_width = 25;
    *chat = newwin(rows - 4, cols - users_width, 0, 0);
    *input = newwin(4, cols - users_width, rows - 4, 0);
    *users = newwin(rows, users_width, 0, cols - users_width);

    chat_win = *chat;
    input_win = *input;
    users_win = *users;

    scrollok(chat_win, TRUE);
    box(input_win, 0, 0);
    box(users_win, 0, 0);
    mvwprintw(users_win, 0, 1, " Usuarios ");
    wrefresh(users_win);
}

// Finalizar interfaz
void end_ui() {
    endwin();
}

// Mostrar línea en ventana de chat
void add_message_to_ui(WINDOW *win, const char *msg) {
    if (chat_line_count < MAX_CHAT_LINES) {
        strncpy(chat_lines[chat_line_count++], msg, MAX_LINE_LENGTH - 1);
    } else {
        // Desplazar hacia arriba
        for (int i = 1; i < MAX_CHAT_LINES; i++) {
            strncpy(chat_lines[i - 1], chat_lines[i], MAX_LINE_LENGTH - 1);
        }
        strncpy(chat_lines[MAX_CHAT_LINES - 1], msg, MAX_LINE_LENGTH - 1);
    }

    werase(chat_win);
    box(chat_win, 0, 0);
    for (int i = 0; i < chat_line_count; i++) {
        mvwprintw(chat_win, i + 1, 1, "%s", chat_lines[i]);
    }
    wrefresh(chat_win);
}

// Obtener línea escrita por el usuario
void get_input_line(WINDOW *input_win, char *buffer, size_t size) {
    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "> ");
    wrefresh(input_win);

    echo();
    wgetnstr(input_win, buffer, size - 1);
    noecho();
}

// Mostrar lista de usuarios conectados
void update_user_list(WINDOW *users_win, struct json_object *user_array) {
    int len = json_object_array_length(user_array);
    werase(users_win);
    box(users_win, 0, 0);
    mvwprintw(users_win, 0, 1, " Usuarios ");

    for (int i = 0; i < len && i < getmaxy(users_win) - 2; i++) {
        const char *username = json_object_get_string(json_object_array_get_idx(user_array, i));
        mvwprintw(users_win, i + 1, 1, "%s", username);
    }

    wrefresh(users_win);
}
