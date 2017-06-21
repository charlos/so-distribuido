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

//int stackPointer;
t_page_offset* nextPageOffsetInStack;
t_PCB* pcb;

int server_socket_kernel, server_socket_memoria;

void procesarMsg(char * msg);

int main(void) {

	//int byte_ejecutados

	crear_logger("/home/utnso/workspace/tp-2017-1c-Stranger-Code/cpu/cpu", &logger, true, LOG_LEVEL_TRACE);
	log_trace(logger, "Log Creado!!");

	/*uint8_t a;
	uint32_t b;
	a=1;
	b=999999;
	*/

	load_properties();
	server_socket_kernel = connect_to_socket(cpu_conf->kernel_ip, cpu_conf->kernel_port);
	server_socket_memoria = connect_to_socket(cpu_conf->memory_ip, cpu_conf->memory_port);
	inicializarFuncionesParser();
	pagesize = handshake(server_socket_memoria, logger);

	log_trace(logger, "Socket kernel: %i \n", server_socket_kernel);

	if (pagesize>0){
		log_trace(logger, "Handshake con Memoria. El tamaño de la página es %d",pagesize);
	} else {
		log_trace(logger, "Problema con Handshake con Memoria.");
	}

	/*while(1) {
		pcb = malloc(sizeof(t_PCB));
		uint8_t operation_code;

		connection_recv(server_socket_kernel, &operation_code, pcb);

		int pc, page, cantInstrucciones;

		cantInstrucciones = pcb->cantidad_paginas;
		lastPageOffset = malloc(sizeof(lastPageOffset));

		loadlastPosStack();

		if(list_size(pcb->indice_stack) == 0) {

			//Si el indice del stack está vacio es porque estamos en la primera línea de código, creo la primera línea del scope
			nuevoContexto();
		}

		t_indice_codigo * icodigo = malloc(sizeof(t_indice_codigo));
		icodigo = ((t_indice_codigo*) pcb->indice_codigo)+pc;

		//TODO ver de cambiar la esctructura indice de codigo
		page = calcularPagina();

		//pido leer la instruccion a la memoria
		t_read_response * read_response = memory_read(server_socket_memoria, pcb->pid, page, icodigo->offset, icodigo->size, logger);

		char * instruccion;
		strcpy(instruccion, read_response->buffer);

		procesarMsg(instruccion);

		free(instruccion);
		free(read_response->buffer);
		free(read_response);
		free(icodigo);

		pcb->PC++;

		connection_send(server_socket_kernel, OC_PCB, pcb);
	}*/

	//TODO: loop de esto y dentro del loop el reciv para quedar a la espera de que kernel nos envíe un pcb
	// una vez que recibimos procesamos una línea, devolvemos el pbc y quedamos a la espera de recibir el proximo
//	pcb = malloc(sizeof(t_PCB));
	uint8_t operation_code;
	//connection_recv(server_socket_kernel, &operation_code, pcb);

	pcb = crear_PCB_Prueba();

	int pc, page, offset, pageend, size_to_read;
	char* instruccion;
	nextPageOffsetInStack = malloc(sizeof(t_page_offset));
	getNextPosStack();  // Actualizo la variable nextPageOffsetInStack guardando page/offset de la proxima ubicación a utilizar en el stack

	//Se incrementa Program Counter para comenzar la ejecución
	//pcb->PC++;

	//for( pc = pcb->PC ; pc <= pcb->cantidad_instrucciones ; pc++){
	while (pcb->PC < pcb->cantidad_instrucciones){
		if(list_size(pcb->indice_stack)==0){
			//Si el indice del stack está vacio es porque estamos en la primera línea de código, creo la primera línea del scope
			nuevoContexto();
			pcb->SP--; //Al crear un nuevo contexto se incrementó el StackPointer, pero en este caso, cuando no había contexto alguno, corresponde que SP
					   //quede en 0 al ser el primer contexto de ejecución.
		}
		t_indice_codigo* icodigo = malloc(sizeof(t_indice_codigo));
		t_read_response * read_response;
		t_read_response * read_response2;
		memcpy(icodigo, ((t_indice_codigo*) pcb->indice_codigo)+pcb->PC, sizeof(t_indice_codigo));

		instruccion = malloc(icodigo->size);

		page = calcularPaginaProxInstruccion();
		offset = icodigo->offset-(page*pagesize);
		pageend = (icodigo->offset+icodigo->size)/pagesize;

		//para el caso en que la instrucción enté partida en dos páginas
		if(page<pageend){
			size_to_read = (page+1)*pagesize-icodigo->offset;
		}else{
			size_to_read = icodigo->size;
		}

		//pido leer la instruccion a la memoria
		read_response = memory_read(server_socket_memoria, pcb->pid, page,offset , size_to_read, logger);
		if (read_response->exec_code!=1){
			log_error(logger, "Error al leer de memoria (Page [%d] |Offset [%d] |Size [%d])", page, icodigo->offset, size_to_read);
		}

		memcpy(instruccion,read_response->buffer,read_response->buffer_size);

		if (page<pageend){
			read_response2 = memory_read(server_socket_memoria, pcb->pid, pageend,0 , icodigo->size-size_to_read, logger);
			if (read_response2->exec_code!=1){
				log_error(logger, "Error al leer de memoria (Page [%d] |Offset [%d] |Size [%d])", pageend, 0,icodigo->size-size_to_read);
			}
			memcpy(instruccion+read_response->buffer_size,read_response2->buffer,read_response2->buffer_size);
		}

		instruccion[(icodigo->size) - 1] = '\0';


		log_trace(logger, "Evaluando instruccion: %s",instruccion);

		procesarMsg(instruccion);

		free(instruccion);
		free(read_response->buffer);
		free(read_response);
		if(page<pageend){
			free(read_response2->buffer);
			free(read_response2);
		}
		free(icodigo);

		pcb->PC++;
	}

	return EXIT_SUCCESS;

}
