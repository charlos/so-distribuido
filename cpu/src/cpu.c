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

AnSISOP_funciones * funciones;
AnSISOP_kernel * func_kernel;
t_cpu_conf* cpu_conf;
t_log* logger;
int pagesize;
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

	//TODO: loop de esto
	pcb = malloc(sizeof(t_PCB));
	uint8_t operation_code;
	connection_recv(server_socket_kernel, &operation_code, pcb);

	int pc, page;

	for( pc = 0 ; pc <= list_size(pcb->indice_codigo) ; pc++){

		if(list_size(pcb->indice_stack)==0){
			//Si el indice del stack está vacio es porque estamos en la primera línea de código, creo la primera línea del scope
			nuevoContexto();
		}
		t_indice_codigo* icodigo = malloc(sizeof(t_indice_codigo));
		icodigo = (t_indice_codigo*)list_get(pcb->indice_codigo, pcb->PC);

		page = calcularPagina(pcb);

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

void inicializarFuncionesParser(void) {
	funciones = malloc(sizeof(AnSISOP_funciones));
	funciones->AnSISOP_definirVariable = definirVariable;
	funciones->AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable;
	funciones->AnSISOP_dereferenciar = dereferenciar;
	funciones->AnSISOP_asignar = asignar;
	funciones->AnSISOP_irAlLabel = irAlLabel;
	funciones->AnSISOP_llamarSinRetorno = llamarSinRetorno;
	funciones->AnSISOP_llamarConRetorno = llamarConRetorno;

	func_kernel = malloc(sizeof(AnSISOP_kernel));
	func_kernel->AnSISOP_abrir = abrir;
	func_kernel->AnSISOP_cerrar = cerrar;
	func_kernel->AnSISOP_borrar = borrar;
	func_kernel->AnSISOP_escribir = escribir;
	func_kernel->AnSISOP_leer = leer;
	func_kernel->AnSISOP_moverCursor = moverCursor;
}
void procesarMsg(char * msg) {

//	char ** lineas = string_split(msg, "\n");
//	int ipointer;
//	for(ipointer = 0; lineas[ipointer] != NULL; ipointer++) {
		analizadorLinea(msg,funciones, func_kernel);
//	}
}

void load_properties(void) {
	t_config * conf = config_create("/home/utnso/workspace/tp-2017-1c-Stranger-Code/cpu/Debug/cpu.cfg");
	cpu_conf = malloc(sizeof(t_cpu_conf));
	cpu_conf->kernel_ip = config_get_string_value(conf, "IP_KERNEL");
	cpu_conf->kernel_port = config_get_string_value(conf, "PUERTO_KERNEL");
	cpu_conf->memory_ip = config_get_string_value(conf, "IP_MEMORIA");
	cpu_conf->memory_port = config_get_string_value(conf, "PUERTO_MEMORIA");
	free(conf);
}

t_link_element* stack_pop(t_stack* stack){
	t_link_element* elemento = list_remove(stack, list_size(stack) - 1);
	return elemento;
}

void stack_push(t_stack* stack, t_element_stack* element){
	list_add(stack, element);
}

//TODO: calcular la pagina donde está la instruccion
int calcularPagina(){
	int page,pc,bytes_codigo;
	bytes_codigo=0;
	t_indice_codigo* icodigo;
	for( pc = 0 ; pc <= pcb->PC ; pc++){
		icodigo = (t_indice_codigo*) list_get(pcb->indice_codigo, pc);
		bytes_codigo += icodigo->size;
	}
	page=bytes_codigo/pagesize;

	return page;
}

int nuevoContexto(){
	t_element_stack* regIndicestack = malloc(sizeof(t_element_stack));
	stack_push(pcb->indice_stack,regIndicestack);

	return list_size(pcb->indice_stack);
}
