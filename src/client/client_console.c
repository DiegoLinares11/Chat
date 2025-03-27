#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <libwebsockets.h>
#include "network.h"
#include "message_handler.h"
#include "context.h"

struct client_context ctx;

#define MAX_MESSAGE_LENGTH 1024

void *receiver_thread(void *arg) {
    (void)arg;
    while (!ctx.interrupted) {
        lws_service(ctx.context, 50);
    }
    return NULL;
}

void *input_thread(void *arg) {
    (void)arg;
    char input[MAX_MESSAGE_LENGTH];

    while (!ctx.interrupted) {
        printf("> ");
        if (fgets(input, MAX_MESSAGE_LENGTH, stdin) == NULL)
            continue;

        input[strcspn(input, "\n")] = 0;  // quitar salto de línea

        if (strncmp(input, "/exit", 5) == 0) {
            send_disconnect_message(ctx.wsi, ctx.username);
            ctx.interrupted = 1;
            break;
        } else if (strncmp(input, "/list", 5) == 0) {
            send_list_users(ctx.wsi, ctx.username);
        } else if (strncmp(input, "/status ", 8) == 0) {
            send_change_status(ctx.wsi, ctx.username, input + 8);
        } else if (strncmp(input, "/info ", 6) == 0) {
            send_user_info(ctx.wsi, ctx.username, input + 6);
        } else if (strcmp(input, "/help") == 0) {
            printf("Comandos disponibles:\n");
            printf("/list           -> Mostrar usuarios conectados\n");
            printf("/info <usuario> -> Ver IP y estado de un usuario\n");
            printf("/status <estado>-> Cambiar tu estado (ACTIVO, OCUPADO, INACTIVO)\n");
            printf("@usuario <msg>  -> Enviar mensaje privado\n");
            printf("mensaje libre   -> Enviar mensaje general (broadcast)\n");
            printf("/exit           -> Salir del chat\n");
        } else if (input[0] == '@') {
            char *space = strchr(input, ' ');
            if (space) {
                *space = '\0';
                const char *target = input + 1;
                const char *message = space + 1;
                send_private_message(ctx.wsi, ctx.username, target, message);
            }
        } else {
            send_broadcast_message(ctx.wsi, ctx.username, input);
        }
    }
    return NULL;
}

static int callback_client(struct lws *wsi, enum lws_callback_reasons reason,
                           void *user, void *in, size_t len) {
    (void)user;
    (void)len;

    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            send_register_message(wsi, ctx.username);
            printf("✅ Conectado y registrado como %s\n", ctx.username);
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            handle_server_message(NULL, NULL, (const char *)in);  // terminal pura
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            printf("🔌 Conexión cerrada.\n");
            ctx.interrupted = 1;
            break;

        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    { "chat-protocol", callback_client, 0, MAX_MESSAGE_LENGTH, 0, NULL, 0 },
    { NULL, NULL, 0, 0, 0, NULL, 0 }
};

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <usuario> <IP> <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    strncpy(ctx.username, argv[1], sizeof(ctx.username) - 1);

    struct lws_context_creation_info info = {0};
    struct lws_client_connect_info ccinfo = {0};

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    ctx.context = lws_create_context(&info);
    if (!ctx.context) {
        fprintf(stderr, "Error creando contexto WebSocket.\n");
        return EXIT_FAILURE;
    }

    ccinfo.context = ctx.context;
    ccinfo.address = argv[2];
    ccinfo.port = atoi(argv[3]);
    ccinfo.path = "/chat";
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;
    ccinfo.pwsi = &ctx.wsi;

    if (!lws_client_connect_via_info(&ccinfo)) {
        fprintf(stderr, "No se pudo conectar al servidor.\n");
        return EXIT_FAILURE;
    }

    printf("💡 Comandos disponibles: /help /list /info /status /exit @usuario mensaje\n");

    pthread_t recv_thread, input_thr;
    pthread_create(&recv_thread, NULL, receiver_thread, NULL);
    pthread_create(&input_thr, NULL, input_thread, NULL);

    pthread_join(input_thr, NULL);
    pthread_join(recv_thread, NULL);

    lws_context_destroy(ctx.context);
    return EXIT_SUCCESS;
}