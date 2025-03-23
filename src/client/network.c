#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include "network.h"
#include "client_context.h"
#include "../common/protocol.h"
#include "ui.h"

#define MAX_SEND_BUFFER 4096

// Cola simple de salida (1 mensaje por ahora)
static char pending_message[MAX_SEND_BUFFER];
static int has_pending_message = 0;

// Enviar mensaje por WebSocket
void send_message_raw(const char *json_str) {
    if (!client_ctx.wsi) return;

    size_t len = strlen(json_str);
    unsigned char buffer[LWS_PRE + MAX_SEND_BUFFER];
    unsigned char *ptr = &buffer[LWS_PRE];

    strncpy((char *)ptr, json_str, MAX_SEND_BUFFER - 1);
    ptr[MAX_SEND_BUFFER - 1] = '\0';

    lws_callback_on_writable(client_ctx.wsi);
    has_pending_message = 1;
    strncpy((char *)pending_message, json_str, MAX_SEND_BUFFER - 1);
    pending_message[MAX_SEND_BUFFER - 1] = '\0';
}

// Función pública para enviar el siguiente mensaje
void send_next_message() {
    if (has_pending_message && client_ctx.wsi) {
        unsigned char buffer[LWS_PRE + MAX_SEND_BUFFER];
        unsigned char *ptr = &buffer[LWS_PRE];
        int len = strlen(pending_message);

        strncpy((char *)ptr, pending_message, MAX_SEND_BUFFER - 1);
        lws_write(client_ctx.wsi, ptr, len, LWS_WRITE_TEXT);
        has_pending_message = 0;
    }
}

// Conexión WebSocket al servidor
int connect_to_server(struct lws_context *context, const char *server_ip, int port) {
    struct lws_client_connect_info conn_info;
    memset(&conn_info, 0, sizeof(conn_info));

    conn_info.context = context;
    conn_info.address = server_ip;
    conn_info.port = port;
    conn_info.path = "/chat";
    conn_info.host = conn_info.address;
    conn_info.origin = conn_info.address;
    conn_info.protocol = "chat-protocol";

    client_ctx.wsi = lws_client_connect_via_info(&conn_info);
    return client_ctx.wsi != NULL;
}

// Enviar mensaje de registro
void send_register_message(const char *username) {
    struct json_object *msg = json_object_new_object();
    json_object_object_add(msg, "type", json_object_new_string(TYPE_REGISTER));
    json_object_object_add(msg, "sender", json_object_new_string(username));
    json_object_object_add(msg, "content", NULL);

    const char *json_str = json_object_to_json_string(msg);
    send_message_raw(json_str);

    json_object_put(msg); // Libera memoria
}

// Enviar mensaje de desconexión
void send_disconnect_message() {
    struct json_object *msg = json_object_new_object();
    json_object_object_add(msg, "type", json_object_new_string(TYPE_DISCONNECT));
    json_object_object_add(msg, "sender", json_object_new_string(client_ctx.username));
    json_object_object_add(msg, "content", json_object_new_string("Cierre de sesión"));

    const char *json_str = json_object_to_json_string(msg);
    send_message_raw(json_str);

    json_object_put(msg);
}
