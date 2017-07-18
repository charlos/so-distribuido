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
#include <sys/time.h>
#include <sys/types.h>


#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define CON_ERROR_COMANDO -1
#define CON_ERROR_ARG -2
#define CON_ERROR_ARCH -3
#define CON_ERROR_PID -4
#define CON_ERROR_VACIO -5
#define CON_OK 1

typedef struct {
	char* ipAddress;
	char* port;
} console_cfg;

typedef struct {
	char * file_content;
	int pid;
	int terminate;
	uint16_t port;
	uint8_t cantidad_escrituras;
	pid_t tid;
	char * color;
} threadpid;

uint16_t puerto_base;
t_list * paletaDeColores;

uint16_t obtener_puerto();
void load_config(char * path);
void enter_command();
int read_command(char * command);
char* read_file(char * path);
void thread_subprograma(threadpid * recon);
t_list * inicializarPaletaDeColores();

#endif /* CONSOLA_H_ */
