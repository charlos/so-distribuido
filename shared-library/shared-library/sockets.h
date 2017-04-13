/*
 * sockets.h
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <commons/log.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>


typedef struct {
	int socket;
	struct addrinfo* plantilla_addrinfo;
}t_socket_struct;

#endif /* SOCKETS_H_ */
