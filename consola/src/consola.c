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

	printf("%s \n", argv[1]);
	crear_logger(argv[1], &logger, true, LOG_LEVEL_TRACE);
	load_config(argv[1]);

	char* timeStart = temporal_get_string_time(); //usando commons
	printf("Tiempo de Inicio del Proceso: %s\n", timeStart);
	saludo();
	connect_send("mi primer mensaje enviado :)"); //usando nuestra shared library


	//connect_to_socket("127.0.0.1", "46000");
	connect_to_socket(console_config->ipAddress, string_itoa(console_config->port));

	enter_command();

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
	console_config = malloc(sizeof(console_cfg));

	console_config->port = config_get_int_value(cfg, "PORT_KERNEL");
	console_config->ipAddress = config_get_string_value(cfg, "IP_KERNEL");
}
void enter_command() {
	char* command = malloc(sizeof(char)*256);

		printf("/***************************************\\ \n");
		printf("| init path     : Iniciar Programa      |\n");
		printf("| kill pid      : Finalizar Programa    |\n");
		printf("| disconnect    : Desconectar Consola   |\n");
		printf("| clean         : Limpiar Mensajes      |\n");
		printf("\\***************************************/\n");

		fgets(command, 256, stdin);

		int ret = read_command(command);
		printf("%d \n", ret);

		enter_command();
}
int read_command(char* command) {

	//char** palabras = string_n_split(command, 2, " ");
	char** palabras = string_split(command, " ");

	int i = 0;
	while(palabras[i]) {
		printf("%s, ", palabras[i]);
		i++;
	}

	if(palabras[0] == NULL) return -2;

	if(strcmp(palabras[0], "init") == 0) {

		int cantPalabras = 0, i=0;
		while(palabras[i]) {
			cantPalabras++;
			i++;
		}

		if(cantPalabras != 2) {
			printf("Cantidad de parametros erronea\n");
			return -1;
		}
		else {
			printf("Buscando archivo\n");

			FILE* file;
			char buffer[255];
			char * path = palabras[1];

			int caracter = 0;
			while (path[caracter] != '\n') caracter++;
			path[caracter] = '\0';

			printf("%s \n", path);

			file = fopen(path, "r");
			if(file) {

				while(fgets(buffer, 255, (FILE*)file)) {

					printf("%s \n", buffer);
				}
				if(feof(file)) {
					fclose(file);
					return 1;
				}

			}
		}
	}
	else if(strcmp(palabras[0], "kill")==0) {
		if((sizeof(palabras)/sizeof(palabras[0])) != 2) {
			printf("Cantidad de parametros erronea\n");
				return -1;
		}
		else {

		}
	}
	else if(strcmp(palabras[0], "disconnect") ==0 ) {

	}
	else if(strcmp(palabras[0], "clean")==0) {

	}
	else return -2;
}
