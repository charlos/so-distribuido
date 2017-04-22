/*
 ============================================================================
 Name        : consola.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "consola.h"

t_log* logger;
console_cfg * console_config;

int main(int argc, char* argv[]) {
	crear_logger(argv[0], &logger, true, LOG_LEVEL_TRACE);
//	load_config(argv[0]);
	char* timeStart = temporal_get_string_time(); //usando commons
	printf("Tiempo de Inicio del Proceso: %s\n", timeStart);
	saludo();
	connect_send("mi primer mensaje enviado :)"); //usando nuestra shared library
	connect_to_socket("127.0.0.1", "46000");
	//connect_to_socket(console_config->ipAddress, string_itoa(console_config->port));

	char* command = malloc(sizeof(char)*256);

	printf("/***************************************\\n");
	printf("| init path		: Iniciar Programa			|\n");
	printf("| kill pid		: Finalizar Programa	|\n");
	printf("| disconnect	: Desconectar Consola	|\n");
	printf("| clean			: Limpiar Mensajes		|\n");
	printf("\***************************************/\n");

	fgets(command, 256, stdin);

	int ret = read_command(command);

	/*while(1){
		sleep(20);
	}*/
	return EXIT_SUCCESS;

}

int saludo() {
	puts("¡Hola CONSOLA!");
	return EXIT_SUCCESS;
}

void load_config(char * path) {
	t_config* cfg = config_create(path);
	console_config->ipAddress = config_get_string_value(cfg, "IP_KERNEL");
	console_config->port = config_get_int_value(cfg, "PORT_KERNEL");
}

int read_command(char* command) {

	char** palabras = string_split(command, " ");

	if(palabras[0] == NULL) return -2;

	if(palabras[0] == "init") {
		if((sizeof(palabras)/sizeof(palabras[0])) != 2) {
			printf("Cantidad de parametros erronea\n");
			return -1;
		}
		else {

		}
	}
	else if(palabras[0] == "kill") {
		if((sizeof(palabras)/sizeof(palabras[0])) != 2) {
			printf("Cantidad de parametros erronea\n");
				return -1;
		}
		else {

		}
	}
	else if(palabras[0] == "disconnect") {

	}
	else if(palabras[0] == "clean") {

	}
	else return -2;
}
