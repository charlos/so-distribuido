/*
 * sockets.c
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */


#include "sockets.h"


int start_socket_server(char* port, t_log* logger){

	struct addrinfo addrAux, *result, *p;
	int socket_fd;

	memset(&addrAux, 0, sizeof addrAux);
	addrAux.ai_family = AF_UNSPEC;
	addrAux.ai_socktype = SOCK_STREAM;
	addrAux.ai_flags = AI_PASSIVE;
	if(getaddrinfo(NULL, port, &addrAux, &result)!= 0){
		fprintf(stderr, "Error en el getaddrinfo()\n");
		log_error(logger, "Error en el getaddrinfo()");
	}

	socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if(bind(socket_fd,result->ai_addr, result->ai_addrlen )== -1){
		fprintf(stderr, "Error en el bind()\n");
		log_error(logger, "Error en el bind()");
		close(socket_fd);
	}

	freeaddrinfo(result);

	if(listen(socket_fd, 15)== -1){
		fprintf(stderr, "Error en el listen()\n");
		log_error(logger, "Error en el listen()");
		close(socket_fd);
		exit(1);
	}
	fprintf(stdout, "Se inicio socket en %d\n", socket_fd);
	return socket_fd;
}


int aceptar_conexion(int socket, t_log* log){
	struct sockaddr_in aux;
	int tamanio = sizeof(aux);
	int nuevoSocket;

	nuevoSocket = accept(socket, (struct sockaddr *) &aux, &tamanio);

	fprintf(stdout, "Se conecto alguien. Socket asignado: %d\n", nuevoSocket);
	log_trace(log, "Se conecto alguien. Socket asignado: %d", nuevoSocket);


	return nuevoSocket;
}

int conectar (char* socket_servidor, char* puerto_servidor, t_log* logger){
	struct addrinfo addrAux, *result, *p;
	int sockfd;


	memset(&addrAux, 0, sizeof addrAux);
	addrAux.ai_family = AF_UNSPEC;
	addrAux.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(socket_servidor, puerto_servidor , &addrAux, &result)!= 0){
		fprintf(stderr, "Error en getaddrinfo\n");
		log_error(logger, "Error en getaddrinfo");
		exit(1);
	}
	sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if(connect(sockfd, result->ai_addr, result->ai_addrlen) == -1){
		close(sockfd);
		fprintf(stderr, "Error en el connect\n");
		log_error(logger, "Error en el connect");
		exit(1);
	}

	freeaddrinfo(result);
	if(p == NULL){
		log_error(logger, "No encuentra servidores a los que conectarse");
		fprintf(stderr, "No se pudo conectar a ningun servidor\n");
		exit(1);
	}

	log_trace(logger, "Pudo conectarse a un server");
	fprintf(stdout, "Se conecto!!!!\n");
	return sockfd;
}
