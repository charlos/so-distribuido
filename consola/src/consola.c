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
	paletaDeColores = inicializarPaletaDeColores();

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

	if(path == NULL) {
		cfg = config_create("./consola.cfg");
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

	for(;;) {
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

	}
}
int read_command(char* command) {

	int caracter = 0;
	while (command[caracter] != '\n') caracter++;
	command[caracter] = '\0';

	if(command[0] == '\0') return CON_ERROR_VACIO;

	char** palabras = string_n_split(command, 2, " ");

	int i=0;
		while(palabras[i]) {
		i++;
	}

	if(strcmp(palabras[0], "init") == 0) {

		if(i != 2) {
			printf("Cantidad de parametros erronea\n");
			return CON_ERROR_ARG;
		}
		else {
			char * path = palabras[1];


			char * file_content = read_file(path);

			if(file_content != NULL) {

				pthread_t thread_program;
				pthread_attr_t attr;

				threadpid* thread_recon = malloc(sizeof(threadpid));
				thread_recon->file_content = file_content;

				pthread_attr_init(&attr);
				pthread_create(&thread_program, &attr, &thread_subprograma, thread_recon);
				pthread_attr_destroy(&attr);

				list_add(thread_list, thread_recon);

				return CON_OK;
			} else {
				return CON_ERROR_ARCH;
			}
		}
	}
	else if(strcmp(palabras[0], "kill")==0) {
		if(i != 2) {
			printf("Cantidad de parametros erronea\n");
				return CON_ERROR_ARG;
		}
		else {
			int pid = strtol(palabras[1], NULL, 10);

			int _coincidePid(threadpid* recon) {
				return (recon->pid == pid);
			}
			threadpid* hilo_a_matar = list_find(thread_list, (void *) _coincidePid);

			if(hilo_a_matar == NULL) {
				return CON_ERROR_PID;
			}
			//hilo_a_matar->terminate = 1;
			//pthread_kill(hilo_a_matar->thread, SIGKILL);
			//pthread_join(hilo_a_matar->tid, NULL);
			connection_send(main_console_socket, OC_KILL_CONSOLA, &(hilo_a_matar->pid));

			void _destroy(threadpid* deon){
				free(deon->file_content);
				free(deon);
			}
			list_remove_and_destroy_by_condition(thread_list, (void *) _coincidePid, (void *)_destroy);

			return CON_OK;
		}
	}
	else if(strcmp(palabras[0], "disconnect") ==0 ) {

		void _killThread(threadpid * recon) {
			pthread_kill(recon->tid, SIGKILL);
		}
		list_iterate(thread_list, (void*) _killThread);

		printf("Hasta Luego");
		//exit(0);
	}
	else if(strcmp(palabras[0], "clean")==0) {
		printf("\e[1;1H\e[2J");
		return CON_OK;
	}
	else return CON_ERROR_COMANDO;
}
void thread_subprograma(threadpid* thread_recon) {


	thread_recon->terminate = 0;
	thread_recon->cantidad_escrituras = 0;
	uint8_t operation_code;
	int sub_console_socket;
	void * buffer;
	char* tiempo_comienzo = temporal_get_string_time();
	struct timeval t1, t2;
	uint32_t elapsedTime;
	thread_recon->tid = pthread_self();

	gettimeofday(&t1, NULL);

	sub_console_socket = connect_to_socket(console_config->ipAddress, console_config->port);
	connection_send(sub_console_socket, OC_SOLICITUD_PROGRAMA_NUEVO, thread_recon->file_content);
	int result = connection_recv(sub_console_socket, &operation_code, &buffer);

	if(result > 1 && operation_code == OC_NUEVA_CONSOLA_PID)
		thread_recon->pid = *(uint8_t *) buffer;

	thread_recon->color = list_get(paletaDeColores, thread_recon->pid % list_size(paletaDeColores));

	printf("%sPID DEL PROGRAMA: %d\n", thread_recon->color,  thread_recon->pid);
	log_trace(logger, "%sPID DEL PROGRAMA: %d", thread_recon->color, thread_recon->pid);

	while(!(thread_recon->terminate)) {

		result = connection_recv(sub_console_socket, &operation_code, &buffer);
		if(operation_code == OC_INSTRUCCION_CONSOLA)
			printf("%s [%d] %s\n", thread_recon->color, thread_recon->pid, (char *) buffer);
		else if(operation_code == OC_ESCRIBIR_EN_CONSOLA) {
			printf("%s [Process: %d] %s \n", thread_recon->color, thread_recon->pid, (char *) buffer);
			log_trace(logger, "PID %d: %s", thread_recon->pid, (char *)buffer);
			thread_recon->cantidad_escrituras++;
		}else if(operation_code == OC_MUERE_PROGRAMA) result = 0;

		if(!result)
			thread_recon->terminate = 1;
	}

	char* tiempo_finalizacion = temporal_get_string_time();
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_usec - t1.tv_usec) / 1000.0;

	log_trace(logger, "[PID: %d] Momento de inicio: %s", thread_recon->pid, tiempo_comienzo);
	log_trace(logger, "[PID: %d] Momento de finalizacion: %s", thread_recon->pid, tiempo_finalizacion);
	log_trace(logger, "[PID: %d] Cantidad de impresiones: %d", thread_recon->pid, thread_recon->cantidad_escrituras);
	log_trace(logger, "[PID: %d] Tiempo de ejecucion: %d", thread_recon->pid, elapsedTime);

	pthread_exit(NULL);

}

char * read_file(char * path) {
	FILE * file;
	char *buffer = malloc(255);

	if(strcmp(path, "1") == 0) {
		file = fopen("/home/utnso/eje.ansisop", "r");
	} else {
		char * dir = string_trim(&path);
		file = fopen(dir, "r");
	}

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
t_list * inicializarPaletaDeColores() {

	t_list * paleta = list_create();

	list_add(paleta, BLU);
	list_add(paleta, YEL);
	list_add(paleta, GRN);
	list_add(paleta, MAG);
	list_add(paleta, CYN);

	return paleta;

}
