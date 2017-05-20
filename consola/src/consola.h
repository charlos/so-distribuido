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
#include <signal.h>
#include <stdio_ext.h>

typedef struct {
	char* ipAddress;
	char* port;
} console_cfg;

typedef struct {
	pthread_t thread;
	char * file_content;
	int pid;
} threadpid;

void load_config(char * path);
void enter_command();
int read_command(char * command);
char* read_file(char * path);
void thread_subprograma(threadpid * recon);

#endif /* CONSOLA_H_ */
