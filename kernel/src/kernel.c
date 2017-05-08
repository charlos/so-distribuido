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

int memory_socket;
t_kernel_conf* kernel_conf;

int main(int argc, char* argv[]) {

	crear_logger(argv[0], &logger, true, LOG_LEVEL_TRACE);
	log_trace(logger, "Log Creado!!");

	load_kernel_properties();

	pthread_t hilo_cpu;
	pthread_t hilo_consola;

	pthread_attr_t attr;

	pthread_attr_init(&attr);

	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	// Se crea hilo de cpu's
	pthread_create(&hilo_cpu, &attr, &manage_select, kernel_conf->cpu_port);

	// Se crea hilo de consolas
	pthread_create(&hilo_consola, &attr, &manage_select, kernel_conf->program_port);

	pthread_attr_destroy(&attr);

	while(1){
		sleep(10);
	}

	return EXIT_SUCCESS;
}

t_stack* stack_create(){
	t_stack* stack = list_create();
	return stack;
}

t_link_element* stack_pop(t_stack* stack){
	t_link_element* elemento = list_remove(stack, list_size(stack) - 1);
	return elemento;
}

void stack_push(t_stack* stack, void* element){
	list_add(stack, element);
}

t_PCB* crear_PCB(){
	t_PCB* PCB = malloc(sizeof(t_PCB));
	PCB->pid = registro_pid++;
	PCB->cantidad_paginas = 0;
	return PCB;
}

void solicitar_progama_nuevo(int file_descriptor, char* codigo){
	int respuesta;
	uint8_t* operation_code;
	char* buffer; // no importa por ahora

	t_PCB* pcb = crear_PCB();

	connection_send(file_descriptor, OC_SOLICITUD_PROGRAMA_NUEVO, codigo);
	respuesta = connection_recv(file_descriptor, operation_code, &buffer);
	if(respuesta != -1) pcb->cantidad_paginas++;
}

t_cpu* cpu_create(int file_descriptor){
	t_cpu* cpu = malloc(sizeof(t_cpu));
	cpu->file_descriptor = file_descriptor;
	cpu->proceso_asignado = NULL;
	return cpu;
}

void load_kernel_properties(void) {
	t_config * conf = config_create("/home/utnso/workspace/tp-2017-1c-Stranger-Code/kernel/Debug/kernel.cfg");
	kernel_conf = malloc(sizeof(t_kernel_conf));
	kernel_conf->program_port = config_get_string_value(conf, "PUERTO_PROG");
	kernel_conf->cpu_port = config_get_string_value(conf, "PUERTO_CPU");
	kernel_conf->memory_ip = config_get_string_value(conf, "IP_MEMORIA");
	kernel_conf->memory_port = config_get_string_value(conf, "PUERTO_MEMORIA");
	kernel_conf->filesystem_ip = config_get_string_value(conf, "IP_FS");
	kernel_conf->filesystem_port = config_get_string_value(conf, "PUERTO_FS");
	kernel_conf->grado_multiprog = config_get_int_value(conf, "GRADO_MULTIPROG");
	kernel_conf->algoritmo = config_get_string_value(conf, "ALGORITMO");
	kernel_conf->quantum = config_get_int_value(conf, "QUANTUM");
	kernel_conf->quantum_sleep = config_get_int_value(conf, "QUANTUM_SLEEP");
	kernel_conf->stack_size = config_get_int_value(conf, "STACK_SIZE");
	kernel_conf->sem_ids = config_get_array_value(conf, "SEM_IDS");
	kernel_conf->sem_init = config_get_array_value(conf, "SEM_INIT");
	kernel_conf->shared_vars = config_get_array_value(conf, "SHARED_VARS");
}

void manage_select(char* port){

	int listening_socket;
	listening_socket = open_socket(20, port);
	int nuevaConexion, fd_seleccionado, recibido, set_fd_max, i;
	uint8_t* operation_code;
	char buf[512];
	fd_set master, lectura;
	set_fd_max = listening_socket;
	FD_ZERO(&lectura);
	FD_ZERO(&master);
	FD_SET(listening_socket, &master);
	while(1){
		lectura = master;
		select(set_fd_max +1, &lectura, NULL, NULL, NULL);
		for(fd_seleccionado = 0 ; fd_seleccionado <= set_fd_max ; fd_seleccionado++){
			if(FD_ISSET(fd_seleccionado, &lectura)){
				if(fd_seleccionado == listening_socket){
					if((nuevaConexion = accept_connection(listening_socket)) == -1){
						log_error(logger, "Error al aceptar conexion");
					} else {
						log_trace(logger, "Nueva conexion: socket %d", nuevaConexion);
						FD_SET(nuevaConexion, &master);
						if(nuevaConexion > set_fd_max)set_fd_max = nuevaConexion;
					}
				} else {

					operation_code = malloc(sizeof(uint8_t));
					void * buffer;
					int ret = connection_recv(fd_seleccionado, operation_code, &buf);

					if(!ret) {
						FD_CLR(fd_seleccionado, &master);
						close_client(fd_seleccionado);
					}
				}
			}
		}
	}
}
