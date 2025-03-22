#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H

#include <libwebsockets.h>
#include <pthread.h>

struct server_context {
    struct lws_context *lws_context;
    int interrupted;
    pthread_mutex_t user_list_mutex;
};

extern struct server_context server_ctx;

#endif // SERVER_CONTEXT_H
