/*
 ============================================================================
 Name        : memoria.c
 Authors     : Carlos Flores, Gustavo Tofaletti, Dante Romero
 Version     :
 Description : Memory Process
 ============================================================================
 */

#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <shared-library/memory_prot.h>
#include <shared-library/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread_db.h>
#include "memoria.h"

#define	SOCKET_BACKLOG 100
const char * DELAY_CMD = "delay";
const char * DUMP_CMD  = "dump";
const char * DUMP_CACHE_CMD  = "cache";
const char * DUMP_MEMORY_STRUCT_CMD = "struct";
const char * DUMP_MEMORY_CONTENT_CMD = "content";
const char * FLUSH_CMD = "flush";
const char * SIZE_CMD = "size";
const char * SIZE_MEMORY_CMD = "memory";

int listenning_socket;
t_memory_conf * memory_conf;
t_log * logger;
char * memory_ptr;

t_reg_invert_table * invert_table;
t_list * available_frame_list;
t_list * pages_per_process_list;

pthread_mutex_t m_lock; // TODO : checkpoint 3 es válido atender los pedidos de forma secuencial

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
void inicialize_page(int, int);
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
	pthread_mutex_destroy(&m_lock); // TODO : checkpoint 3 es válido atender los pedidos de forma secuencial
	return EXIT_SUCCESS;
}

/**
 * @NAME load_memory_properties
 * @DESC Carga las configuraciones del proceso, desde un archivo de configuración
 * @PARAMS
 */
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

/**
 * @NAME print_memory_properties
 * @DESC Guarda en el archivo log del proceso, las configuraciones para el proceso
 */
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

/**
 * @NAME create_logger
 * @DESC Crea el archivo log del proceso
 */
void create_logger(void) {
	logger = log_create(memory_conf->logfile, "MEMORY_PROCESS", true, LOG_LEVEL_TRACE);
}

/**
 * @NAME load
 * @DESC Genera las siguientes estruturas a utilizar durante el ciclo de vida del proceso:
 *					- memoria: bloque de memoria contigua, para simular la memoria principal (tamaño configurable por archivo de configuración)
 *					- tabla de páginas invertida
 *					- lista de frames disponibles
 *					- lista de cantidad de páginas asociadas a un proceso
 *       Inicia los semáforos a utilizar
 */
void load(void) {

	// memory
	memory_ptr = malloc((memory_conf->frames) * (memory_conf->frame_size));

	// TODO : la estructura de páginas se debe encuentrar en memoria principal
	// 				(checkpoint 3: no se requiere que la estructura de páginas se encuentre en memoria principal)

	// invert table
	invert_table = (t_reg_invert_table *) malloc(memory_conf->frames * sizeof(t_reg_invert_table));
	// available frames list
	available_frame_list = list_create();

	t_reg_invert_table * inv_table_aux = invert_table;
	int i = 0;
	int * node;
	while (i < memory_conf->frames) {
		inv_table_aux += i;
		inv_table_aux->frame = i;
		inv_table_aux->page = 0;
		inv_table_aux->pid = 0;
		node = malloc(sizeof(int));
		memcpy(node, &i, sizeof(int));
		list_add(available_frame_list, node);
		i++;
	}

	// pages per process
	pages_per_process_list = list_create();

	pthread_mutex_init(&m_lock, NULL); // TODO : checkpoint 3 es válido atender los pedidos de forma secuencial
}

/**
 * @NAME process_request
 * @DESC Procesa solicitudes de un cliente
 * @PARAMS client_socket
 */
void process_request(int * client_socket) {
	t_ope_code * request_ope_code = recv_operation_code(logger, client_socket);
	while (request_ope_code->exec_code == SUCCESS) {
		log_info(logger, "------ CLIENT %d >> operation code : %d", * client_socket, request_ope_code->ope_code);
		pthread_mutex_lock(&m_lock); // TODO : checkpoint 3 es válido atender los pedidos de forma secuencial
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
		pthread_mutex_unlock(&m_lock); // TODO : checkpoint 3 es válido atender los pedidos de forma secuencial

		free(request_ope_code);
		request_ope_code = recv_operation_code(logger, client_socket);
	}
	close_client(* client_socket);
	free(client_socket);
	return;
}

