/*
 ============================================================================
 Name        : cpu.c
 Authors     : Carlos Flores, Gustavo Tofaletti, Dante Romero
 Version     :
 Description : Kernel Proccess
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <shared-library/socket.h>
#include <shared-library/memory_prot.h>
#include <pthread.h>
#include "kernel.h"
#include "solicitudes.h"



int main(int argc, char* argv[]) {

	cola_listos = queue_create();
	registro_pid = 1;

	crear_logger(argv[0], &logger, false, LOG_LEVEL_TRACE);
	log_trace(logger, "Log Creado!!");

	load_kernel_properties();

	tabla_paginas_heap = list_create();
	memory_socket = connect_to_socket(kernel_conf->memory_ip, kernel_conf->memory_port);

	TAMANIO_PAGINAS = handshake(memory_socket, logger);

//	fs_socket = connect_to_socket(kernel_conf->filesystem_ip, kernel_conf->filesystem_port);


	pthread_t hilo_cpu;
	pthread_t hilo_consola;

	pthread_attr_t attr;
	t_aux *estruc_cpu, *estruc_prog;
	estruc_cpu = malloc(sizeof(t_aux));
	estruc_prog = malloc(sizeof(t_aux));
	estruc_cpu->port = kernel_conf->cpu_port;
	estruc_cpu->master = master_cpu;
	estruc_prog->port = kernel_conf->program_port;
	estruc_prog->master = master_prog;
	pthread_attr_init(&attr);

//	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	// Se crea hilo de cpu's
	pthread_create(&hilo_cpu, NULL, &manage_select, estruc_cpu);

	// Se crea hilo de consolas
	pthread_create(&hilo_consola, NULL, &manage_select, estruc_prog);

//	pthread_attr_destroy(&attr);

	pthread_join(hilo_consola, NULL);
	pthread_join(hilo_cpu, NULL);

	return EXIT_SUCCESS;
}

t_stack* stack_create(){
	t_stack* stack = list_create();
	return stack;
}


t_cpu* cpu_create(int file_descriptor){
	t_cpu* cpu = malloc(sizeof(t_cpu));
	cpu->file_descriptor = file_descriptor;
	cpu->proceso_asignado = NULL;
	return cpu;
}

void manage_select(t_aux* estructura){

	int listening_socket;
	listening_socket = open_socket(20, estructura->port);
	int nuevaConexion, fd_seleccionado, recibido, set_fd_max, i;
	uint8_t* operation_code;
	char buf[512];
	fd_set lectura;
	set_fd_max = listening_socket;
	FD_ZERO(&lectura);
	FD_ZERO(&(estructura->master));
	FD_SET(listening_socket, &(estructura->master));
	while(1){
		lectura = (estructura->master);
		select(set_fd_max +1, &lectura, NULL, NULL, NULL);
		for(fd_seleccionado = 0 ; fd_seleccionado <= set_fd_max ; fd_seleccionado++){
			if(FD_ISSET(fd_seleccionado, &lectura)){
				if(fd_seleccionado == listening_socket){
					if((nuevaConexion = accept_connection(listening_socket)) == -1){
						log_error(logger, "Error al aceptar conexion");
					} else {
						log_trace(logger, "Nueva conexion: socket %d", nuevaConexion);
						FD_SET(nuevaConexion, &(estructura->master));
						if(nuevaConexion > set_fd_max)set_fd_max = nuevaConexion;
					}
				} else {

					solve_request(fd_seleccionado, &(estructura->master));
				}
			}
		}
	}
}

uint8_t handshake_memory(int socket){
	uint8_t op_code, *buffer;
	uint32_t* msg = malloc(sizeof(uint32_t));
	*msg = 1;
	connection_send(socket, HANDSHAKE_OC, msg); //OC_HANDSHAKE_MEMORY
	connection_recv(socket, &op_code, &buffer);
	return *buffer;
}

void handshake_filsesystem(int socket){
	uint8_t op_code;
	char* buffer;
	// TODO Definir handshake en filsesysyem
}
