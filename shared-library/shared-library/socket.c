/*
 * connect.c
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */

#include <commons/config.h>
#include <netdb.h>
#include <sys/socket.h>
#include "socket.h"
#include <errno.h>

int open_socket(int backlog, int port, t_log* logger) {

	struct sockaddr_in server;
	int yes = 1;
	int listenning_socket = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	setsockopt(listenning_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if(bind(listenning_socket, (struct sockaddr *) &server, sizeof(server)) == -1){
		log_error(logger, "Error en el bind().\n error de tipo: %d", errno);
		close(listenning_socket);
	}
	if(listen(listenning_socket, backlog) == -1){
		log_error(logger, "Error en el listen().\n error de tipo: %d", errno);
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

	if(	getaddrinfo(server_ip, server_port, &hints, &server_info) != 0){
		fprintf(stderr, "Error en getaddrinfo\n");
		exit(1);
	}

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if(server_socket == -1){
		fprintf(stderr, "Error en socket()");
		exit(1);
	}
	if(connect(server_socket, server_info->ai_addr, server_info->ai_addrlen) == -1){
		close(server_socket);
		fprintf(stderr, "Error en el connect\n");
		exit(1);
	}
	freeaddrinfo(server_info);
	return server_socket;
}

void manage_select(int socket, t_log* log){
	int nuevaConexion, a, recibido, set_fd_max, i;
	char buf[512];
	fd_set master, lectura;
	set_fd_max = socket;
	FD_ZERO(&lectura);
	FD_ZERO(&master);
	FD_SET(socket, &master);
	while(1){
		lectura = master;
		select(set_fd_max +1, &lectura, NULL, NULL, NULL);
		for(a = 0 ; a <= set_fd_max ; a++){
			if(FD_ISSET(a, &lectura)){
				if(a == socket){
					if((nuevaConexion = accept_connection(socket)) == -1){
						log_error(log, "Error al aceptar conexion");
					} else {
						log_trace(log, "Nueva conexion: socket %d", nuevaConexion);
						FD_SET(nuevaConexion, &master);
						if(nuevaConexion > set_fd_max)set_fd_max = nuevaConexion;
					}
				} else {
					recibido = recv(a, buf, sizeof(buf), 0);
					if(recibido <= 0){
						log_error(log, "Desconexion de cliente en socket %d", a);
						FD_CLR(a, &master);
						close_client(a);
					} else {
						for(i=0; i < set_fd_max; i++){
							if(FD_ISSET(i, &master)){
								if(i != socket && i != a){
									if(send(i, buf, recibido, 0) == -1) log_error(log, "Error en send()");
								}
							}
						}
					}
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
