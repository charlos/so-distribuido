# Shared library - Memoria: Funciones principales

## init process
```
void memory_init_process(int server_socket, int pid, int pages)
```

Envía pedido de inicio de proceso: 
- server_socket : file descriptor socket
- pid: id del proceso
- pages: cantidad de páginas a solicitar para el proceso



```
t_init_process_response * memory_init_process_recv_resp(int server_socket) 
```

Retorna estructura resultado del inicio de proceso: 
- server_socket : file descriptor socket

Estructura de respuesta
- t_init_process_response.received_bytes = cantidad de bytes recibidos
- t_init_process_response.resp_code = código de respuesta

**No olvidar liberar memoria**



## write
```
void memory_write(int server_socket, int pid, int page, int offset, int size, int buffer_size, void * buffer)
```

Envía pedido de escritura: 
- server_socket : file descriptor socket
- pid: id del proceso
- page: página a realizar la escritura
- offset: posición de inicio de escritura
- size: cantidad de bytes a escribir
- size: tamaño del buffer
- buffer: buffer con los bytes a escribir



```
t_write_response * memory_write_recv_resp(int server_socket)
```

Retorna estructura resultado de escritura: 
- server_socket : file descriptor socket

Estructura de respuesta
- t_write_response.received_bytes = cantidad de bytes recibidos
- t_write_response.resp_code = código de respuesta

**No olvidar liberar memoria**


## read
```
void memory_read(int server_socket, int pid, int page, int offset, int size)
```

Envía pedido de lectura: 
- server_socket : file descriptor socket
- pid: id del proceso
- page: página a realizar la lectura
- offset: posición de inicio de lectura
- size: cantidad de bytes a leer



```
t_read_response * memory_read_recv_resp(int server_socket)
```

Retorna estructura resultado de lectura: 
- server_socket : file descriptor socket

Estructura de respuesta
- t_read_response.received_bytes = cantidad de bytes recibidos
- t_read_response.resp_code = código de respuesta
- t_read_response.buffer_size = tamaño del buffer (si hubo algún error, el mismo es igual a 0)
- t_read_response.buffer = buffer con los bytes leidos

**No olvidar liberar memoria**
