// message_handler_ui.c
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "ui.h"

void handle_server_message(WINDOW *chat_win, WINDOW *users_win, const char *json_str) {
    struct json_object *msg = json_tokener_parse(json_str);
    if (!msg) {
        add_message_to_ui(chat_win, "[ERROR] Mensaje JSON inválido.");
        return;
    }

    const char *type = json_object_get_string(json_object_object_get(msg, "type"));
    const char *sender = json_object_get_string(json_object_object_get(msg, "sender"));
    const char *target = json_object_get_string(json_object_object_get(msg, "target"));
    struct json_object *content_obj = json_object_object_get(msg, "content");

    if (strcmp(type, "register_success") == 0) {
        add_message_to_ui(chat_win, "✅ Registro exitoso.");
    } else if (strcmp(type, "broadcast") == 0) {
        const char *text = json_object_get_string(content_obj);
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "[%s]: %s", sender, text);
        add_message_to_ui(chat_win, buffer);
    } else if (strcmp(type, "private") == 0) {
        const char *text = json_object_get_string(content_obj);
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "[Privado de %s]: %s", sender, text);
        add_message_to_ui(chat_win, buffer);
    } else if (strcmp(type, "list_users_response") == 0) {
        if (json_object_is_type(content_obj, json_type_array)) {
            int len = json_object_array_length(content_obj);
            clear_users_ui(users_win);
            add_message_to_ui(chat_win, "Usuarios conectados:");
            for (int i = 0; i < len; i++) {
                const char *user = json_object_get_string(json_object_array_get_idx(content_obj, i));
                add_user_to_ui(users_win, user);
                add_message_to_ui(chat_win, user);
            }
        }
    } else if (strcmp(type, "user_info_response") == 0) {
        if (json_object_is_type(content_obj, json_type_object)) {
            const char *ip = json_object_get_string(json_object_object_get(content_obj, "ip"));
            const char *status = json_object_get_string(json_object_object_get(content_obj, "status"));
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Info de %s -> IP: %s, Estado: %s", target, ip, status);
            add_message_to_ui(chat_win, buffer);
        }
    } else if (strcmp(type, "status_update") == 0) {
        if (json_object_is_type(content_obj, json_type_object)) {
            const char *user = json_object_get_string(json_object_object_get(content_obj, "user"));
            const char *new_status = json_object_get_string(json_object_object_get(content_obj, "status"));
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "[%s cambió su estado a: %s]", user, new_status);
            add_message_to_ui(chat_win, buffer);
        }
    } else if (strcmp(type, "user_disconnected") == 0) {
        const char *text = json_object_get_string(content_obj);
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "[%s]", text);
        add_message_to_ui(chat_win, buffer);
    } else if (strcmp(type, "error") == 0) {
        const char *text = json_object_get_string(content_obj);
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "[ERROR]: %s", text);
        add_message_to_ui(chat_win, buffer);
    }

    json_object_put(msg);
}