#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include "user_manager.h"
#include "session.h"
#include "server_context.h"  /


#define MAX_PAYLOAD 4096
#define INACTIVITY_TIMEOUT 10 //en segundos

extern struct server_context server_ctx;

// Lista de usuarios y mutex para sincronización
static struct user *user_list = NULL;
static pthread_mutex_t *user_mutex = NULL;

// Inicializa el gestor de usuarios
void init_user_manager(pthread_mutex_t *mutex) {
    user_mutex = mutex;
}

// Libera la memoria de la lista de usuarios
void cleanup_user_manager() {
    pthread_mutex_lock(user_mutex);
    struct user *current = user_list;
    while (current) {
        struct user *next = current->next;
        free(current);
        current = next;
    }
    user_list = NULL;
    pthread_mutex_unlock(user_mutex);
}

// Añade un nuevo usuario a la lista
int add_user(struct lws *wsi, const char *username, const char *ip) {
    pthread_mutex_lock(user_mutex);

    struct user *current = user_list;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            pthread_mutex_unlock(user_mutex);
            return 0; // Usuario ya existe
        }
        current = current->next;
    }

    struct user *new_user = (struct user *)malloc(sizeof(struct user));
    if (!new_user) {
        pthread_mutex_unlock(user_mutex);
        return -1;
    }

    strncpy(new_user->username, username, sizeof(new_user->username)-1);
    new_user->username[sizeof(new_user->username)-1] = '\0';
    strncpy(new_user->ip, ip, sizeof(new_user->ip)-1);
    new_user->ip[sizeof(new_user->ip)-1] = '\0';

    new_user->status = ACTIVE;
    new_user->wsi = wsi;
    new_user->last_activity = time(NULL);

    // Esta línea es nueva: obtenemos el pss desde wsi y lo guardamos
    new_user->pss = (per_session_data *)lws_wsi_user(wsi);

    new_user->next = user_list;
    user_list = new_user;

    pthread_mutex_unlock(user_mutex);
    return 1;
}


// Elimina un usuario por su websocket
void remove_user_by_wsi(struct lws *wsi) {
    char message[128];
    int found = 0;

    pthread_mutex_lock(user_mutex);

    struct user *current = user_list;
    struct user *prev = NULL;

    while (current) {
        if (current->wsi == wsi) {
            snprintf(message, sizeof(message),
                     "{\"type\":\"user_disconnected\",\"sender\":\"server\",\"content\":\"%s ha salido\",\"timestamp\":\"%ld\"}",
                     current->username, time(NULL));

            if (prev) {
                prev->next = current->next;
            } else {
                user_list = current->next;
            }

            free(current);
            found = 1;
            break;
        }

        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(user_mutex);

    if (found) {
        //  el broadcast afuera del lock
        broadcast_message_except(message, wsi);
    }
}


// Busca un usuario por su nombre
User *find_user_by_name(const char *username) {
    pthread_mutex_lock(user_mutex);
    
    struct user *current = user_list;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            pthread_mutex_unlock(user_mutex);
            return current;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(user_mutex);
    return NULL;
}

// Actualiza el estado de un usuario
int update_user_status(const char *username, UserStatus new_status) {
    struct user *current;
    char message[256];
    const char *status_str;

    pthread_mutex_lock(user_mutex);

    current = user_list;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            current->status = new_status;
            current->last_activity = time(NULL);

            switch (new_status) {
                case ACTIVE: status_str = "ACTIVO"; break;
                case BUSY: status_str = "OCUPADO"; break;
                case INACTIVE: status_str = "INACTIVO"; break;
                default: status_str = "DESCONOCIDO"; break;
            }

            snprintf(message, sizeof(message),
                     "{\"type\":\"status_update\",\"sender\":\"server\",\"content\":{\"user\":\"%s\",\"status\":\"%s\"},\"timestamp\":\"%ld\"}",
                     username, status_str, time(NULL));

            pthread_mutex_unlock(user_mutex);  //liberar mutex antes de enviar mensaje

            broadcast_message(message);  //ahora si enviar mensaje

            return 1;
        }
        current = current->next;
    }

    pthread_mutex_unlock(user_mutex);
    return 0;
}


// Actualiza la actividad de un usuario
void update_user_activity(const char *username) {
    pthread_mutex_lock(user_mutex);
    
    struct user *current = user_list;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            current->last_activity = time(NULL);
            break;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(user_mutex);
}

// Verifica usuarios inactivos
void check_inactive_users() {
    time_t current_time = time(NULL);

    pthread_mutex_lock(user_mutex);

    struct user *current = user_list;
    while (current) {
        if (current->status != INACTIVE &&
            difftime(current_time, current->last_activity) > INACTIVITY_TIMEOUT) {
            
            // Cambiar a inactivo
            current->status = INACTIVE;
            current->needs_status_broadcast = 1;

            // ahora (debug)
            printf("[INFO] Usuario %s marcado como INACTIVO\n", current->username);
        }
        current = current->next;
    }

    pthread_mutex_unlock(user_mutex);

    // ⚠️ Esto despierta el hilo principal de libwebsockets
    lws_cancel_service(server_ctx.lws_context);
}


