/*
 * consola.h
 *
 *  Created on: 14/4/2017
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <shared-library/connect.h>
#include <shared-library/socket.h>
#include <commons/config.h>
#include <commons/collections/list.h>

typedef struct {
	char* ipAddress;
	char* port;
} console_cfg;

void thread_subprograma(char * string);

#endif /* CONSOLA_H_ */
