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

t_reg_invert_table* invert_table_begin_ptr; // Puntero al primer registro de la tabla invertida
t_reg_invert_table* invert_table_end_ptr;   // Puntero al ultimo registro de la tabla invertida

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
/*
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

*/
	return EXIT_SUCCESS;
}

void load_memory_properties(void) {
	t_config * conf = config_create("memoria.cfg");
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
void load(){
	memory_ptr = malloc((memory_conf->frames) * (memory_conf->frame_size));
	// TODO : Se debe generar las estruturas administrativas:
	//				- tabla de páginas invertida
	//				- bitmap de frames disponibles


	//Generación de tabla de páginas invertida
	uint32_t i;
	t_reg_invert_table* invert_table;
	invert_table_begin_ptr = (t_reg_invert_table*) memory_ptr;
	invert_table_end_ptr = invert_table_begin_ptr + memory_conf->frames;

	invert_table = (t_reg_invert_table *) malloc(memory_conf->frames * sizeof(t_reg_invert_table));


	for(i=0;i<memory_conf->frames;i++){
		(invert_table+i)->frame = i;
		(invert_table+i)->page = 0;
		(invert_table+i)->pid = 0;
	}

	memcpy(memory_ptr, &invert_table, memory_conf->frames * sizeof(t_reg_invert_table));

	free(invert_table);

	//Generación de bitmap de frames libres
	//bitmap = bitarray_create_with_mode(bitmap_ptr, memory_conf->frames, MSB_FIRST);

}

void process_request(int * client_socket) {
	// << receiving message >>
	uint8_t op_code;
	uint8_t prot_ope_code_size = 1;
	int received_bytes = socket_recv(client_socket, &op_code, prot_ope_code_size);

	while (received_bytes > 0) {
		log_info(logger, "------ CLIENT %d >> operation code : %d", * client_socket, op_code);
		switch (op_code) {
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
		received_bytes = socket_recv(client_socket, &op_code, prot_ope_code_size);
	}
	close_client(* client_socket);
	free(client_socket);
	return;
}

void inicialize_process(int * client_socket) {
	uint32_t pid;
	uint32_t prot_pid = 4;
	int received_bytes = socket_recv(client_socket, &pid, prot_pid);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	uint32_t req_pages;
	uint32_t prot_req_pages = 4;
	received_bytes = socket_recv(client_socket, &req_pages, prot_req_pages);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}

	assign_pages_to_process(pid, req_pages);

	// << sending response >>
	uint8_t resp_code = SUCCESS;
	uint8_t resp_prot_code = 1;

	int response_size = sizeof(char) * (resp_prot_code);
	void * resp = malloc(response_size);
	memcpy(resp, &resp_code, resp_prot_code);
	socket_write(client_socket, resp, response_size);
	free(resp);
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

	return EXIT_SUCCESS;
}

int get_available_frame(void) {
	// TODO : La función retorna el nro. de frame que se encuentra disponible.
	//
	//		  Se debe verificar un bitmap de frames libres (estructura administrativa)
	//
	//		  Se debe tener disponible en una variable GLOBAL el último índice del bitmap en donde fue encontrado un frame disponible (last_used_index)
	//
	//		  Para la búsqueda, se debe posicionar en ell índice last_used_index. Empezar a moverse, verificando que no me pasé del limite (index < memoria_arch_conf->frames - 1).
	//		  En caso de que llegué al final del bitmap, me posiciono al inicio (indice = 0), y continuo la búsqueda.
	//
	//		  Si llego nuevamente a la posición last_used_index, significa que di una vuelta completa, sin haber encontrado un frame disponible. Se debe retornar error indicando
	// 		  falta de espacio en disco.

	int pos;
	bool its_busy;
	pos = last_used_index+1;

	while(1){

		if (pos>memory_conf->frames-1){
			pos=0;
		}

		its_busy = bitarray_test_bit(bitmap, pos);
		if (!its_busy) {
			bitarray_set_bit(bitmap, pos);
			break;
		}
		pos++;
		if (pos==last_used_index){
			return -1; // poner codigo de error "no hay páginas disponibles"
			break;
		}
	}
	last_used_index = pos;
	return pos;
}



void write_page(int * client_socket) {
	uint32_t pid;
	uint8_t prot_pid = 4;
	int received_bytes = socket_recv(client_socket, &pid, prot_pid);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	uint32_t page;
	uint8_t prot_page = 4;
	received_bytes = socket_recv(client_socket, &page, prot_page);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	uint32_t offset;
	uint8_t prot_offset = 4;
	received_bytes = socket_recv(client_socket, &offset, prot_offset);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	uint32_t size;
	uint8_t prot_size = 4;
	received_bytes = socket_recv(client_socket, &size, prot_size);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	uint32_t buf_size;
	uint8_t prot_buf_size = 4;
	received_bytes = socket_recv(client_socket, &buf_size, prot_buf_size);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	char * buffer = malloc(sizeof(char) * (size));
	received_bytes = socket_recv(client_socket, buffer, buf_size);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	log_info(logger, "------ CLIENT %d >> WRITE [ id %d page %d offset %d size %d ]", * client_socket, pid, page, offset, size);



	int frame = get_frame(pid, page);
	char * write_pos = memory_ptr +  (frame * (memory_conf->frame_size));
	memcpy(&write_pos[offset], buffer, size);



	// << sending response >>
	uint8_t resp_code = SUCCESS;
	uint8_t resp_prot_code = 1;

	int response_size = sizeof(char) * (resp_prot_code);
	void * resp = malloc(response_size);
	memcpy(resp, &resp_code, resp_prot_code);
	socket_write(client_socket, resp, response_size);
	free(resp);
	free(buffer);
}

int get_frame(int pid, int page) {
	// TODO : La función retorna el nro. de frame para un proceso y página buscando en la tabla de páginas invertida
	// TODO : Se deberá implementar una función hash para buscar el pid dentro de la tabla

	// Para el checkpoint 2 solo implementamos una busqueda secuencial
	t_reg_invert_table* reg_invert_table;
	reg_invert_table = invert_table_begin_ptr;

	while (reg_invert_table->pid!=pid && reg_invert_table->page!=page){
		//Detecto si es el fin de la tabla
		if(reg_invert_table==invert_table_end_ptr){
			break;
		}
		reg_invert_table++;
	}

	return reg_invert_table->frame;
}




void read_page(int * client_socket) {
	uint32_t pid;
	uint8_t prot_pid = 4;
	int received_bytes = socket_recv(client_socket, &pid, prot_pid);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	uint32_t page;
	uint8_t prot_page = 4;
	received_bytes = socket_recv(client_socket, &page, prot_page);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	uint32_t offset;
	uint8_t prot_offset = 4;
	received_bytes = socket_recv(client_socket, &offset, prot_offset);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}
	uint32_t size;
	uint8_t prot_size = 4;
	received_bytes = socket_recv(client_socket, &size, prot_size);
	if (received_bytes <= 0) {
		log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return;
	}



	void * buff = malloc(size);
	int frame = get_frame(pid, page);
	void * read_pos = memory_ptr +  (frame * memory_conf->frame_size);
	memcpy(buff, read_pos + offset, size);



	// << sending response >>
	uint8_t resp_code = SUCCESS;
	uint8_t prot_resp_code = 1;

	int response_size = sizeof(char) * (prot_resp_code + size);
	void * resp = malloc(response_size);
	memcpy(resp, &resp_code, prot_resp_code);
	memcpy(resp + prot_resp_code, buff, size);

	free(resp);
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
