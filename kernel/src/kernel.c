/*
 ============================================================================
 Name        : kernel.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "kernel.h"
#include <shared-library/connect.h>


t_log* logger;

int main(int argc, char* argv[]) {
	int socket_escucha;
	crear_logger(argv[0], &logger, true, LOG_LEVEL_TRACE); // logger listo para usar
	char* timeStart = temporal_get_string_time(); //usando commons
	printf("Tiempo de Inicio del Proceso: %s\n", timeStart);
	saludo();
	connect_send("mi primer mensaje enviado :)"); //usando nuestra shared library
	log_trace(logger, "Prueba de log");
	socket_escucha = open_socket(10, 46000, logger);
	log_trace(logger, "Socket de escucha: %d", socket_escucha);
	manage_select(socket_escucha, logger);
	return EXIT_SUCCESS;
}

int saludo() {
	puts("¡Hola KERNEL!");
	return EXIT_SUCCESS;
}
