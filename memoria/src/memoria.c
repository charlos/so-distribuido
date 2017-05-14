/*
 ============================================================================
 Name        : memoria.c
 Authors     : Carlos Flores, Gustavo Tofaletti, Dante Romero
 Version     :
 Description : Memory Process
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shared-library/socket.h>
#include <shared-library/memory_prot.h>
#include <thread_db.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <commons/string.h>
#include <commons/bitarray.h>

#include "memoria.h"

#define	SOCKET_BACKLOG 100
const char * START_MEMORY = "start";
const char * CONSOLE = "console";
const char * DUMP_CMD = "dump";

t_memory_conf * memory_conf;
t_log * logger;
int listenning_socket;
char * memory_ptr;
t_bitarray * bitmap;
int last_used_index;

t_reg_invert_table* invert_table_begin_ptr;
t_reg_invert_table* invert_table_end_ptr;

void load_memory_properties(void);
void create_logger(void);
void print_memory_properties(void);
void load(void);
void console(void *);
void process_request(int *);
void inicialize_process(int *);
void read_page(int *);
void write_page(int *);
void assign_page(int *);
void finalize_process(int *);
int assign_pages_to_process(int, int);
int get_available_frame(void);
int inicialize_page(int);
int get_frame(int, int);

int main(int argc, char * argv[]) {
	load_memory_properties();
	create_logger();
	print_memory_properties();
	load();

	// console thread
	pthread_attr_t attr;
	pthread_t thread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread, &attr, &console, NULL);
	pthread_attr_destroy(&attr);

	// socket thread
	int * new_sock;
	listenning_socket = open_socket(SOCKET_BACKLOG, memory_conf->port);
	for (;;) {
		new_sock = malloc(1);
		* new_sock = accept_connection(listenning_socket);

		pthread_attr_t attr;
		pthread_t thread;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&thread, &attr, &process_request, (void *) new_sock);
		pthread_attr_destroy(&attr);
	}
	close_socket(listenning_socket);


	return EXIT_SUCCESS;
}

void load_memory_properties(void) {
	t_config * conf = config_create("/home/utnso/memoria.cfg"); // TODO : Ver porque no lo toma del workspace
	memory_conf = malloc(sizeof(t_memory_conf));
	memory_conf->port = config_get_int_value(conf, "PUERTO");
	memory_conf->frames = config_get_int_value(conf, "MARCOS");
	memory_conf->frame_size = config_get_int_value(conf, "MARCO_SIZE");
	memory_conf->cache_entries = config_get_int_value(conf, "ENTRADAS_CACHE");
	memory_conf->cache_x_process = config_get_int_value(conf, "CACHE_X_PROC");
	memory_conf->memory_delay = config_get_int_value(conf, "RETARDO_MEMORIA");
	memory_conf->cache_algorithm = config_get_string_value(conf, "REEMPLAZO_CACHE");
	memory_conf->logfile = config_get_string_value(conf, "LOGFILE");
	memory_conf->consolefile = config_get_string_value(conf, "CONSOLEFILE");
}

void print_memory_properties(void) {
	log_info(logger, "MEMORY PROCESS" );
	log_info(logger, "------ PORT: %u" , memory_conf->port);
	log_info(logger, "------ FRAMES: %u" , memory_conf->frames);
	log_info(logger, "------ FRAME SIZE: %u" , memory_conf->frame_size);
	log_info(logger, "------ CACHE ENTRIES: %u" , memory_conf->cache_entries);
	log_info(logger, "------ CACHE_X_PROC: %u" , memory_conf->cache_x_process);
	log_info(logger, "------ MEMORY DELAY: %u" , memory_conf->memory_delay);
	log_info(logger, "------ CACHE ALGORITHM: %s" , memory_conf->cache_algorithm);
	log_info(logger, "------ LOGFILE: %s" , memory_conf->logfile);
}

void create_logger(void) {
	logger = log_create(memory_conf->logfile, "MEMORY_PROCESS", true, LOG_LEVEL_TRACE);
}

void create_console_file(void) {
	logger = log_create(memory_conf->logfile, "MEMORY_PROCESS", true, LOG_LEVEL_TRACE);
}
void load() {

	// TODO : Se debe generar las estruturas administrativas:
	//				- tabla de páginas invertida
	//				- lista de frames disponibles

	// invert table
	memory_ptr = malloc((memory_conf->frames) * (memory_conf->frame_size));
	t_reg_invert_table * invert_table;
	invert_table_begin_ptr = (t_reg_invert_table *) memory_ptr;
	invert_table_end_ptr = invert_table_begin_ptr + memory_conf->frames;

	invert_table = (t_reg_invert_table *) malloc(memory_conf->frames * sizeof(t_reg_invert_table));
	int i;
	for (i = 0; i < memory_conf->frames; i++) {
		(invert_table + i)->frame = i;
		(invert_table + i)->page = 0;
		(invert_table + i)->pid = 0;
	}

	memcpy(memory_ptr, &invert_table, memory_conf->frames * sizeof(t_reg_invert_table));
	free(invert_table);

	// TODO: available frames collection

}

void process_request(int * client_socket) {
	t_ope_code * request_ope_code = recv_operation_code(logger, client_socket);
	while ((request_ope_code->received_bytes) > 0) {
		log_info(logger, "------ CLIENT %d >> operation code : %d", * client_socket, request_ope_code->ope_code);
		switch (request_ope_code->ope_code) {
		case INIT_PROCESS_OC:
			inicialize_process(client_socket);
			break;
		case READ_OC:
			read_page(client_socket);
			break;
		case WRITE_OC:
			write_page(client_socket);
			break;
		case ASSIGN_PAGE_OC:
			assign_page(client_socket);
			break;
		case END_PROCESS_OC:
			finalize_process(client_socket);
			break;
		default:;
		}
		free(request_ope_code);
		request_ope_code = recv_operation_code(logger, client_socket);
	}
	close_client(* client_socket);
	free(client_socket);
	return;
}

void inicialize_process(int * client_socket) {
	t_init_process_request * request = init_process_recv_req(logger, client_socket);
	assign_pages_to_process(request->pid, request->pages);
	free(request);
	init_process_send_resp(client_socket, SUCCESS);
}

int assign_pages_to_process(int pid, int required_pages) {
	int i;
	for (i = 0; i < required_pages; i++) {
		inicialize_page(pid);
	}
	return EXIT_SUCCESS;
}

int inicialize_page(int pid) {
	int free_frame = get_available_frame();

	// TODO : La función asigna una página de un proceso a un frame, en la tabla de páginas invertida (estructuras administrativas)
	//
	//		  Se debe conocer el último nro. de página asignado a un proceso (pid), para incrementarlo ¿Como puedo obtener el último nro. de página asignado a un proceso?

	return free_frame;
}

int get_available_frame(void) {

	// TODO : La función retorna el nro. de frame que se encuentra disponible.
	//
	// 		  Se debe tomar un elemento de la lista de frames libres, y decrementar el contador de cantidad de elementos de la lista. Si el contador de cantidad de elementos es igual a cero,
	//		  se debe retornar error indicando falta de espacio en memoria.

	return 0;  //pos;
}

void write_page(int * client_socket) {
	t_write_request * w_req = write_recv_req(logger, client_socket);

	int frame = get_frame(w_req->pid, w_req->page);
	char * write_pos = memory_ptr + (frame * (memory_conf->frame_size));
	memcpy(&write_pos[w_req->offset], w_req->buffer, w_req->size);

	free(w_req->buffer);
	free(w_req);
	write_send_resp(client_socket, SUCCESS);
}

int get_frame(int pid, int page) {
	// TODO : La función retorna el nro. de frame para un proceso y página buscando en la tabla de páginas invertida
	// TODO : Se deberá implementar una función hash para buscar el pid dentro de la tabla

	/*t_reg_invert_table * reg_invert_table;
	reg_invert_table = invert_table_begin_ptr;

	while (reg_invert_table->pid != pid && reg_invert_table->page != page) {
		if (reg_invert_table == invert_table_end_ptr) {
			break; // end of invert table
		}
		reg_invert_table++;
	}

	return reg_invert_table->frame;*/
	return 0;
}





void read_page(int * client_socket) {
	t_read_request * r_req = read_recv_req(logger, client_socket);

	void * buff = malloc(r_req->size);
	int frame = get_frame(r_req->pid, r_req->page);
	void * read_pos = memory_ptr +  (frame * memory_conf->frame_size);
	memcpy(buff, read_pos + r_req->offset, r_req->size);
	free(r_req);

	read_send_resp(client_socket, SUCCESS, r_req->size, buff);
	free(buff);
}





void assign_page(int * client_socket) {

}





void finalize_process(int * client_socket) {

}





void console(void * unused) {
	t_log * console = log_create(memory_conf->consolefile, "MEMORY_CONSOLE", true, LOG_LEVEL_TRACE);
	char * command = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&command, &len, stdin)) != -1) {
		if (read > 0) {
			command[read-1] = '\0';
			if (strcmp(command, DUMP_CMD) == 0) {
				log_info(console, "MEMORY CONSOLE : DUMP : %s", memory_ptr);
			}
		}
	}
	free(command);
}
