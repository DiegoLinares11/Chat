#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include "user_input.h"
#include "network.h"
#include "client_context.h"
#include "ui.h"
#include "../common/protocol.h"

// Utilidad para enviar mensaje tipo JSON
static void send_json_message(const char *type, const char *target, const char *content) {
    struct json_object *msg = json_object_new_object();
    json_object_object_add(msg, "type", json_object_new_string(type));
    json_object_object_add(msg, "sender", json_object_new_string(client_ctx.username));

    if (target)
        json_object_object_add(msg, "target", json_object_new_string(target));

    if (content)
        json_object_object_add(msg, "content", json_object_new_string(content));

    // Timestamp actual
    time_t now = time(NULL);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    json_object_object_add(msg, "timestamp", json_object_new_string(timestamp));

    const char *json_str = json_object_to_json_string(msg);
    send_message_raw(json_str);

    json_object_put(msg);
}

void process_user_input(const char *input) {
    // Comando: ayuda
    if (strcmp(input, "/help") == 0) {
        show_help_message();
        return;
    }

    // Comando: salir
    if (strcmp(input, "/quit") == 0 || strcmp(input, "/exit") == 0) {
        client_ctx.interrupted = 1;
        return;
    }

    // Comando: listar usuarios
    if (strcmp(input, "/list") == 0) {
        send_json_message(TYPE_LIST_USERS, NULL, NULL);
        return;
    }

    // Comando: cambiar estado
    if (strncmp(input, "/status ", 8) == 0) {
        const char *estado = input + 8;
        if (strcmp(estado, "ACTIVO") == 0 || strcmp(estado, "OCUPADO") == 0 || strcmp(estado, "INACTIVO") == 0) {
            send_json_message(TYPE_CHANGE_STATUS, NULL, estado);
        } else {
            add_message_to_ui("Estado inválido. Usa ACTIVO, OCUPADO o INACTIVO", "error");
        }
        return;
    }

    // Comando: información de usuario
    if (strncmp(input, "/info ", 6) == 0) {
        const char *usuario = input + 6;
        if (strlen(usuario) > 0) {
            send_json_message(TYPE_USER_INFO, usuario, NULL);
        } else {
            add_message_to_ui("Debes ingresar un nombre de usuario", "error");
        }
        return;
    }

    // Mensaje privado: @usuario mensaje
    if (input[0] == '@') {
        const char *espacio = strchr(input, ' ');
        if (espacio && espacio[1] != '\0') {
            char destinatario[32];
            size_t nombre_len = espacio - (input + 1);
            strncpy(destinatario, input + 1, nombre_len);
            destinatario[nombre_len] = '\0';

            const char *mensaje = espacio + 1;
            send_json_message(TYPE_PRIVATE, destinatario, mensaje);
            char confirmacion[256];
            snprintf(confirmacion, sizeof(confirmacion), "Tú a %s: %s", destinatario, mensaje);
            add_message_to_ui(confirmacion, NULL);
        } else {
            add_message_to_ui("Formato inválido. Usa: @usuario mensaje", "error");
        }
        return;
    }

    // Por defecto: broadcast
    if (strlen(input) > 0) {
        send_json_message(TYPE_BROADCAST, NULL, input);

        char confirmacion[256];
        snprintf(confirmacion, sizeof(confirmacion), "Tú (todos): %s", input);
        add_message_to_ui(confirmacion, NULL);
    }
}
