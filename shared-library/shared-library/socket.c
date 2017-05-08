/*
 * connect.c
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */

#include <commons/config.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include "socket.h"

int open_socket(int backlog, int port) {

	struct sockaddr_in server;
	int yes = 1;
	int listenning_socket = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	setsockopt(listenning_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (bind(listenning_socket, (struct sockaddr *) &server, sizeof(server)) == -1) {
		fprintf("socket - error en el bind() (error de tipo: %d)\n", errno);
		close(listenning_socket);
	}

	if (listen(listenning_socket, backlog) == -1) {
		fprintf("socket - error en el listen() (error de tipo: %d)\n", errno);
		close(listenning_socket);
	};

	return listenning_socket;
}

int close_socket(int listenning_socket) {
	return close(listenning_socket);
}

int accept_connection(int listenning_socket) {
	struct sockaddr_in client;
	int c = sizeof(struct sockaddr_in);
	int client_sock = accept(listenning_socket, (struct sockaddr *) &client, (socklen_t *) &c);
	return client_sock;
}

int connect_to_socket(char * server_ip, char * server_port) {
	struct addrinfo hints;
	struct addrinfo * server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(server_ip, server_port, &hints, &server_info) != 0) {
		fprintf(stderr, "socket - error en getaddrinfo()\n");
		exit(1);
	}
	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (server_socket == -1) {
		fprintf(stderr, "socket - error en socket()\n");
		exit(1);
	}
	if (connect(server_socket, server_info->ai_addr, server_info->ai_addrlen) == -1) {
		close(server_socket);
		fprintf(stderr, "socket - error en connect()\n");
		exit(1);
	}
	freeaddrinfo(server_info);
	return server_socket;
}

void manage_select(int socket, t_log * log){
	int nuevaConexion, fd_seleccionado, recibido, set_fd_max, i;
	char buf[512];
	fd_set master, lectura;
	set_fd_max = socket;
	FD_ZERO(&lectura);
	FD_ZERO(&master);
	FD_SET(socket, &master);
	while(1){
		lectura = master;
		select(set_fd_max +1, &lectura, NULL, NULL, NULL);
		for(fd_seleccionado = 0 ; fd_seleccionado <= set_fd_max ; fd_seleccionado++){
			if(FD_ISSET(fd_seleccionado, &lectura)){
				if(fd_seleccionado == socket){
					if((nuevaConexion = accept_connection(socket)) == -1){
						log_error(log, "Error al aceptar conexion");
					} else {
						log_trace(log, "Nueva conexion: socket %d", nuevaConexion);
						FD_SET(nuevaConexion, &master);
						if(nuevaConexion > set_fd_max)set_fd_max = nuevaConexion;
					}
				} else {

					void * operation_code = malloc(sizeof(uint8_t));
					void * buffer;
					int ret;
					ret = connection_recv(fd_seleccionado, operation_code,  buffer);

					if(!ret) {
						FD_CLR(fd_seleccionado, &master);
						close_client(fd_seleccionado);
					}

					/*recibido = recv(fd_seleccionado, buf, sizeof(buf), 0);
					if(recibido <= 0){
						log_error(log, "Desconexion de cliente en socket %d", fd_seleccionado);
						FD_CLR(fd_seleccionado, &master);
						close_client(fd_seleccionado);
					} else {
						for(i=0; i < set_fd_max; i++){
							if(FD_ISSET(i, &master)){
								if(i != socket && i != fd_seleccionado){
									if(send(i, buf, recibido, 0) == -1) log_error(log, "Error en send()");
								}
							}
						}
					}*/
				}
			}
		}
	}
}

int socket_send(int * server_socket, void * buffer, int buffer_size, int flags) {
	return send(* server_socket, buffer, buffer_size, flags);
}

int socket_recv(int * client_socket, void * buffer, int buffer_size) {
	return recv(* client_socket, buffer, buffer_size, MSG_WAITALL);
}

int socket_write(int * client_socket, void * response, int response_size) {
	return write(* client_socket, response, response_size);
}

