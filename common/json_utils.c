#include "json_utils.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Parsear JSON string a estructura ChatMessage
ChatMessage* parse_json_message(const char *json_str) {
    cJSON *json = cJSON_Parse(json_str);
    if (!json) return NULL;

    ChatMessage *msg = malloc(sizeof(ChatMessage));
    if (!msg) {
        cJSON_Delete(json);
        return NULL;
    }

    msg->type = strdup(cJSON_GetObjectItem(json, "type")->valuestring);
    msg->sender = strdup(cJSON_GetObjectItem(json, "sender")->valuestring);

    cJSON *target = cJSON_GetObjectItem(json, "target");
    msg->target = target ? strdup(target->valuestring) : NULL;

    cJSON *content = cJSON_GetObjectItem(json, "content");
    msg->content = content && cJSON_IsString(content) ? strdup(content->valuestring) : NULL;

    cJSON *timestamp = cJSON_GetObjectItem(json, "timestamp");
    msg->timestamp = timestamp ? strdup(timestamp->valuestring) : NULL;

    cJSON_Delete(json);
    return msg;
}

// Liberar memoria de ChatMessage
void free_chat_message(ChatMessage *msg) {
    if (!msg) return;
    free(msg->type);
    free(msg->sender);
    if (msg->target) free(msg->target);
    if (msg->content) free(msg->content);
    if (msg->timestamp) free(msg->timestamp);
    free(msg);
}

// Construir mensaje de respuesta simple
char* build_simple_response(const char *type, const char *content) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", type);
    cJSON_AddStringToObject(json, "sender", "server");
    cJSON_AddStringToObject(json, "content", content);
    cJSON_AddStringToObject(json, "timestamp", get_current_timestamp());

    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return json_str;
}

// Construir respuesta con lista de usuarios
char* build_user_list_response(const char *type, char **users, int count) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", type);
    cJSON_AddStringToObject(json, "sender", "server");
    cJSON *arr = cJSON_CreateStringArray((const char * const*)users, count);
    cJSON_AddItemToObject(json, "content", arr);
    cJSON_AddStringToObject(json, "timestamp", get_current_timestamp());

    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return json_str;
}

// Construir respuesta con información de usuario
char* build_user_info_response(const char *username, const char *ip, const char *status) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "user_info_response");
    cJSON_AddStringToObject(json, "sender", "server");
    cJSON_AddStringToObject(json, "target", username);

    cJSON *content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "ip", ip);
    cJSON_AddStringToObject(content, "status", status);
    cJSON_AddItemToObject(json, "content", content);

    cJSON_AddStringToObject(json, "timestamp", get_current_timestamp());

    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return json_str;
}

// Construir mensaje de error
char* build_error_message(const char *error_msg) {
    return build_simple_response("error", error_msg);
}

// Obtener timestamp actual
char* get_current_timestamp() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char *buf = malloc(30);
    if (!buf) return NULL;
    strftime(buf, 30, "%Y-%m-%d %H:%M:%S", t);
    return buf;
}