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

void procesarMsg(char * msg);

int main(void) {

	//int byte_ejecutados

	crear_logger("/home/utnso/workspace/tp-2017-1c-Stranger-Code/cpu/cpu", &logger, true, LOG_LEVEL_TRACE);
	log_trace(logger, "Log Creado!!");

	load_properties();
	//server_socket_kernel = connect_to_socket(cpu_conf->kernel_ip, cpu_conf->kernel_port);
	server_socket_memoria = connect_to_socket(cpu_conf->memory_ip, cpu_conf->memory_port);
	inicializarFuncionesParser();
	pagesize = handshake(server_socket_memoria, logger);

	//TODO: loop de esto
	t_PCB* pcb = malloc(sizeof(t_PCB));
	uint8_t operation_code;
	connection_recv(server_socket_kernel, &operation_code, pcb);

	int pc, page;
	for( pc = 0 ; pc <= list_size(pcb->indice_codigo) ; pc++){

		t_indice_codigo* icodigo = malloc(sizeof(t_indice_codigo));
		icodigo = (t_indice_codigo*)list_get(pcb->indice_codigo, pcb->PC);

		page = calcularPagina(icodigo);

		t_read_response * read_response = memory_read(server_socket_memoria, pcb->pid, page, icodigo->offset, icodigo->size, logger);

		char * instruccion;
		strcpy(instruccion, read_response->buffer);
		t_element_stack* regIndicestack = malloc(sizeof(t_element_stack));
		stack_push(pcb->indice_stack,regIndicestack);
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
int calcularPagina(t_indice_codigo* icodigo){
	int page;

	page=0;

	return page;
}

uint8_t handshake_memory(int socket){
	uint8_t op_code, *buffer;
	uint32_t* msg = malloc(sizeof(uint32_t));
	*msg = 1;
	connection_send(server_socket_memoria, OC_HANDSHAKE_MEMORY, msg);
	connection_recv(server_socket_memoria, &op_code, &buffer);
	return *buffer;
}

