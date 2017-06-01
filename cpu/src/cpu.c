/*
 ============================================================================
 Name        : cpu.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <shared-library/socket.h>
#include <parser/metadata_program.h>
#include "cpu.h"
#include "funcionesCPU.h"

AnSISOP_funciones * funciones;
AnSISOP_kernel * func_kernel;
t_cpu_conf* cpu_conf;
t_log* logger;
int pagesize;
int stackPointer;
int lastPageOffset[1][2];
t_PCB* pcb;

void procesarMsg(char * msg);

int main(void) {

	//int byte_ejecutados

	crear_logger("/home/utnso/workspace/tp-2017-1c-Stranger-Code/cpu/cpu", &logger, true, LOG_LEVEL_TRACE);
	log_trace(logger, "Log Creado!!");

	load_properties();
	server_socket_kernel = connect_to_socket(cpu_conf->kernel_ip, cpu_conf->kernel_port);
	server_socket_memoria = connect_to_socket(cpu_conf->memory_ip, cpu_conf->memory_port);
	inicializarFuncionesParser();
	pagesize = handshake(server_socket_memoria, logger);

	if (pagesize>0){
		log_trace(logger, "Handshake con Memoria. El tamaño de la página es %d",pagesize);
	} else {
		log_trace(logger, "Problema con Handshake con Memoria.");
	}

	//TODO: loop de esto y dentro del loop el reciv para quedar a la espera de que kernel nos envíe un pcb
	// una vez que recibimos procesamos una línea, devolvemos el pbc y quedamos a la espera de recibir el proximo
	pcb = malloc(sizeof(t_PCB));
	uint8_t operation_code;
	connection_recv(server_socket_kernel, &operation_code, pcb);

	int pc, page;
	loadlastPosStack();

	for( pc = 0 ; pc <= list_size(pcb->indice_codigo) ; pc++){

		if(list_size(pcb->indice_stack)==0){
			//Si el indice del stack está vacio es porque estamos en la primera línea de código, creo la primera línea del scope
			nuevoContexto();
		}
		t_indice_codigo* icodigo = malloc(sizeof(t_indice_codigo));
		icodigo = (t_indice_codigo*)list_get(pcb->indice_codigo, pcb->PC);
//TODO ver de cambiar la esctructura indice de codigo
		page = calcularPagina();

		//pido leer la instruccion a la memoria
		t_read_response * read_response = memory_read(server_socket_memoria, pcb->pid, page, icodigo->offset, icodigo->size, logger);

		char * instruccion;
		strcpy(instruccion, read_response->buffer);

		procesarMsg(instruccion);

		pcb->PC++;
		free(instruccion);
		free(read_response->buffer);
		free(read_response);
		free(icodigo);

	}

	return EXIT_SUCCESS;

}
