#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <libwebsockets.h>
#include "ui.h"
#include "network.h"
#include "message_handler.h"
#include "user_input.h"
#include "client_context.h"

#define MAX_MESSAGE_LENGTH 1024

struct client_context client_ctx;

// Thread para recibir mensajes del servidor
void *message_receiver(void *arg) {
    while (!client_ctx.interrupted) {
        lws_service(client_ctx.context, 50);
    }
    return NULL;
}

// Callback de WebSocket
static int callback_client(struct lws *wsi, enum lws_callback_reasons reason,
                           void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            send_register_message(client_ctx.username);
            add_message_to_ui("Conectado al servidor. Enviando registro...", NULL);
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (len < 1) break;

            pthread_mutex_lock(&client_ctx.mutex);
            if (client_ctx.queue_size < 10) {
                strncpy(client_ctx.message_queue[client_ctx.queue_tail], (char *)in, MAX_MESSAGE_LENGTH - 1);
                client_ctx.message_queue[client_ctx.queue_tail][MAX_MESSAGE_LENGTH - 1] = '\0';
                client_ctx.queue_tail = (client_ctx.queue_tail + 1) % 10;
                client_ctx.queue_size++;
                pthread_cond_signal(&client_ctx.recv_cond);
            }
            pthread_mutex_unlock(&client_ctx.mutex);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            send_next_message(); // Enviar si hay mensajes pendientes
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            add_message_to_ui("Conexión con el servidor cerrada", NULL);
            client_ctx.interrupted = 1;
            break;

        default:
            break;
    }
    return 0;
}

// Protocolos soportados
static struct lws_protocols protocols[] = {
    {
        "chat-protocol",
        callback_client,
        0,
        4096,
    },
    { NULL, NULL, 0, 0 }
};

// Signal para terminar
void sigint_handler(int sig) {
    client_ctx.interrupted = 1;
}

// Thread que procesa la cola de mensajes
void *message_processor(void *arg) {
    char message[MAX_MESSAGE_LENGTH];

    while (!client_ctx.interrupted) {
        pthread_mutex_lock(&client_ctx.mutex);
        while (client_ctx.queue_size == 0 && !client_ctx.interrupted) {
            pthread_cond_wait(&client_ctx.recv_cond, &client_ctx.mutex);
        }

        if (client_ctx.interrupted) {
            pthread_mutex_unlock(&client_ctx.mutex);
            break;
        }

        strncpy(message, client_ctx.message_queue[client_ctx.queue_head], MAX_MESSAGE_LENGTH);
        client_ctx.queue_head = (client_ctx.queue_head + 1) % 10;
        client_ctx.queue_size--;
        pthread_mutex_unlock(&client_ctx.mutex);

        process_incoming_message(message); // manejar mensaje JSON
    }

    return NULL;
}

int main(int argc, char **argv) {
    struct lws_context_creation_info info;
    pthread_t service_thread, processor_thread;
    char input_buffer[256];
    int port;

    if (argc != 4) {
        fprintf(stderr, "Uso: %s <nombredeusuario> <IPdelservidor> <puertodelservidor>\n", argv[0]);
        return 1;
    }

    memset(&client_ctx, 0, sizeof(client_ctx));
    strncpy(client_ctx.username, argv[1], sizeof(client_ctx.username) - 1);

    port = atoi(argv[3]);
    if (port <= 0) {
        fprintf(stderr, "Puerto inválido\n");
        return 1;
    }

    pthread_mutex_init(&client_ctx.mutex, NULL);
    pthread_cond_init(&client_ctx.recv_cond, NULL);
    signal(SIGINT, sigint_handler);

    // Inicializar libwebsockets
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    client_ctx.context = lws_create_context(&info);
    if (!client_ctx.context) {
        fprintf(stderr, "Error al crear el contexto WebSocket\n");
        return 1;
    }

    if (!connect_to_server(client_ctx.context, argv[2], port)) {
        fprintf(stderr, "Error al conectar al servidor\n");
        lws_context_destroy(client_ctx.context);
        return 1;
    }

    // Inicializar UI
    init_ui();

    // Iniciar threads
    pthread_create(&service_thread, NULL, message_receiver, NULL);
    pthread_create(&processor_thread, NULL, message_processor, NULL);

    // Bucle principal
    while (!client_ctx.interrupted) {
        read_user_input(input_buffer, sizeof(input_buffer));
        if (input_buffer[0] != '\0') {
            process_user_input(input_buffer);
        }
        update_ui();
        usleep(50000); // 50ms
    }

    send_disconnect_message();
    pthread_join(service_thread, NULL);
    pthread_join(processor_thread, NULL);
    destroy_ui();
    lws_context_destroy(client_ctx.context);
    pthread_mutex_destroy(&client_ctx.mutex);
    pthread_cond_destroy(&client_ctx.recv_cond);

    printf("Cliente finalizado\n");
    return 0;
}