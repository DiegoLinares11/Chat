#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

#include <libwebsockets.h>
#include <pthread.h>

#define MAX_MESSAGE_LENGTH 1024

// Declaramos la estructura
struct client_context {
    char username[32];
    struct lws_context *context;
    struct lws *wsi;
    int interrupted;

    pthread_mutex_t mutex;
    pthread_cond_t recv_cond;

    char message_queue[10][MAX_MESSAGE_LENGTH];
    int queue_head;
    int queue_tail;
    int queue_size;
};

// Definimos la variable global
extern struct client_context client_ctx;

#endif // CLIENT_CONTEXT_H