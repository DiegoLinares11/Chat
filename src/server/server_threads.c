#include <stdio.h>
#include <unistd.h>
#include "server_threads.h"
#include "user_manager.h"
#include "server_context.h" 

// Referencia al contexto global declarado en server.c
extern struct server_context server_ctx;

void* inactivity_checker(void *arg) {
    while (!server_ctx.interrupted) {
        sleep(10);
        check_inactive_users();
    }
    return NULL;
}

