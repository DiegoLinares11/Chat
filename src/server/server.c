#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include <pthread.h>
#include "server_threads.h"
#include "user_manager.h"
#include "message_handler.h"
#include "server_context.h" 


#define MAX_PAYLOAD 4096

// Contexto global
struct server_context server_ctx;

// Lista de usuarios conectados
static struct user *user_list = NULL;

// Callback para manejar eventos de WebSocket
static int callback_chat(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{
    struct json_object *json_msg = NULL;
    struct json_object *json_type = NULL;
    struct json_object *json_sender = NULL;
    const char *msg_type = NULL;
    const char *sender = NULL;
    
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("WebSocket connection established\n");
            break;
            
        case LWS_CALLBACK_RECEIVE:
            // Procesar mensaje recibido como JSON
            if (len < 1) return 0;
            
            printf("Received: %s\n", (char *)in);
            
            json_msg = json_tokener_parse((char *)in);
            if (!json_msg) {
                printf("Failed to parse JSON\n");
                return 0;
            }
            
            // Extraer tipo de mensaje y remitente
            if (json_object_object_get_ex(json_msg, "type", &json_type) &&
                json_object_object_get_ex(json_msg, "sender", &json_sender)) {
                
                msg_type = json_object_get_string(json_type);
                sender = json_object_get_string(json_sender);
                
                // Manejar mensajes según su tipo
                if (strcmp(msg_type, "register") == 0) {
                    handle_register(wsi, sender, json_msg);
                }
                else if (strcmp(msg_type, "broadcast") == 0) {
                    handle_broadcast(json_msg);
                }
                else if (strcmp(msg_type, "private") == 0) {
                    handle_private_message(json_msg);
                }
                else if (strcmp(msg_type, "list_users") == 0) {
                    handle_list_users(wsi);
                }
                else if (strcmp(msg_type, "user_info") == 0) {
                    handle_user_info(wsi, json_msg);
                }
                else if (strcmp(msg_type, "change_status") == 0) {
                    handle_change_status(json_msg);
                }
                else if (strcmp(msg_type, "disconnect") == 0) {
                    handle_disconnect(wsi, sender);
                }
            }
            
            json_object_put(json_msg);
            break;
            
        case LWS_CALLBACK_CLOSED:
            printf("WebSocket connection closed\n");
            // Buscar y eliminar usuario cuando se cierra la conexión
            remove_user_by_wsi(wsi);
            break;
            
        default:
            break;
    }
    
    return 0;
}

// Protocolos soportados por el servidor
static struct lws_protocols protocols[] = {
    {
        "chat-protocol",
        callback_chat,
        0, // per_session_data_size
        MAX_PAYLOAD,
    },
    { NULL, NULL, 0, 0 } // terminador
};

// Manejador de señal para terminar limpiamente
void sigint_handler(int sig)
{
    server_ctx.interrupted = 1;
}

int main(int argc, char **argv)
{
    struct lws_context_creation_info info;
    int port;
    pthread_t inactivity_thread;

    if (argc != 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        return 1;
    }

    port = atoi(argv[1]);
    if (port <= 0) {
        printf("Puerto inválido\n");
        return 1;
    }

    // Inicializar contexto del servidor
    memset(&server_ctx, 0, sizeof(server_ctx));
    pthread_mutex_init(&server_ctx.user_list_mutex, NULL);

    // Configurar manejador de señal
    signal(SIGINT, sigint_handler);

    // Inicializar lista de usuarios
    init_user_manager(&server_ctx.user_list_mutex);

    // Configurar contexto de libwebsockets
    memset(&info, 0, sizeof(info));
    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    server_ctx.lws_context = lws_create_context(&info);
    if (!server_ctx.lws_context) {
        printf("Error al crear el contexto de libwebsockets\n");
        return 1;
    }

    printf("Servidor de chat iniciado en el puerto %d\n", port);

    // Iniciar thread para verificar inactividad
    pthread_create(&inactivity_thread, NULL, inactivity_checker, NULL);

    // Bucle principal del servidor
    while (!server_ctx.interrupted) {
        lws_service(server_ctx.lws_context, 50);
    }

    // Limpieza
    pthread_join(inactivity_thread, NULL);
    lws_context_destroy(server_ctx.lws_context);
    pthread_mutex_destroy(&server_ctx.user_list_mutex);
    cleanup_user_manager();

    printf("Servidor finalizado\n");
    return 0;
}