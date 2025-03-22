#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include "user_manager.h"

#define MAX_PAYLOAD 4096
#define INACTIVITY_TIMEOUT 300 // 5 minutos en segundos


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
    // Comprobar si el usuario ya existe
    pthread_mutex_lock(user_mutex);
    
    struct user *current = user_list;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            pthread_mutex_unlock(user_mutex);
            return 0; // Usuario ya existe
        }
        current = current->next;
    }
    
    // Crear nuevo usuario
    struct user *new_user = (struct user *)malloc(sizeof(struct user));
    if (!new_user) {
        pthread_mutex_unlock(user_mutex);
        return -1; // Error de memoria
    }
    
    strncpy(new_user->username, username, sizeof(new_user->username)-1);
    new_user->username[sizeof(new_user->username)-1] = '\0';
    
    strncpy(new_user->ip, ip, sizeof(new_user->ip)-1);
    new_user->ip[sizeof(new_user->ip)-1] = '\0';
    
    new_user->status = ACTIVE;
    new_user->wsi = wsi;
    new_user->last_activity = time(NULL);
    
    // Añadir a la lista
    new_user->next = user_list;
    user_list = new_user;
    
    pthread_mutex_unlock(user_mutex);
    return 1; // Éxito
}

// Elimina un usuario por su websocket
void remove_user_by_wsi(struct lws *wsi) {
    pthread_mutex_lock(user_mutex);
    
    struct user *current = user_list;
    struct user *prev = NULL;
    
    while (current) {
        if (current->wsi == wsi) {
            // Broadcast notification of user disconnect
            char message[128];
            snprintf(message, sizeof(message), 
                     "{\"type\":\"user_disconnected\",\"sender\":\"server\",\"content\":\"%s ha salido\",\"timestamp\":\"%ld\"}", 
                     current->username, time(NULL));
            broadcast_message_except(message, wsi);
            
            // Remove from list
            if (prev) {
                prev->next = current->next;
            } else {
                user_list = current->next;
            }
            
            free(current);
            break;
        }
        
        prev = current;
        current = current->next;
    }
    
    pthread_mutex_unlock(user_mutex);
}

    
// Actualiza el estado de un usuario
int update_user_status(const char *username, UserStatus new_status) {
    pthread_mutex_lock(user_mutex);
    
    struct user *current = user_list;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            current->status = new_status;
            current->last_activity = time(NULL);
            
            // Notificar cambio de estado
            char message[256];
            const char *status_str;
            
            switch (new_status) {
                case ACTIVE: status_str = "ACTIVO"; break;
                case BUSY: status_str = "OCUPADO"; break;
                case INACTIVE: status_str = "INACTIVO"; break;
                default: status_str = "DESCONOCIDO"; break;
            }
            
            snprintf(message, sizeof(message), 
                     "{\"type\":\"status_update\",\"sender\":\"server\",\"content\":{\"user\":\"%s\",\"status\":\"%s\"},\"timestamp\":\"%ld\"}", 
                     username, status_str, time(NULL));
            
            broadcast_message(message);
            
            pthread_mutex_unlock(user_mutex);
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
            
            // Notificar cambio
            char message[256];
            snprintf(message, sizeof(message), 
                     "{\"type\":\"status_update\",\"sender\":\"server\",\"content\":{\"user\":\"%s\",\"status\":\"INACTIVO\"},\"timestamp\":\"%ld\"}", 
                     current->username, current_time);
            
            broadcast_message(message);
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(user_mutex);
}

