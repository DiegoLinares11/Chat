#ifndef PROTOCOL_H
#define PROTOCOL_H

// Tipos de mensajes soportados
#define TYPE_REGISTER "register"
#define TYPE_REGISTER_SUCCESS "register_success"
#define TYPE_BROADCAST "broadcast"
#define TYPE_PRIVATE "private"
#define TYPE_LIST_USERS "list_users"
#define TYPE_LIST_USERS_RESPONSE "list_users_response"
#define TYPE_USER_INFO "user_info"
#define TYPE_USER_INFO_RESPONSE "user_info_response"
#define TYPE_CHANGE_STATUS "change_status"
#define TYPE_STATUS_UPDATE "status_update"
#define TYPE_DISCONNECT "disconnect"
#define TYPE_USER_DISCONNECTED "user_disconnected"
#define TYPE_ERROR "error"

#endif // PROTOCOL_H