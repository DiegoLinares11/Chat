Instalar las dependencias necesarias:
```bash
sudo apt-get update
sudo apt-get install libwebsockets-dev cmake build-essential
```

Compilacion: 
Necesitas estar en la misma carpeta de server.c y ejecutar:
```bash
gcc server.c server_threads.c user_manager.c message_handler.c -o server -lwebsockets -ljson-c -pthread
```
y luego 
```bash
./server [numero del puerto quieras ej: 9000]
```