int close_client(int client_socket) {
	return close(client_socket);
}

int connection_send(int file_descriptor, uint8_t operation_code, void* message){
/**	╔══════════════════════════════════════════════╦══════════════════════════════════════════╦══════════════════════════════╗
	║ operation_code_value (operation_code_length) ║ message_size_value (message_size_length) ║ message (message_size_value) ║
	╚══════════════════════════════════════════════╩══════════════════════════════════════════╩══════════════════════════════╝ **/

	uint8_t operation_code_value = operation_code;
	uint8_t message_size_value;

	switch ((int)operation_code) {
		case OC_SOLICITUD_PROGRAMA_NUEVO:
			message_size_value = *(uint8_t*) message;
			(uint8_t *)message++;
			break;
		case OC_MEMORIA_INSUFICIENTE:
		case OC_SOLICITUD_MEMORIA:
		case OC_LIBERAR_MEMORIA:
//		DEFINIR COMPORTAMIENTO
		default:
			printf("ERROR: Socket %d, Invalid operation code...\n", file_descriptor);
			break;
	}

	uint8_t operation_code_length = sizeof(uint8_t);
	uint8_t message_size_length = sizeof(uint8_t);
	void * buffer = malloc(operation_code_length + message_size_length + message_size_value);
	memcpy(buffer, &operation_code_value, operation_code_length);
	memcpy(buffer + operation_code_length, &message_size_value, message_size_length);
	memcpy(buffer + operation_code_length + message_size_length, message, message_size_value);
	int ret = send(file_descriptor, buffer, operation_code_length + message_size_length + message_size_value, 0);
	free(buffer);

	return ret;
}

int connection_recv(int file_descriptor, uint8_t* operation_code_value, void** message){
/**	╔══════════════════════════════════════════════╦══════════════════════════════════════════╦══════════════════════════════╗
	║ operation_code_value (operation_code_length) ║ message_size_value (message_size_length) ║ message (message_size_value) ║
	╚══════════════════════════════════════════════╩══════════════════════════════════════════╩══════════════════════════════╝ **/

	uint8_t prot_ope_code_size = sizeof(uint8_t);
	uint8_t prot_message_size = sizeof(uint8_t);
	uint8_t message_size;
	int status = 1;
	int ret = 0;
	char* buffer;

	status = recv(file_descriptor, operation_code_value, prot_ope_code_size, 0);
	if (status <= 0) {
		printf("ERROR: Socket %d, disconnected...\n", file_descriptor);
	} else {
		ret = ret + status;
		status = recv(file_descriptor, &message_size, prot_message_size, 0);
		if (status <= 0) {
			printf("ERROR: Socket %d, no message size...\n", file_descriptor);
		} else {
			ret = ret + status;
			//message = (void*) malloc(message_size);
			switch ((int)*operation_code_value) {
			case OC_SOLICITUD_PROGRAMA_NUEVO:

				*message = malloc(message_size +1);
				buffer = (char*)*message;
				status = recv(file_descriptor, buffer, message_size, 0);
				buffer[message_size] = '\0';
				if(status > 0) {

					printf("\nScript: %s\n", buffer);
				}


				break;
			case OC_MEMORIA_INSUFICIENTE:
			case OC_SOLICITUD_MEMORIA:
			case OC_LIBERAR_MEMORIA:
				buffer = malloc(message_size + 1);
				if(message_size > 0){
					status = recv(file_descriptor, buffer, message_size, 0);
				}
				if(status > 0){
					buffer[message_size] = '\0';
					*message = buffer;
				}
				//free(buffer);
				break;
			default:
				printf("ERROR: Socket %d, Invalid operation code(%d)...\n", file_descriptor, (int)*operation_code_value);
				break;
			}

			if (status <= 0) {
				printf("ERROR: Socket %d, no message...\n", file_descriptor);
				ret = status;
			}else{
				ret = ret + status;
			}

		}
	}

	return ret;
}
