#ifndef NETWORK_H
#define NETWORK_H

#include <libwebsockets.h>

// Establece la conexión con el servidor
int connect_to_server(struct lws_context *context, const char *server_ip, int port);

void send_message_raw(const char *json_str);
// Enviar mensajes JSON por WebSocket
void send_register_message(const char *username);
void send_disconnect_message();
void send_next_message();  // Envía el siguiente mensaje pendiente (si hay)

#endif // NETWORK_H
