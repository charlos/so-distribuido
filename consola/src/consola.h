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
#include <shared-library/connect.h>
#include <shared-library/socket.h>
#include <commons/config.h>

typedef struct {
	char* ipAddress;
	int* port;
} console_cfg;


#endif /* CONSOLA_H_ */
