/*
 ============================================================================
 Name        : consola.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <shared-library/connect.h>

#include "consola.h"

console_cfg * console_config;
t_log * console_log;

int main(int argc, char ** argv) {

	char* timeStart = temporal_get_string_time(); //usando commons
	printf("Tiempo de Inicio del Proceso: %s\n", timeStart);
	saludo();
	connect_send("mi primer mensaje enviado :)"); //usando nuestra shared library

	load_config(argv[0]);
	log_create("log_principal", "CONSOLA", true, LOG_LEVEL_INFO);
	log_info(console_log, "IP del Kernel: %s", console_config->ipAddress);
	log_info(console_log, "Puerto: %d", console_config->port);

	conectar(console_config->ipAddress, string_itoa(console_config->port));

	return EXIT_SUCCESS;
}

int saludo() {
	puts("¡Hola CONSOLA!");
	return EXIT_SUCCESS;
}

load_config(char * path) {
	t_config* cfg = config_create(path);
	console_config->ipAddress = config_get_string_value(cfg, "IP_KERNEL");
	console_config->port = config_get_int_value(cfg, "PORT_KERNEL");
}
