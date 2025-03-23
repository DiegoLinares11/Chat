#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <json-c/json.h>

// Crea un mensaje de error en JSON
struct json_object* create_error_message(const char *content);

// Crea un mensaje genérico del servidor con tipo y contenido
struct json_object* create_server_message(const char *type, const char *content);

// Agrega timestamp al objeto JSON
void add_timestamp(struct json_object *obj);

// Convierte un objeto JSON a string duplicado (debes liberar con free())
char* json_to_string_dup(struct json_object *obj);

#endif // JSON_UTILS_H