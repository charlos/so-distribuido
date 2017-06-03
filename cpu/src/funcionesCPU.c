/*
 * funcionesCPU.c
 *
 *  Created on: 23/5/2017
 *      Author: gtofaletti
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <shared-library/socket.h>
#include <shared-library/generales.h>
#include <shared-library/memory_prot.h>
#include "cpu.h"
#include "funcionesCPU.h"

extern t_PCB* pcb;
extern int stackPointer;
extern t_page_offset* lastPageOffset;
extern AnSISOP_funciones * funciones;
extern AnSISOP_kernel * func_kernel;
extern t_cpu_conf* cpu_conf;
extern t_log* logger;
extern int pagesize;

int nuevoContexto(){
	t_element_stack* regIndicestack = malloc(sizeof(t_element_stack));

	stack_push(pcb->indice_stack,regIndicestack);
	stackPointer++;
	return list_size(pcb->indice_stack)-1;
}

int agregarAStack(t_args_vars* new_arg_var,int tipo){
	t_element_stack* regContextStack = list_get(pcb->indice_stack,stackPointer);
	int resp;
	switch (tipo) {
	case VAR_STACK:
		arg_var_push(regContextStack->vars, new_arg_var);
		break;
	case ARG_STACK:
		arg_var_push(regContextStack->args, new_arg_var);
		break;
	default:
		resp = -3;
		break;
	}
	return resp;
}

t_link_element* stack_pop(t_stack* stack){
	t_link_element* elemento = list_remove(stack, list_size(stack) - 1);
	return elemento;
}

void stack_push(t_stack* stack, t_element_stack* element){
	list_add(stack, element);
}


t_args_vars* arg_var_pop(t_list* lista){
	t_args_vars *elemento = list_remove(lista, list_size(lista) - 1);
	return elemento;
}

void arg_var_push(t_list* lista, t_args_vars* element){
	list_add(lista, element);
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
	func_kernel->AnSISOP_reservar = alocar;
	func_kernel->AnSISOP_liberar = liberar;
	func_kernel->AnSISOP_wait = wait;
	func_kernel->AnSISOP_signal = signal;
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


int calcularPagina(){
	int page,pc,bytes_codigo;
	//TODO cambiar este valor fijo por el que recibamos durante la recepción del PCB
	int cantInstrucciones = 10;
	bytes_codigo=0;
	t_indice_codigo* icodigo;
	for( pc = 0 ; pc <= cantInstrucciones ; pc++){
		icodigo = ((t_indice_codigo*) pcb->indice_codigo)+ pc;
		bytes_codigo += icodigo->size;
	}
	page=bytes_codigo/pagesize;

	return page;
}

void loadlastPosStack(){
	t_args_vars* lastPosStack = malloc(sizeof(t_args_vars));
	stackPointer = list_size(pcb->indice_stack)-1;
	t_element_stack* lastcontext = list_get(pcb->indice_stack,stackPointer);
	u_int32_t pagina,offset;

	if (list_size(lastcontext->args)>0){
		lastPosStack = list_get(lastcontext->args,list_size(lastcontext->args)-1);
		pagina = lastPosStack->pagina;
		offset = lastPosStack->offset;
	} else{
		pagina = 0;
		offset = 0;
	}

	if (list_size(lastcontext->vars)>0){
		lastPosStack = list_get(lastcontext->vars,list_size(lastcontext->vars)-1);
		if(lastPosStack->pagina > pagina){
			pagina = lastPosStack->pagina;
			offset = lastPosStack->offset;
		}else if(pagina == lastPosStack->pagina){
			if(lastPosStack->offset > offset){
				offset = lastPosStack->offset;
			}
		}
	}

	lastPageOffset->page=pagina;
	lastPageOffset->offset=offset;

	free(lastPosStack);

}

void updatePageAvailable(u_int32_t size){
	int page;
	if (lastPageOffset->offset+size > pagesize){
		lastPageOffset->page=lastPageOffset->page+1;
		lastPageOffset->offset=0;
	}else{
		lastPageOffset->page;
	}

}

