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
#include <shared-library/socket.h>
#include <thread_db.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <commons/string.h>

#include "memoria.h"

#define	SOCKET_BACKLOG 100

memoria_config * memoria_arch_conf;
t_log * logger;
int listenning_socket;
char * RESP_MSG = "received";


int main(void) {
	load_memory_properties();
	create_logger();
	print_memory_properties();

	int * new_sock;
	listenning_socket = open_socket(SOCKET_BACKLOG, memoria_arch_conf->puerto);

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

void create_logger(void) {
	logger = log_create(memoria_arch_conf->logfile, "MEMORIA", true, LOG_LEVEL_TRACE);
}

void load_memory_properties(void) {
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
}

void print_memory_properties(void) {
	log_info(logger, "Proceso MEMORIA" );
	log_info(logger, "------ PUERTO: %u" , memoria_arch_conf->puerto);
	log_info(logger, "------ MARCOS: %u" , memoria_arch_conf->marcos);
	log_info(logger, "------ MARCO_SIZE: %u" , memoria_arch_conf->marco_size);
	log_info(logger, "------ ENTRADAS_CACHE: %u" , memoria_arch_conf->entradas_cache);
	log_info(logger, "------ CACHE_X_PROC: %u" , memoria_arch_conf->cache_x_proc);
	log_info(logger, "------ RETARDO_MEMORIA: %u" , memoria_arch_conf->retardo);
	log_info(logger, "------ REEMPLAZO_CACHE: %s" , memoria_arch_conf->reemplazo_cache);
	log_info(logger, "------ LOGFILE: %s" , memoria_arch_conf->logfile);
}

void process_request(int * client_socket) {
	// << receiving message >>
	uint8_t op_code;
	uint8_t prot_ope_code_size = 1;
	int received_bytes = socket_recv(client_socket, &op_code, prot_ope_code_size);

	while (received_bytes > 0) {
		log_info(logger, "------ CLIENT %d >> operation code : %d", * client_socket, op_code);

		// msg size
		uint8_t prot_msg_size = 4;
		uint32_t req_msg_size;
		received_bytes = socket_recv(client_socket, &req_msg_size, prot_msg_size);
		if (received_bytes <= 0) {
			log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
			return;
		}
		log_info(logger, "------ CLIENT %d >> message lenght : %d", * client_socket, req_msg_size);
		// msg
		char * msg = malloc(sizeof(char) * (req_msg_size + 1));
		received_bytes = socket_recv(client_socket, msg, req_msg_size);
		if (received_bytes <= 0) {
			log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
			return;
		}
		msg[req_msg_size] = '\0';
		log_info(logger, "------ CLIENT %d >> message : %s", * client_socket, msg);
		free(msg);

		// << sending response >>
		// response msg
		char * resp_msg = string_duplicate(RESP_MSG);
		// response code
		uint8_t prot_resp_code_size = 1;
		uint8_t resp_code = rand() % 20; // random int between 0 and 19
		// response size
		uint32_t prot_resp_size = 4;
		uint32_t resp_size = strlen(resp_msg) + 1;
		int response_size = sizeof(char) * (prot_resp_code_size + prot_resp_size + resp_size);

		void * resp = malloc(response_size);
		memcpy(resp, &resp_code, prot_resp_code_size);
		memcpy(resp + prot_resp_code_size, &resp_size, prot_resp_size);
		memcpy(resp + prot_resp_code_size + prot_resp_size, resp_msg, resp_size);
		socket_write(client_socket, resp, response_size);
		free(resp_msg);
		free(resp);

		received_bytes = socket_recv(client_socket, &op_code, prot_ope_code_size);

	}
	close_client(* client_socket);
	free(client_socket);
	return;
}
