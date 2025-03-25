#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include "ui.h"

// Tamaño máximo para mensajes
#define MAX_JSON_SIZE 1024

// Función auxiliar para enviar un objeto JSON por WebSocket
void send_json_message(struct lws *wsi, struct json_object *jobj) {
    const char *json_str = json_object_to_json_string(jobj);
    unsigned char buffer[LWS_PRE + MAX_JSON_SIZE];
    unsigned char *p = &buffer[LWS_PRE];

    size_t len = strlen(json_str);
    if (len >= MAX_JSON_SIZE) {
        fprintf(stderr, "Mensaje JSON demasiado grande.\n");
        return;
    }

    memcpy(p, json_str, len);
    lws_write(wsi, p, len, LWS_WRITE_TEXT);
}

// Enviar mensaje de registro
void send_register_message(struct lws *wsi, const char *username) {
    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "type", json_object_new_string("register"));
    json_object_object_add(jobj, "sender", json_object_new_string(username));
    json_object_object_add(jobj, "content", NULL);

    send_json_message(wsi, jobj);
    json_object_put(jobj);
}

// Enviar mensaje de broadcast (chat general)
void send_broadcast_message(struct lws *wsi, const char *sender, const char *message) {
    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "type", json_object_new_string("broadcast"));
    json_object_object_add(jobj, "sender", json_object_new_string(sender));
    json_object_object_add(jobj, "content", json_object_new_string(message));

    send_json_message(wsi, jobj);
    json_object_put(jobj);
}

// Enviar mensaje privado
void send_private_message(struct lws *wsi, const char *sender, const char *target, const char *message) {
    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "type", json_object_new_string("private"));
    json_object_object_add(jobj, "sender", json_object_new_string(sender));
    json_object_object_add(jobj, "target", json_object_new_string(target));
    json_object_object_add(jobj, "content", json_object_new_string(message));

    send_json_message(wsi, jobj);
    json_object_put(jobj);
}

// Enviar solicitud para listar usuarios
void send_list_users(struct lws *wsi, const char *sender) {
    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "type", json_object_new_string("list_users"));
    json_object_object_add(jobj, "sender", json_object_new_string(sender));

    send_json_message(wsi, jobj);
    json_object_put(jobj);
}

// Enviar solicitud de información de un usuario específico
void send_user_info(struct lws *wsi, const char *sender, const char *target) {
    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "type", json_object_new_string("user_info"));
    json_object_object_add(jobj, "sender", json_object_new_string(sender));
    json_object_object_add(jobj, "target", json_object_new_string(target));

    send_json_message(wsi, jobj);
    json_object_put(jobj);
}

// Enviar cambio de estado (ACTIVO, OCUPADO, INACTIVO)
void send_change_status(struct lws *wsi, const char *sender, const char *status) {
    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "type", json_object_new_string("change_status"));
    json_object_object_add(jobj, "sender", json_object_new_string(sender));
    json_object_object_add(jobj, "content", json_object_new_string(status));

    send_json_message(wsi, jobj);
    json_object_put(jobj);
}

// Enviar mensaje de desconexión
void send_disconnect_message(struct lws *wsi, const char *sender) {
    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "type", json_object_new_string("disconnect"));
    json_object_object_add(jobj, "sender", json_object_new_string(sender));
    json_object_object_add(jobj, "content", json_object_new_string("Cierre de sesión"));

    send_json_message(wsi, jobj);
    json_object_put(jobj);
}