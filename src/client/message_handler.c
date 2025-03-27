#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "context.h"

void handle_server_message(void *unused1, void *unused2, const char *json_str) {
    struct json_object *msg = json_tokener_parse(json_str);
    if (!msg) {
        printf("\r\033[K[ERROR] Mensaje JSON inválido.\n> ");
        fflush(stdout);
        return;
    }

    const char *type = json_object_get_string(json_object_object_get(msg, "type"));
    const char *sender = json_object_get_string(json_object_object_get(msg, "sender"));
    const char *target = json_object_get_string(json_object_object_get(msg, "target"));
    struct json_object *content_obj = json_object_object_get(msg, "content");

    if (strcmp(type, "register_success") == 0) {
        printf("\r\033[K✅ Registro exitoso.\n");
    } else if (strcmp(type, "broadcast") == 0) {
        const char *text = json_object_get_string(content_obj);
        printf("\r\033[K[%s]: %s\n", sender, text);
    } else if (strcmp(type, "private") == 0) {
        const char *text = json_object_get_string(content_obj);
        printf("\r\033[K[Privado de %s]: %s\n", sender, text);
    } else if (strcmp(type, "list_users_response") == 0) {
        if (json_object_is_type(content_obj, json_type_array)) {
            int len = json_object_array_length(content_obj);
            printf("\r\033[KUsuarios conectados:\n");
            for (int i = 0; i < len; i++) {
                const char *user = json_object_get_string(json_object_array_get_idx(content_obj, i));
                printf(" - %s\n", user);
            }
        }
    } else if (strcmp(type, "user_info_response") == 0) {
        if (json_object_is_type(content_obj, json_type_object)) {
            const char *ip = json_object_get_string(json_object_object_get(content_obj, "ip"));
            const char *status = json_object_get_string(json_object_object_get(content_obj, "status"));
            printf("\r\033[KInfo de %s -> IP: %s, Estado: %s\n", target, ip, status);
        }
    } else if (strcmp(type, "status_update") == 0) {
        if (json_object_is_type(content_obj, json_type_object)) {
            const char *user = json_object_get_string(json_object_object_get(content_obj, "user"));
            const char *new_status = json_object_get_string(json_object_object_get(content_obj, "status"));
            if (strcmp(user, ctx.username) != 0) {
                printf("\r\033[K[%s cambió su estado a: %s]\n", user, new_status);
            }
        }
    } else if (strcmp(type, "user_disconnected") == 0) {
        const char *text = json_object_get_string(content_obj);
        printf("\r\033[K[Desconectado] %s\n", text);
    } else if (strcmp(type, "error") == 0) {
        const char *text = json_object_get_string(content_obj);
        printf("\r\033[K[ERROR]: %s\n", text);
    }

    printf("> ");
    fflush(stdout);
    json_object_put(msg);
}