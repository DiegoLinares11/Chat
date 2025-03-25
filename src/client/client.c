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


#define MAX_MESSAGE_LENGTH 1024

// Contexto global del cliente
struct client_context {
    char username[32];
    struct lws_context *context;
    struct lws *wsi;
    int interrupted;

    // Ventanas UI
    WINDOW *chat_win;
    WINDOW *input_win;
    WINDOW *users_win;
};

static struct client_context ctx;

// Thread para recibir mensajes del servidor
void *receiver_thread(void *arg) {
    while (!ctx.interrupted) {
        lws_service(ctx.context, 50); // Procesa eventos WebSocket
    }
    return NULL;
}

// Thread para leer entrada del usuario y enviar mensajes
void *input_thread(void *arg) {
    char input[MAX_MESSAGE_LENGTH];

    while (!ctx.interrupted) {
        get_input_line(ctx.input_win, input, MAX_MESSAGE_LENGTH);

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
            add_message_to_ui(ctx.chat_win, "Comandos disponibles:");
            add_message_to_ui(ctx.chat_win, "/list           -> Mostrar usuarios conectados");
            add_message_to_ui(ctx.chat_win, "/info <usuario> -> Ver IP y estado de un usuario");
            add_message_to_ui(ctx.chat_win, "/status <estado>-> Cambiar tu estado (ACTIVO, OCUPADO, INACTIVO)");
            add_message_to_ui(ctx.chat_win, "@usuario <msg>  -> Enviar mensaje privado");
            add_message_to_ui(ctx.chat_win, "mensaje libre   -> Enviar mensaje general (broadcast)");
            add_message_to_ui(ctx.chat_win, "/exit           -> Salir del chat");
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



// Callback del WebSocket (conexión, recepción, cierre, etc.)



            send_register_message(wsi, ctx.username);
            add_message_to_ui(ctx.chat_win, "Conectado al servidor y registrado.");
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            handle_server_message(ctx.chat_win, ctx.users_win, (const char *)in);
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            add_message_to_ui(ctx.chat_win, "Conexión cerrada.");
            ctx.interrupted = 1;

            break;

        default:
            break;
    }
    return 0;
}


// Protocolos WebSocket
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

    // Inicializar la interfaz
    init_ui(&ctx.chat_win, &ctx.input_win, &ctx.users_win);

    // Configurar contexto WebSocket
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    ctx.context = lws_create_context(&info);
    if (!ctx.context) {
        end_ui();
        fprintf(stderr, "Error creando contexto WebSocket.\n");
        return EXIT_FAILURE;
    }

    // Conexión con el servidor
    ccinfo.context = ctx.context;
    ccinfo.address = argv[2];
    ccinfo.port = atoi(argv[3]);
    ccinfo.path = "/chat";
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;
    ccinfo.pwsi = &ctx.wsi;

    if (!lws_client_connect_via_info(&ccinfo)) {
        end_ui();
        fprintf(stderr, "No se pudo conectar al servidor.\n");
        return EXIT_FAILURE;
    }

    add_message_to_ui(ctx.chat_win, "💡 Comandos disponibles: /help /list /info /status /exit @usuario mensaje");
    add_message_to_ui(ctx.chat_win, "🧠 Escribí /help en cualquier momento para ver esta lista otra vez.");

    // Crear hilos
    pthread_t recv_thread, input_thr;
    pthread_create(&recv_thread, NULL, receiver_thread, NULL);
    pthread_create(&input_thr, NULL, input_thread, NULL);

    pthread_join(input_thr, NULL);
    pthread_join(recv_thread, NULL);

    // Finalizar
    lws_context_destroy(ctx.context);
    end_ui();
    return EXIT_SUCCESS;

}