// Envía un mensaje a todos los usuarios
void broadcast_message(const char *message) {
    pthread_mutex_lock(user_mutex);
    
    struct user *current = user_list;
    while (current) {
        per_session_data *pss = (per_session_data *)lws_wsi_user(current->wsi);
        if (pss) {
            snprintf(pss->buffer + LWS_PRE, MAX_PAYLOAD, "%s", message);
            pss->buffer_len = strlen(pss->buffer + LWS_PRE);
            pss->buffer_ready = 1;
            lws_callback_on_writable(current->wsi);
        }
        current = current->next;
    }

    pthread_mutex_unlock(user_mutex);
}


// Envía un mensaje a todos excepto al remitente
void broadcast_message_except(const char *message, struct lws *exclude_wsi) {
    pthread_mutex_lock(user_mutex);
    
    struct user *current = user_list;
    while (current) {
        if (current->wsi != exclude_wsi) {
            per_session_data *pss = (per_session_data *)lws_wsi_user(current->wsi);
            if (pss) {
                snprintf(pss->buffer + LWS_PRE, MAX_PAYLOAD, "%s", message);
                pss->buffer_len = strlen(pss->buffer + LWS_PRE);
                pss->buffer_ready = 1;
                lws_callback_on_writable(current->wsi);
            }
        }
        current = current->next;
    }

    pthread_mutex_unlock(user_mutex);
}


// Prepara respuesta con lista de usuarios
char *get_user_list_json() {
    pthread_mutex_lock(user_mutex);
    
    // Crear array JSON para la lista de usuarios
    struct json_object *user_array = json_object_new_array();
    struct user *current = user_list;
    
    while (current) {
        json_object_array_add(user_array, json_object_new_string(current->username));
        current = current->next;
    }
    
    // Crear objeto JSON de respuesta
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "type", json_object_new_string("list_users_response"));
    json_object_object_add(response, "sender", json_object_new_string("server"));
    json_object_object_add(response, "content", user_array);
    json_object_object_add(response, "timestamp", json_object_new_int64(time(NULL)));
    
    const char *json_string = json_object_to_json_string(response);
    char *result = strdup(json_string);
    
    json_object_put(response);
    pthread_mutex_unlock(user_mutex);
    
    return result;
}

// Genera respuesta JSON con información de un usuario
char *get_user_info_json(const char *username) {
    pthread_mutex_lock(user_mutex);
    
    struct user *current = user_list;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            // Crear objeto JSON con información de usuario
            struct json_object *user_info = json_object_new_object();
            json_object_object_add(user_info, "ip", json_object_new_string(current->ip));
            
            const char *status_str;
            switch (current->status) {
                case ACTIVE: status_str = "ACTIVO"; break;
                case BUSY: status_str = "OCUPADO"; break;
                case INACTIVE: status_str = "INACTIVO"; break;
                default: status_str = "DESCONOCIDO"; break;
            }
            
            json_object_object_add(user_info, "status", json_object_new_string(status_str));
            
            // Crear respuesta completa
            struct json_object *response = json_object_new_object();
            json_object_object_add(response, "type", json_object_new_string("user_info_response"));
            json_object_object_add(response, "sender", json_object_new_string("server"));
            json_object_object_add(response, "target", json_object_new_string(username));
            json_object_object_add(response, "content", user_info);
            json_object_object_add(response, "timestamp", json_object_new_int64(time(NULL)));
            
            const char *json_string = json_object_to_json_string(response);
            char *result = strdup(json_string);
            
            json_object_put(response);
            pthread_mutex_unlock(user_mutex);
            
            return result;
        }
        current = current->next;
    }
    
    // Usuario no encontrado
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "type", json_object_new_string("error"));
    json_object_object_add(response, "sender", json_object_new_string("server"));
    json_object_object_add(response, "content", json_object_new_string("Usuario no encontrado"));
    json_object_object_add(response, "timestamp", json_object_new_int64(time(NULL)));
    
    const char *json_string = json_object_to_json_string(response);
    char *result = strdup(json_string);
    json_object_put(response);
    pthread_mutex_unlock(user_mutex);
    
    return result;
}
User *find_user_by_wsi(struct lws *wsi) {
    pthread_mutex_lock(user_mutex);
    struct user *current = user_list;
    while (current) {
        if (current->wsi == wsi) {
            pthread_mutex_unlock(user_mutex);
            return current;
        }
        current = current->next;
    }
    pthread_mutex_unlock(user_mutex);
    return NULL;
}