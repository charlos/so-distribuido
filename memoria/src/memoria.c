/*
 ============================================================================
 Name        : memoria.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shared-library/connect.h>
#include <commons/config.h>
#include <commons/log.h>

#include "memoria.h"

memoria_config * memoria_arch_conf;
t_log * logger;

int main(void) {


	char* timeStart = temporal_get_string_time(); //usando commons
	printf("Tiempo de Inicio del Proceso: %s\n", timeStart);
	saludo();
	connect_send("mi primer mensaje enviado :)"); //usando nuestra shared library


//Levantando el archivo de configuración
	t_config * conf = config_create("memoria.cfg");
	memoria_arch_conf = malloc(sizeof(memoria_config));

	memoria_arch_conf->puerto = config_get_int_value(conf, "PUERTO");
	memoria_arch_conf->marcos = config_get_int_value(conf, "MARCOS");
	memoria_arch_conf->marco_size = config_get_int_value(conf, "MARCO_SIZE");
	memoria_arch_conf->entradas_cache = config_get_int_value(conf, "ENTRADAS_CACHE");
	memoria_arch_conf->cache_x_proc = config_get_int_value(conf, "CACHE_X_PROC");
	memoria_arch_conf->retardo = config_get_int_value(conf, "RETARDO_MEMORIA");
	memoria_arch_conf->reemplazo_cache = config_get_string_value(conf, "REEMPLAZO_CACHE");
	memoria_arch_conf->logfile = config_get_string_value(conf, "LOGFILE");

	create_logger();

//Mostrando configuración por pantalla
	log_info(logger,  "Proceso MEMORIA" );
	log_info(logger,  "------ PUERTO: %u" , memoria_arch_conf->puerto);
	log_info(logger,  "------ MARCOS: %u" , memoria_arch_conf->marcos);
	log_info(logger,  "------ MARCO_SIZE: %u" , memoria_arch_conf->marco_size);
	log_info(logger,  "------ ENTRADAS_CACHE: %u" , memoria_arch_conf->entradas_cache);
	log_info(logger,  "------ CACHE_X_PROC: %u" , memoria_arch_conf->cache_x_proc);
	log_info(logger,  "------ RETARDO_MEMORIA: %u" , memoria_arch_conf->retardo);
	log_info(logger,  "------ REEMPLAZO_CACHE: %s" , memoria_arch_conf->reemplazo_cache);
	log_info(logger,  "------ LOGFILE: %s" , memoria_arch_conf->logfile);

	log_info(logger,  "FIN Proceso MEMORIA" );

	return EXIT_SUCCESS;
}

int saludo() {
	puts("¡Hola MEMORIA!");
	return EXIT_SUCCESS;
}

void create_logger(void) {
	logger = log_create(memoria_arch_conf->logfile, "MEMORIA", true, LOG_LEVEL_TRACE);
}
