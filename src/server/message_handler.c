#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include "user_manager.h"
#include "message_handler.h"
#include "../../common/protocol.h"
#include "../../common/json_utils.h"


// Buffer para mensajes salientes
static unsigned char buffer[LWS_PRE + 4096];

// Maneja el registro de un nuevo usuario
void handle_register(struct lws *wsi, const char *username, struct json_object *json_msg) {
    // Obtener dirección IP del cliente
    char ip[32];
    char name_buffer[256];
    
    lws_get_peer_simple(wsi, ip, sizeof(ip));
    
    // Intentar registrar usuario
    int result = add_user(wsi, username, ip);
    
    // Preparar respuesta
    struct json_object *response = json_object_new_object();
    
    if (result == 1) {
        // Registro exitoso
        json_object_object_add(response, "type", json_object_new_string("register_success"));
        json_object_object_add(response, "sender", json_object_new_string("server"));
        json_object_object_add(response, "content", json_object_new_string("Registro exitoso"));
        
        // Añadir lista de usuarios
        char *user_list_str = get_user_list_json();
        struct json_object *user_list = json_tokener_parse(user_list_str);
        struct json_object *content_array;
        
        if (json_object_object_get_ex(user_list, "content", &content_array)) {
            json_object_object_add(response, "userList", json_object_get(content_array));
        }
        
        json_object_object_add(response, "timestamp", json_object_new_int64(time(NULL)));
        
        free(user_list_str);
        json_object_put(user_list);
        
        // Notificar a otros usuarios
        sprintf(name_buffer, "{\"type\":\"user_connected\",\"sender\":\"server\",\"content\":\"%s ha entrado al chat\",\"timestamp\":\"%ld\"}", 
                username, time(NULL));
        broadcast_message_except(name_buffer, wsi);
    } else if (result == 0) {
        // Usuario ya existe
        json_object_object_add(response, "type", json_object_new_string("error"));
        json_object_object_add(response, "sender", json_object_new_string("server"));
        json_object_object_add(response, "content", json_object_new_string("Nombre de usuario ya en uso"));
        json_object_object_add(response, "timestamp", json_object_new_int64(time(NULL)));
    } else {
        // Error de registro
        json_object_object_add(response, "type", json_object_new_string("error"));
        json_object_object_add(response, "sender", json_object_new_string("server"));
        json_object_object_add(response, "content", json_object_new_string("Error al registrar usuario"));
        json_object_object_add(response, "timestamp", json_object_new_int64(time(NULL)));
    }
    
    // Enviar respuesta
    const char *response_str = json_object_to_json_string(response);
    int len = strlen(response_str);
    
    memcpy(buffer + LWS_PRE, response_str, len);
    lws_write(wsi, buffer + LWS_PRE, len, LWS_WRITE_TEXT);
    
    json_object_put(response);
}

// Maneja mensajes de broadcast
void handle_broadcast(struct json_object *json_msg) {
    const char *message_str = json_object_to_json_string(json_msg);
    broadcast_message(message_str);
    
    // Actualizar tiempo de actividad
    struct json_object *json_sender;
    if (json_object_object_get_ex(json_msg, "sender", &json_sender)) {
        const char *sender = json_object_get_string(json_sender);
        update_user_activity(sender);
    }
}

    // Maneja mensajes privados
    void handle_private_message(struct json_object *json_msg) {
        struct json_object *json_sender, *json_target;
        const char *sender, *target;
        
        if (json_object_object_get_ex(json_msg, "sender", &json_sender) &&
            json_object_object_get_ex(json_msg, "target", &json_target)) {
            
            sender = json_object_get_string(json_sender);
            target = json_object_get_string(json_target);
            
            // Buscar destinatario
            struct user *user = find_user_by_name(target);
            if (user) {
                // Enviar mensaje
                const char *message_str = json_object_to_json_string(json_msg);
                int len = strlen(message_str);
                
                memcpy(buffer + LWS_PRE, message_str, len);
                lws_write(user->wsi, buffer + LWS_PRE, len, LWS_WRITE_TEXT);
                
                // También enviar al remitente como confirmación
                struct user *sender_user = find_user_by_name(sender);
                if (sender_user) {
                    lws_write(sender_user->wsi, buffer + LWS_PRE, len, LWS_WRITE_TEXT);
                }
                
                // Actualizar tiempo de actividad
                update_user_activity(sender);
            } else {
                // Usuario no encontrado, enviar error al remitente
                struct user *sender_user = find_user_by_name(sender);
                if (sender_user) {
                    char error_msg[256];
                    sprintf(error_msg, "{\"type\":\"error\",\"sender\":\"server\",\"content\":\"Usuario %s no encontrado\",\"timestamp\":\"%ld\"}", 
                            target, time(NULL));
                    
                    int len = strlen(error_msg);
                    memcpy(buffer + LWS_PRE, error_msg, len);
                    lws_write(sender_user->wsi, buffer + LWS_PRE, len, LWS_WRITE_TEXT);
                }
            }
        }
    }


// Maneja solicitudes de lista de usuarios
void handle_list_users(struct lws *wsi) {
    char *response = get_user_list_json();
    
    int len = strlen(response);
    memcpy(buffer + LWS_PRE, response, len);
    lws_write(wsi, buffer + LWS_PRE, len, LWS_WRITE_TEXT);
    
    free(response);
} 


// Maneja solicitudes de información de usuario
void handle_user_info(struct lws *wsi, struct json_object *json_msg) {
    struct json_object *json_target;
    const char *target;
    
    if (json_object_object_get_ex(json_msg, "target", &json_target)) {
        target = json_object_get_string(json_target);
        
        char *response = get_user_info_json(target);
        
        int len = strlen(response);
        memcpy(buffer + LWS_PRE, response, len);
        lws_write(wsi, buffer + LWS_PRE, len, LWS_WRITE_TEXT);
        
        free(response);
    }
}


// Maneja desconexiones
void handle_disconnect(struct lws *wsi, const char *username) {
    remove_user_by_wsi(wsi);
}
