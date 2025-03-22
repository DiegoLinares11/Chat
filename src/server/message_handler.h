#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <libwebsockets.h>
#include <json-c/json.h>

// Funciones para manejar los diferentes tipos de mensajes
void handle_register(struct lws *wsi, const char *username, struct json_object *json_msg);
void handle_broadcast(struct json_object *json_msg);
void handle_private_message(struct json_object *json_msg);
void handle_list_users(struct lws *wsi);
void handle_user_info(struct lws *wsi, struct json_object *json_msg);
void handle_change_status(struct json_object *json_msg);
void handle_disconnect(struct lws *wsi, const char *username);

#endif // MESSAGE_HANDLER_H
