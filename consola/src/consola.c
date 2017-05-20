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

int main_console_socket;
t_list * thread_list;


int main(int argc, char* argv[]) {

	crear_logger(argv[0], &logger, true, LOG_LEVEL_TRACE);
	load_config(argv[1]);
	thread_list = list_create();

	char* timeStart = temporal_get_string_time(); //usando commons
	printf("Tiempo de Inicio del Proceso: %s\n", timeStart);
	saludo();
	connect_send("mi primer mensaje enviado :)"); //usando nuestra shared library

	//Conexion a kernel
	main_console_socket = connect_to_socket(console_config->ipAddress, console_config->port);

	//Inicio UI
	enter_command();

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

	free(cfg);
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
	free(command);
	enter_command();
}
int read_command(char* command) {

	int caracter = 0;
	while (command[caracter] != '\n') caracter++;
	command[caracter] = '\0';

	char** palabras = string_n_split(command, 2, " ");

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
			char * path = palabras[1];
			char * file_content = read_file(path);

			if(*file_content != NULL) {

				pthread_t thread_program;
				pthread_attr_t attr;

				threadpid* thread_recon = malloc(sizeof(threadpid));
				thread_recon->thread = thread_program;
				thread_recon->file_content = file_content;

				pthread_attr_init(&attr);
				pthread_create(&thread_program, &attr, &thread_subprograma, thread_recon);
				pthread_attr_destroy(&attr);

				list_add(thread_list, thread_recon);

				return 1;
			}
		}
	}
	else if(strcmp(palabras[0], "kill")==0) {
		if((sizeof(palabras)/sizeof(palabras[0])) != 2) {
			printf("Cantidad de parametros erronea\n");
				return -1;
		}
		else {
			int pid = strtol(palabras[1], NULL, 10);

			int _coincidePid(threadpid* recon) {
				return (recon->pid == pid);
			}
			threadpid* hilo_a_matar = list_find(thread_list, (void *) _coincidePid);
			pthread_kill(hilo_a_matar->thread, SIGKILL);
			//pthread_join(hilo_a_matar->thread, NULL);
			connection_send(main_console_socket, OC_KILL_CONSOLA, hilo_a_matar->pid);

			void _destroy(threadpid* deon){
				free(deon->file_content);
				free(deon);
			}
			list_remove_and_destroy_by_condition(thread_list, (void *) _coincidePid, (void *)_destroy);
		}
	}
	else if(strcmp(palabras[0], "disconnect") ==0 ) {

		void _killThread(threadpid * recon) {
			pthread_kill(recon->thread, SIGKILL);
		}
		list_iterate(thread_list, (void*) _killThread);
		exit(0);
	}
	else if(strcmp(palabras[0], "clean")==0) {
		__fpurge(stdout);
	}
	else return -2;
}
void mandarScriptAKernel(char * string) {

	connection_send(main_console_socket, OC_SOLICITUD_PROGRAMA_NUEVO, string);

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
void thread_subprograma(threadpid* thread_recon) {

	uint8_t operation_code;
	int sub_console_socket;
	void * buffer;

	sub_console_socket = connect_to_socket(console_config->ipAddress, console_config->port);
	connection_send(sub_console_socket, OC_SOLICITUD_PROGRAMA_NUEVO, thread_recon->file_content);
	int result = connection_recv(sub_console_socket, &operation_code, &buffer);

	if(result > 1 && operation_code == OC_NUEVA_CONSOLA_PID)
		thread_recon->pid = *(uint8_t *) buffer;

	int active = 1;
	while(active) {
		result = connection_recv(sub_console_socket, &operation_code, &buffer);
		if(operation_code == OC_INSTRUCCION_CONSOLA)
			printf("[%d] %s\n", thread_recon->pid, (char *) buffer);
	}

}
char * read_file(char * path) {
	FILE * file;
	char *buffer = malloc(255);

	file = fopen(path, "r");

	if(file) {
		char* string = string_new();

		while(fgets(buffer, 255, (FILE*)file)) {
			string_append(&string, buffer);
		}
		if(feof(file)) {
			fclose(file);
			printf("\nMensaje compilado: %s \n", string);
			free(buffer);
			return string;
		}
		return NULL;
	}
	return NULL;
}