/**
 * @NAME inicialize_process
 * @DESC
 * @PARAMS client_socket
 */
void inicialize_process(int * client_socket) {
	t_init_process_request * init_req = init_process_recv_req(client_socket, logger);
	if (init_req->exec_code == DISCONNECTED_CLIENT) return;
	int resp_code = assign_pages_to_process(init_req->pid, init_req->pages);
	free(init_req);
	init_process_send_resp(client_socket, resp_code);
}

/**
 * @NAME assign_pages_to_process
 * @DESC Asigna páginas a un proceso
 * @PARAMS 						pid : process id
 *				 required_pages : cantidad de páginas requeridas
 */
int assign_pages_to_process(int pid, int required_pages) {

	if (available_frame_list->elements_count < required_pages) { // check available frames for request
		return ENOSPC;
	}

	int located = -1;
	t_reg_pages_process_table * pages_per_process;
	if ((pages_per_process_list->elements_count) > 0) { // searching process on pages per process list
		int index = 0;
		while (located < 0) {
			pages_per_process = (t_reg_pages_process_table *) list_get(pages_per_process_list, index);
			if (pages_per_process->pid == pid) {
				located = index; // process exists on list
				break;
			}
			index++;
		}
	}

	if (located < 0) {
		// process  doesn't exist on pages per process list
		// creating node for list
		pages_per_process = (t_reg_pages_process_table *) malloc (sizeof(t_reg_pages_process_table));
		pages_per_process->pid = pid;
		pages_per_process->pages_count= 0;
		list_add(pages_per_process_list, pages_per_process);
	}

	// assigning frames for process pages
	int i;
	for (i = 0; i < required_pages; i++) {
		(pages_per_process->pages_count)++;
		inicialize_page(pid, (pages_per_process->pages_count) - 1);
	}

	return SUCCESS;
}

/**
 * @NAME inicialize_page
 * @DESC Asigna un marco (frame) a una página de un proceso (utilizando la tabla de páginas invertida)
 * @PARAMS 						pid : process id
 *				  				 page : nro. página a asignarle un marco (frame)
 */
void inicialize_page(int pid, int page) {
	int free_frame = get_available_frame(); // getting available frame
	t_reg_invert_table * inv_table_aux_ptr = invert_table;
	inv_table_aux_ptr += free_frame;
	inv_table_aux_ptr->pid = pid;
	inv_table_aux_ptr->page = page;
}

/**
 * @NAME get_available_frame
 * @DESC Obtiene un marco disponible (frame)
 */
int get_available_frame(void) {
	int * aux = (int *) list_remove(available_frame_list, 0);
	int frame = * aux;
	free(aux);
	return frame;
}

/**
 * @NAME write_page
 * @DESC
 * @PARAMS client_socket
 */
void write_page(int * client_socket) {
	t_write_request * w_req = write_recv_req(logger, client_socket);
	if (w_req->exec_code == DISCONNECTED_CLIENT) return;

	// TODO : ver protección de memoria para los distintos segmentos

	// getting associated frame
	int frame = get_frame(w_req->pid, w_req->page);
	// getting write position
	char * write_pos = memory_ptr + (frame * (memory_conf->frame_size)) + w_req->offset;
	// writing
	memcpy(&write_pos, w_req->buffer, w_req->size);

	free(w_req->buffer);
	free(w_req);
	write_send_resp(client_socket, SUCCESS);
}

/**
 * @NAME get_frame
 * @DESC Obtiene el nro. de marco (frame) para un proceso y página (buscando en la tabla de páginas invertida)
 * @PARAMS 						pid : process id
 *				  				 page : nro. página del proceso
 */
int get_frame(int pid, int page) {
	// TODO : implementar función hash para realizar la búsqueda dentro de la tabla de páginas invertida
	t_reg_invert_table * inv_table_aux_ptr = invert_table;
	int frame = 0;
	while (frame < (memory_conf->frames)) {
		inv_table_aux_ptr += frame;
		if (((inv_table_aux_ptr->pid) == pid) && ((inv_table_aux_ptr->page) == page)) {
			break;
		} else {
			frame++;
		}
	}
	return frame;
}

