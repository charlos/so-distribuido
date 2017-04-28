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
#include "kernel.h"


int main(int argc, char* argv[]) {

	char * server_ip = "127.0.0.1";
	char * server_port = "5003";
	int listening_socket;

	crear_logger(argv[0], &logger, true, LOG_LEVEL_TRACE);
	log_trace(logger, "Log Creado!!");

	listening_socket = open_socket(10, PUERTO_DE_ESCUCHA);
	manage_select(listening_socket, logger);


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
