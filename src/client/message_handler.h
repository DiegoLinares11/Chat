#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

// Procesa un mensaje entrante desde el servidor (formato JSON)
void process_incoming_message(const char *json_string);

#endif // MESSAGE_HANDLER_H