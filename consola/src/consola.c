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

	//char* timeStart = temporal_get_string_time(); //usando commons
	//printf("Tiempo de Inicio del Proceso: %s\n", timeStart);

	//Conexion a kernel
	main_console_socket = connect_to_socket(console_config->ipAddress, console_config->port);

	puerto_base = 16000;

	//Inicio UI
	enter_command();

	return EXIT_SUCCESS;

}

void load_config(char * path) {

	t_config * cfg = malloc(sizeof(t_config));

	if(strcmp(path, "1") == 0) {
		cfg = config_create("/home/utnso/workspace/tp-2017-1c-Stranger-Code/consola/Debug/consola.cfg");
	}
	else {
		cfg = config_create(path);
	}

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

	int i=0;
		while(palabras[i]) {
		i++;
	}

	if(strcmp(palabras[0], "init") == 0) {

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
		if(i != 2) {
			printf("Cantidad de parametros erronea\n");
				return -1;
		}
		else {
			int pid = strtol(palabras[1], NULL, 10);

			int _coincidePid(threadpid* recon) {
				return (recon->pid == pid);
			}
			threadpid* hilo_a_matar = list_find(thread_list, (void *) _coincidePid);
			hilo_a_matar->terminate = 1;
			//pthread_kill(hilo_a_matar->thread, SIGKILL);
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
		printf("\e[1;1H\e[2J");
		enter_command();
	}
	else return -2;
}
void thread_subprograma(threadpid* thread_recon) {

	uint8_t operation_code;
	int sub_console_socket;
	void * buffer;
	thread_recon->terminate = 0;

	sub_console_socket = connect_to_socket(console_config->ipAddress, console_config->port);
	connection_send(sub_console_socket, OC_SOLICITUD_PROGRAMA_NUEVO, thread_recon->file_content);
	int result = connection_recv(sub_console_socket, &operation_code, &buffer);

	if(result > 1 && operation_code == OC_NUEVA_CONSOLA_PID)
		thread_recon->pid = *(uint8_t *) buffer;

	//printf("%d", thread_recon->pid);
	log_trace(logger, "PID DEL PROGRAMA: %d", thread_recon->pid);
	//fprintf(stdout, "%d", thread_recon->pid);

	int listening_socket = open_socket(2, obtener_puerto());

	int nuevaConexion, fd_seleccionado, recibido, set_fd_max, i;
	uint8_t* operation_code;
	char buf[512];
	fd_set lectura;
	pthread_attr_t attr;
	//t_info_socket_solicitud* info_solicitud = malloc(sizeof(t_info_socket_solicitud));
	set_fd_max = listening_socket;
	FD_ZERO(&lectura);
	FD_ZERO((estructura->master));

	int active = 1;
	while(active) {

		//signal(SIGUSR1, _handler_morir);
		if(thread_recon->terminate) pthread_exit(NULL);

		result = connection_recv(sub_console_socket, &operation_code, &buffer);
		if(operation_code == OC_INSTRUCCION_CONSOLA)
			printf("[%d] %s\n", thread_recon->pid, (char *) buffer);
		else if(operation_code == OC_ESCRIBIR_EN_CONSOLA) {
			printf("[Process: %d] %s \n", thread_recon->pid, (char *) buffer);
			log_trace(logger, "PID %d: %s", thread_recon->pid, (char *)buffer);
		}

		if(!result)
			active = 0;

		/*select(set_fd_max +1, &lectura, NULL, NULL, NULL);
		for(fd_seleccionado = 0 ; fd_seleccionado <= set_fd_max ; fd_seleccionado++){
			if(fd_seleccionado == 1)continue;
			if(FD_ISSET(fd_seleccionado, &lectura)){
				if(fd_seleccionado == listening_socket){
					if((nuevaConexion = accept_connection(listening_socket)) == -1){
						log_error(logger, "Error al aceptar conexion");
					} else {
						log_trace(logger, "Nueva conexion: socket %d", nuevaConexion);
						FD_SET(nuevaConexion, (estructura->master));
						if(nuevaConexion > set_fd_max)set_fd_max = nuevaConexion;
					}
				} else {
					pthread_t hilo_solicitud;

						info_solicitud->file_descriptor = fd_seleccionado;
						info_solicitud->set = estructura->master;


						solve_request(info_solicitud);

				}
			}
		}*/
	}


}

char * read_file(char * path) {
	FILE * file;
	char *buffer = malloc(255);

	if(strcmp(path, "1") == 0) {
		file = fopen("/home/utnso/eje.ansisop", "r");
	}
	else file = fopen(path, "r");

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
uint16_t obtener_puerto() {
	return puerto_base + list_size(thread_list);
}
