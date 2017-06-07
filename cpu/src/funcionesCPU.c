/*
 * funcionesCPU.c
 *
 *  Created on: 23/5/2017
 *      Author: gtofaletti
 */

#include <stdio.h>
#include <stdlib.h>
#include <parser/parser.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <shared-library/socket.h>
#include <shared-library/generales.h>
#include <shared-library/memory_prot.h>
#include "cpu.h"
#include "funcionesCPU.h"

extern t_PCB* pcb;
//extern int stackPointer;
extern t_page_offset* nextPageOffsetInStack;
extern AnSISOP_funciones * funciones;
extern AnSISOP_kernel * func_kernel;
extern t_cpu_conf* cpu_conf;
extern t_log* logger;
extern int pagesize;

t_element_stack* nuevoContexto(){
	t_element_stack* regIndicestack = malloc(sizeof(t_element_stack));

	regIndicestack->args = list_create();
	regIndicestack->vars = list_create();

	stack_push(pcb->indice_stack,regIndicestack);

	return regIndicestack;
}

int agregarAStack(t_args_vars* new_arg_var,int tipo){
	t_element_stack* regContextStack = list_get(pcb->indice_stack,pcb->SP);
	int resp;
	switch (tipo) {
	case VAR_STACK:
		arg_var_push(regContextStack->vars, new_arg_var);
		break;
	case ARG_STACK:
		arg_var_push(regContextStack->args, new_arg_var);
		break;
	default:
		resp = -1;
		break;
	}
	return resp;
}

t_link_element* stack_pop(t_stack* stack){
	t_link_element* elemento = list_remove(stack, list_size(stack) - 1);
	pcb->SP--;
	return elemento;
}

void stack_push(t_stack* stack, t_element_stack* element){
	list_add(stack, element);
	pcb->SP++;
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
	funciones->AnSISOP_finalizar = finalizar;

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
		analizadorLinea(msg,funciones, func_kernel);
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


int calcularPaginaProxInstruccion(){
	int page,pc,bytes_codigo;

	bytes_codigo=0;
	t_indice_codigo* icodigo;
	for( pc = 0 ; pc <= pcb->PC ; pc++){
		icodigo = ((t_indice_codigo*) pcb->indice_codigo)+ pc;
		bytes_codigo += icodigo->size;
	}
	page=bytes_codigo/pagesize;

	return page;
}


void getNextPosStack(){
	t_args_vars* lastPosStack = malloc(sizeof(t_args_vars));
	u_int32_t pagina,offset, size;
	if (pcb->SP!=0){
	t_element_stack* lastcontext = list_get(pcb->indice_stack,pcb->SP);

	if (list_size(lastcontext->args)>0){
		lastPosStack = list_get(lastcontext->args,list_size(lastcontext->args)-1);
		pagina = lastPosStack->pagina;
		offset = lastPosStack->offset;
		size = lastPosStack->size;
	} else{
		pagina = pcb->cantidad_paginas;
		offset = 0;
		size = 0;
	}

	if (list_size(lastcontext->vars)>0){
		lastPosStack = list_get(lastcontext->vars,list_size(lastcontext->vars)-1);
		if(lastPosStack->pagina > pagina){
			pagina = lastPosStack->pagina;
			offset = lastPosStack->offset;
			size = lastPosStack->size;
		}else if(pagina == lastPosStack->pagina){
			if(lastPosStack->offset > offset){
				offset = lastPosStack->offset;
				size = lastPosStack->size;
			}
		}
	}
	} else {
		pagina = pcb->cantidad_paginas;
		offset = 0;
		size = 0;
	}

	nextPageOffsetInStack->page=pagina;
	nextPageOffsetInStack->offset=offset;
	updatePageOffsetAvailable(size);

	free(lastPosStack);

}

void updatePageOffsetAvailable(u_int32_t size){
	if (nextPageOffsetInStack->offset+size > pagesize){
		nextPageOffsetInStack->page=nextPageOffsetInStack->page+1;
		nextPageOffsetInStack->offset=0;
	}else{
		nextPageOffsetInStack->offset=nextPageOffsetInStack->offset+size;
	}
}

u_int32_t getPageofPos(t_puntero pos){
	return pos/pagesize;
}

u_int32_t getOffsetofPos(t_puntero pos){
	u_int32_t page = getPageofPos(pos);
	return pos - (page * pagesize);
}

t_PCB* crear_PCB_Prueba(){
	pcb = malloc(sizeof(t_PCB));
	pcb->pid = 1;

	char* PROGRAMA =
			"#!/usr/bin/ansisop\n"
			"begin\n"
			"variables a, b\n"
			"a = 3\n"
			"b = 5\n"
			"a = b + 12\n"
			"end\n"
			"\n";
	char *programa = strdup(PROGRAMA);

	pcb->cantidad_paginas = 1;
	pcb->PC = 0;

	t_metadata_program* metadata = metadata_desde_literal(programa);

	pcb->SP = 0;
	pcb->cantidad_instrucciones = metadata->instrucciones_size;
	pcb->indice_codigo = obtener_indice_codigo(metadata);
	pcb->indice_etiquetas = obtener_indice_etiquetas(metadata);

	pcb->indice_stack = list_create();

	metadata_destruir(metadata);
	// Mandar Codigo a memoria

	return pcb;
}



t_indice_codigo* obtener_indice_codigo(t_metadata_program* metadata){
	int i = 0;
	log_trace(logger, "Dentro de obtener_indice_codigo");
	t_indice_codigo* indice_codigo = malloc(sizeof(t_indice_codigo) * metadata->instrucciones_size);
	for(i = 0; i < metadata->instrucciones_size; i++){
		memcpy((indice_codigo + i), (metadata->instrucciones_serializado )+ i, sizeof(t_indice_codigo));
		log_trace(logger, "Instrucción nro %d: offset %d, size %d", i,(indice_codigo + i)->offset, (indice_codigo + i)->size);
	}
	return indice_codigo;
}

t_dictionary* obtener_indice_etiquetas(t_metadata_program* metadata){
	t_dictionary* indice_etiquetas = dictionary_create();
	char* key;
	int *value, offset = 0;
	value = malloc(sizeof(t_puntero_instruccion));
	int i, cantidad_etiquetas_total = metadata->cantidad_de_etiquetas + metadata->cantidad_de_funciones;	// cantidad de tokens que espero sacar del bloque de bytes
	for(i=0; i < cantidad_etiquetas_total; i++){
		int cant_letras_token = 0;
		while(metadata->etiquetas[cant_letras_token + offset] != '\0')cant_letras_token++;
		key = malloc(cant_letras_token + 1);
		memcpy(key, metadata->etiquetas + offset, cant_letras_token + 1);		// copio los bytes de metadata->etiquetas desplazado las palabras que ya copie
		offset += cant_letras_token + 1;										// el offset suma el largo de la palabra + '\0'
		memcpy(value, metadata->etiquetas+offset,sizeof(t_puntero_instruccion)); //	copio el puntero de instruccion
		offset += sizeof(t_puntero_instruccion);
		dictionary_put(indice_etiquetas, key, (void*)value);
	}
	return indice_etiquetas;
}

