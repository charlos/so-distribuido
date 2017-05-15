# Shared library - Memoria: Funciones principales

## init process (request)
```
t_init_process_response * memory_init_process(int server_socket, int pid, int pages, t_log * logger)
```

Pedido de inicio de proceso: 
- server_socket : file descriptor socket
- pid: id del proceso
- pages: cantidad de páginas a solicitar para el proceso
- logger: opcional

Respuesta: 
- t_init_process_response->exec_code = código resultado comunicación socket (DISCONNECTED_CLIENT=201, DISCONNECTED_SERVER=202) 
- t_init_process_response->resp_code = código respuesta inicio del proceso (SUCCESS=1, ERROR=200)

**No olvidar liberar memoria** 
```
t_init_process_response * init_process_resp = memory_init_process(server_socket, pid, pages, logger);
free(init_process_resp);
```  



## write (request)
```
t_write_response * memory_write(int server_socket, int pid, int page, int offset, int size, int buffer_size, void * buffer, t_log * logger)
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
- t_write_response->exec_code = código resultado comunicación socket (DISCONNECTED_CLIENT=201, DISCONNECTED_SERVER=202) 
- t_write_response->resp_code = código respuesta escritura (SUCCESS=1, ERROR=200)

**No olvidar liberar memoria** 
```
t_write_response * write_response = memory_write(server_socket, pid, page, offset, size, buffer_size, buffer, logger);
free(write_response);
```  



## read (request)
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
- t_read_response->exec_code = código resultado comunicación socket (DISCONNECTED_CLIENT=201, DISCONNECTED_SERVER=202) 
- t_read_response->resp_code = código respuesta escritura (SUCCESS=1, ERROR=200)
- t_read_response->buffer_size = tamaño del buffer
- t_read_response->buffer = buffer con los bytes leídos

**No olvidar liberar memoria** 
```
t_read_response * read_response = memory_read(server_socket, pid, page, offset, size, logger);
free(read_response->buffer);
free(read_response);
```  
