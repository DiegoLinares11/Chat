#ifndef SESSION_H
#define SESSION_H

#include <libwebsockets.h>

#define MAX_PAYLOAD 4096

typedef struct per_session_data {
    char buffer[LWS_PRE + MAX_PAYLOAD];
    int buffer_len;
    int buffer_ready;
} per_session_data;

#endif