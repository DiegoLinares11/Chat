#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <pthread.h>
#include <libwebsockets.h>
#include <time.h>

// Estados de usuario
typedef enum {
    ACTIVE,
    BUSY,
    INACTIVE
} UserStatus;

// Estructura del usuario
typedef struct user {
    char username[32];
    char ip[32];
    UserStatus status;
    struct lws *wsi;
    time_t last_activity;
    struct user *next;
} User;

// Inicialización y limpieza
void init_user_manager(pthread_mutex_t *mutex);
void cleanup_user_manager();

// Gestión de usuarios
int add_user(struct lws *wsi, const char *username, const char *ip);
void remove_user_by_wsi(struct lws *wsi);
User* find_user_by_name(const char *username);
int update_user_status(const char *username, UserStatus status);
void update_user_activity(const char *username);

// Verificación periódica
void check_inactive_users();

// Envío de mensajes
void broadcast_message(const char *message);
void broadcast_message_except(const char *message, struct lws *exclude_wsi);

// Generación de respuestas JSON
char* get_user_list_json();
char* get_user_info_json(const char *username);

#endif // USER_MANAGER_H
