# Shared library - Memoria: Funciones principales

## handshake
```
int handshake(int * server_socket, t_log * logger)
```

Pedido de tamaño de frame en memoria: 
- server_socket : file descriptor socket
- logger: opcional

Respuesta: 
- int = tamaño frame de memoria. Posibles errores: (DISCONNECTED_SERVER=-202)



## init process
```
int memory_init_process(int server_socket, int pid, int pages, t_log * logger)
```

Pedido de inicio de proceso: 
- server_socket : file descriptor socket
- pid: id del proceso
- pages: cantidad de páginas a solicitar para el proceso
- logger: opcional

Respuesta: 
- int = código respuesta inicio del proceso (SUCCESS=1, DISCONNECTED_SERVER=-202, ENOSPC=-203)



## write
```
int memory_write(int server_socket, int pid, int page, int offset, int size, int buffer_size, void * buffer, t_log * logger)
```

Pedido de escritura: 
- server_socket : file descriptor socket
- pid: id del proceso
- page: página a realizar la escritura
- offset: posición de inicio de escritura
- size: cantidad de bytes a escribir
- buffer_size: tamaño del buffer
- buffer: buffer con los bytes a escribir
- logger: opcional

Respuesta: 
- int = código respuesta escritura (SUCCESS=1, DISCONNECTED_SERVER=-202)



## read
```
t_read_response * memory_read(int server_socket, int pid, int page, int offset, int size, t_log * logger) 
```

Pedido de lectura: 
- server_socket : file descriptor socket
- pid: id del proceso
- page: página a realizar la lectura
- offset: posición de inicio de lectura
- size: cantidad de bytes a leer
- logger: opcional

Respuesta:
- t_read_response->exec_code = código respuesta lectura (SUCCESS=1, DISCONNECTED_SERVER=-202)
- t_read_response->buffer_size = tamaño del buffer
- t_read_response->buffer = buffer con los bytes leídos

**No olvidar liberar memoria** 
```
t_read_response * read_response = memory_read(server_socket, pid, page, offset, size, logger);
free(read_response->buffer);
free(read_response);
```  



## assign pages
```
int memory_assign_pages(int server_socket, int pid, int pages, t_log * logger) 
```

Pedido de asignación de páginas a un proceso: 
- server_socket : file descriptor socket
- pid: id del proceso
- pages: cantidad de páginas a asignar
- logger: opcional

Respuesta:
- int = código respuesta asignación (SUCCESS=1, DISCONNECTED_SERVER=-202, ENOSPC=-203)



## finalize process
```
int memory_finalize_process(int server_socket, int pid, t_log * logger) 
```

Pedido de finalización del proceso: 
- server_socket : file descriptor socket
- pid: id del proceso
- logger: opcional

Respuesta:
- int = código respuesta finalización del proceso (SUCCESS=1, DISCONNECTED_SERVER=-202)
