#ifndef CONTEXT_H
#define CONTEXT_H

#include <libwebsockets.h>
#include <ncurses.h>

struct client_context {
    char username[32];
    struct lws_context *context;
    struct lws *wsi;
    int interrupted;
};

extern struct client_context ctx;

#endif