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
int console_socket;
t_list* thread_list;

int main(int argc, char* argv[]) {

	uint8_t operation_code;
	printf("%s \n", argv[1]);
	crear_logger(argv[0], &logger, true, LOG_LEVEL_TRACE);
	load_config(argv[1]);
	thread_list = list_create();

	char* timeStart = temporal_get_string_time(); //usando commons
	printf("Tiempo de Inicio del Proceso: %s\n", timeStart);
	saludo();
	connect_send("mi primer mensaje enviado :)"); //usando nuestra shared library

	console_socket = connect_to_socket(console_config->ipAddress, console_config->port);

	char* mensaje = string_duplicate("este es un mensaje que se va a mandar a kernel");
	connection_send(console_socket, OC_SOLICITUD_PROGRAMA_NUEVO_A_MEMORIA, mensaje);
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

	console_config->port = config_get_string_value(cfg, "PORT_KERNEL");
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
		printf("\n%d \n", ret);

		enter_command();
}
int read_command(char* command) {

	int caracter = 0;
	int car = string_length(command);
	//while (command[caracter] != '\n') caracter++;
	//command[caracter] = '\0';
	command[string_length(command)] = '\0';

	char** palabras = string_n_split(command, 2, " ");

	if(palabras[0] == NULL) return -2;

	if(strcmp(palabras[0], "init") == 0) {

		int i=0;
		while(palabras[i]) {
			i++;
		}
		if(i != 2) {
			printf("Cantidad de parametros erronea\n");
			return -1;
		}
		else {
			FILE* file;
			char buffer[255];
			char * path = palabras[1];

			file = fopen(path, "r");
			if(file) {

				char* string = string_new();

				while(fgets(buffer, 255, (FILE*)file)) {
					string_append(&string, buffer);
					printf("%s", buffer);
				}
				if(feof(file)) {
					fclose(file);
					printf("\nMensaje compilado: %s \n", string);

					pthread_t thread_program;
					pthread_attr_t attr;
					pthread_attr_init(&attr);
					pthread_create(&thread_program, &attr, &thread_subprograma, string);
					pthread_attr_destroy(&attr);

					list_add(thread_list, &thread_program);

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
void mandarScriptAKernel(char * string) {

	connection_send(console_socket, OC_SOLICITUD_PROGRAMA_NUEVO_A_MEMORIA, string);

	/*uint8_t size_opc = sizeof(uint8_t);
	uint8_t size_char = sizeof(char);
	uint8_t length_string = strlen(string) * size_char;
	uint8_t length_message = length_string +1;

	void * buffer = malloc(length_message);



	memcpy(buffer, &length_string, 1);
	memcpy(buffer + 1, string, length_string);
	connection_send(console_socket, operation_code, buffer);
	free(buffer);*/

}
void thread_subprograma(char * string) {
	connection_send(console_socket, OC_SOLICITUD_PROGRAMA_NUEVO_A_MEMORIA, string);
}