/**
 * @NAME read_page
 * @DESC
 * @PARAMS client_socket
 */
void read_page(int * client_socket) {
	t_read_request * r_req = read_recv_req(logger, client_socket);
	if (r_req->exec_code == DISCONNECTED_CLIENT) return;

	void * buff = malloc(r_req->size);
	int frame = get_frame(r_req->pid, r_req->page);
	void * read_pos = memory_ptr +  (frame * memory_conf->frame_size) + r_req->offset;
	memcpy(buff, read_pos, r_req->size);
	free(r_req);

	read_send_resp(client_socket, SUCCESS, r_req->size, buff);
	free(buff);
}

/**
 * @NAME assign_page
 * @DESC
 * @PARAMS client_socket
 */
void assign_page(int * client_socket) {
	// TODO
}

/**
 * @NAME finalize_process
 * @DESC
 * @PARAMS client_socket
 */
void finalize_process(int * client_socket) {
	// TODO
}

void console(void * unused) {
	t_log * console = log_create(memory_conf->consolefile, "MEMORY_CONSOLE", true, LOG_LEVEL_TRACE);
	char * input = NULL;
	char * command = NULL;
	char * param = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&input, &len, stdin)) != -1) {
		if (read > 0) {
			input[read-1] = '\0';

			char * token = strtok(input, " ");
			if (token != NULL) command = token;
			token = strtok(input, " ");
			if (token != NULL) param = token;
			if (strcmp(command, DELAY_CMD) == 0) {
				log_info(console, "DELAY COMMAND");
				log_info(console, "END DELAY COMMAND");
			} else if (strcmp(command, DUMP_CMD) == 0) {
				// DUMP COMMAND
				log_info(console, "DUMP COMMAND");
				if (strcmp(param, DUMP_CACHE_CMD) == 0) {
					log_info(console, "CACHE CONTENT");
					log_info(console, "END CACHE CONTENT");
				} else if (strcmp(param, DUMP_MEMORY_STRUCT_CMD) == 0) {
					log_info(console, "INVERT TABLE");
					log_info(console, "    #FRAME        PID           #PAG");
					log_info(console, "    ══════════    ══════════    ══════════");
					t_reg_invert_table * inv_table_aux_ptr = invert_table;
					int frame = 0;
					while (frame < (memory_conf->frames)) {
						inv_table_aux_ptr += frame;
						log_info(console, "    %10d    %10d    %10d", inv_table_aux_ptr->frame, inv_table_aux_ptr->pid, inv_table_aux_ptr->page);
						frame++;
					}
					log_info(console, "END INVERT TABLE");
				} else if (strcmp(param, DUMP_MEMORY_CONTENT_CMD) == 0) {
					log_info(console, "MEMORY CONTENT");
					char * read_pos = memory_ptr;
					char * frame_content;
					int frame = 0;
					while (frame < memory_conf->frames) {
						read_pos += frame;
						frame_content = malloc(memory_conf->frame_size);
						memcpy(frame_content, read_pos, memory_conf->frame_size);
						log_info(console, frame_content);
						free(frame_content);
						frame++;
					}
					log_info(console, "END MEMORY CONTENT");
				}
				log_info(console, "END DUMP COMMAND");
			} else if (strcmp(command, FLUSH_CMD) == 0) {
				log_info(console, "FLUSH COMMAND");
				log_info(console, "END FLUSH COMMAND");
			} else if (strcmp(command, SIZE_CMD) == 0) {
				// SIZE COMMAND
				log_info(console, "SIZE COMMAND");
				if (strcmp(param, SIZE_MEMORY_CMD) == 0) {
					log_info(console, "MEMORY SIZE");
					log_info(console, "FRAMES ------------> %10d", memory_conf->frames);
					log_info(console, "AVAILABLE FRAMES --> %10d", available_frame_list->elements_count);
					log_info(console, "BUSY FRAMES -------> %10d", (memory_conf->frames) - (available_frame_list->elements_count));
					log_info(console, "END MEMORY SIZE");
				} else {

				}
				log_info(console, "END SIZE COMMAND");
			}
		}
	}
	free(command);
}
