#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "message_handler.h"
#include "client_context.h"
#include "ui.h"
#include "../common/protocol.h" // ✅ Agregado

void process_incoming_message(const char *json_string) {
    struct json_object *parsed = json_tokener_parse(json_string);
    if (!parsed) {
        add_message_to_ui("Error al parsear mensaje JSON", NULL);
        return;
    }

    const char *type = NULL;
    const char *sender = NULL;
    const char *content = NULL;
    const char *target = NULL;
    struct json_object *jvalue;

    if (json_object_object_get_ex(parsed, "type", &jvalue))
        type = json_object_get_string(jvalue);
    if (json_object_object_get_ex(parsed, "sender", &jvalue))
        sender = json_object_get_string(jvalue);
    if (json_object_object_get_ex(parsed, "content", &jvalue))
        content = json_object_is_type(jvalue, json_type_string) ? json_object_get_string(jvalue) : NULL;
    if (json_object_object_get_ex(parsed, "target", &jvalue))
        target = json_object_get_string(jvalue);

    if (!type) {
        add_message_to_ui("Mensaje inválido: sin campo 'type'", NULL);
        json_object_put(parsed);
        return;
    }

    if (strcmp(type, TYPE_BROADCAST) == 0 || strcmp(type, TYPE_PRIVATE) == 0) {
        if (sender && content) {
            add_message_to_ui(content, sender);
        }
    } else if (strcmp(type, TYPE_REGISTER_SUCCESS) == 0) {
        add_message_to_ui("Registro exitoso en el servidor", NULL);

        if (json_object_object_get_ex(parsed, "userList", &jvalue)) {
            int n = json_object_array_length(jvalue);
            char *usernames[n];
            for (int i = 0; i < n; i++) {
                struct json_object *juser = json_object_array_get_idx(jvalue, i);
                usernames[i] = (char *)json_object_get_string(juser);
            }
            update_users_list(usernames, n);
        }
    } else if (strcmp(type, TYPE_LIST_USERS_RESPONSE) == 0) {
        if (json_object_object_get_ex(parsed, "content", &jvalue)) {
            int n = json_object_array_length(jvalue);
            char *usernames[n];
            for (int i = 0; i < n; i++) {
                struct json_object *juser = json_object_array_get_idx(jvalue, i);
                usernames[i] = (char *)json_object_get_string(juser);
            }
            update_users_list(usernames, n);
            add_message_to_ui("Lista de usuarios actualizada", NULL);
        }
    } else if (strcmp(type, TYPE_USER_INFO_RESPONSE) == 0) {
        if (target && content) {
            struct json_object *info = json_object_object_get(parsed, "content");
            if (info) {
                const char *ip = NULL, *status = NULL;
                struct json_object *ip_obj, *status_obj;
                if (json_object_object_get_ex(info, "ip", &ip_obj))
                    ip = json_object_get_string(ip_obj);
                if (json_object_object_get_ex(info, "status", &status_obj))
                    status = json_object_get_string(status_obj);

                char formatted[256];
                snprintf(formatted, sizeof(formatted), "Usuario: %s | IP: %s | Estado: %s", target, ip, status);
                add_message_to_ui(formatted, "info");
            }
        }
    } else if (strcmp(type, TYPE_STATUS_UPDATE) == 0) {
        if (content) {
            struct json_object *user_obj, *status_obj;
            if (json_object_object_get_ex(parsed, "content", &jvalue)) {
                if (json_object_object_get_ex(jvalue, "user", &user_obj) &&
                    json_object_object_get_ex(jvalue, "status", &status_obj)) {
                    const char *user = json_object_get_string(user_obj);
                    const char *status = json_object_get_string(status_obj);

                    char formatted[128];
                    snprintf(formatted, sizeof(formatted), "%s cambió su estado a %s", user, status);
                    add_message_to_ui(formatted, "status");
                }
            }
        }
    } else if (strcmp(type, TYPE_USER_DISCONNECTED) == 0) {
        if (content) {
            add_message_to_ui(content, "servidor");
        }
    } else if (strcmp(type, TYPE_ERROR) == 0) {
        if (content)
            add_message_to_ui(content, "ERROR");
        else
            add_message_to_ui("Error desconocido recibido del servidor", "ERROR");
    } else {
        add_message_to_ui("Tipo de mensaje no reconocido", "servidor");
    }

    json_object_put(parsed);
}