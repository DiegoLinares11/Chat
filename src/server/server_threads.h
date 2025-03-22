#ifndef SERVER_THREADS_H
#define SERVER_THREADS_H

// Hilo que verifica usuarios inactivos cada cierto tiempo
void* inactivity_checker(void *arg);

#endif // SERVER_THREADS_H
