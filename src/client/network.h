#ifndef NETWORK_H
#define NETWORK_H

#include <libwebsockets.h>


// Enviar mensaje de registro
void send_register_message(struct lws *wsi, const char *username);

// Mensajes de chat
void send_broadcast_message(struct lws *wsi, const char *sender, const char *message);
void send_private_message(struct lws *wsi, const char *sender, const char *target, const char *message);

// Comandos del cliente
void send_list_users(struct lws *wsi, const char *sender);
void send_user_info(struct lws *wsi, const char *sender, const char *target);
void send_change_status(struct lws *wsi, const char *sender, const char *status);
void send_disconnect_message(struct lws *wsi, const char *sender);

// Interno
void send_json_message(struct lws *wsi, struct json_object *jobj);

#endif // NETWORK_H

