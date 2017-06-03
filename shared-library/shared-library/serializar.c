/*
 ============================================================================
 Name        : serializar.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "serializar.h"
/* TEST
int main(void) {
	t_stream* stream;
	puts("PCB Serializer\n");
	//inicializar PCB
	t_PCB* pcb = malloc(sizeof(t_PCB));
	t_indice_codigo* elem_1 = malloc(sizeof(t_indice_codigo));
	elem_1->offset = 911;
	elem_1->size = 912;
	t_indice_codigo* elem_2 = malloc(sizeof(t_indice_codigo));
	elem_2->offset = 921;
	elem_2->size = 922;
	t_indice_codigo* elem_3 = malloc(sizeof(t_indice_codigo));
	elem_3->offset = 931;
	elem_3->size = 932;

	pcb->pid = 111;
	pcb->PC = 22;
	pcb->cantidad_paginas = 33;
	pcb->exit_code = -4;
	pcb->stack_pointer = 7;
	pcb->cantidad_instrucciones = 10;
	pcb->indice_codigo = malloc(sizeof(t_indice_codigo));
	pcb->indice_codigo = elem_1;
	pcb->indice_stack = list_create();
		t_element_stack* elem_stack_1 = malloc(sizeof(t_element_stack));
		elem_stack_1->retPos = 666;
		elem_stack_1->retVar = malloc(sizeof(posicion_memoria));
		elem_stack_1->retVar->offset = 6661;
		elem_stack_1->retVar->pagina = 6662;
		elem_stack_1->retVar->size = 6663;
		elem_stack_1->args = list_create();
		t_args_vars* elem_arg_1 = malloc(sizeof(t_args_vars));
			elem_arg_1->id = 7771;
			elem_arg_1->offset = 7772;
			elem_arg_1->pagina = 7773;
			elem_arg_1->size = 7774;
		list_add(elem_stack_1->args, elem_arg_1);
		elem_stack_1->vars = list_create();
		t_args_vars* elem_var_1 = malloc(sizeof(t_args_vars));
			elem_var_1->id = 7781;
			elem_var_1->offset = 7782;
			elem_var_1->pagina = 7783;
			elem_var_1->size = 7784;
		list_add(elem_stack_1->vars, elem_var_1);
	list_add(pcb->indice_stack, elem_stack_1);
	//list_add(pcb->indice_stack, elem_stack_2);
	//list_add(pcb->indice_stack, elem_stack_3);

	stream = pcb_serializer(pcb);

	//escribimos en el archivo
 	FILE *fp;
 	fp = fopen("fichero.txt", "wb+");
 	fwrite(&stream->length, sizeof(int), 1, fp);
 	fwrite(stream->data, sizeof(char), stream->length, fp);
 	fclose (fp);
 	puts("END\n");
 	//getchar();

 	//leemos el archivo
 	t_stream* stream_2 = malloc(sizeof(t_stream));

 	puts("LEER ARCHIVO");
 	FILE *file;
 	file = fopen("fichero.txt", "rb");
 	fread(&stream_2->length, (sizeof(uint16_t)), 1, file);
 	stream_2->data = malloc(stream_2->length);
 	fread(stream_2->data, sizeof(char), stream_2->length, file);
 	fclose (file);
 	//stream_2->data = data;

 	t_PCB* pcb_recv = deserializer_pcb(stream_2->data);

 	puts("END\n");

 	return EXIT_SUCCESS;
}
*/
t_PCB* deserializer_pcb(char* buffer){
	t_PCB* pcb = malloc(sizeof(t_PCB));
	memcpy(&pcb->pid, buffer, 2);
 	memcpy(&pcb->PC, buffer+2, 2);
 	memcpy(&pcb->cantidad_paginas, buffer+4, 2);
 	memcpy(&pcb->exit_code, buffer+6, 2);
 	uint16_t lengthIndiceCodigo;
 	memcpy(&lengthIndiceCodigo, buffer+8, 2);

 	pcb->indice_codigo = malloc(lengthIndiceCodigo);
 	uint16_t cantElementos = lengthIndiceCodigo/sizeof(t_indice_codigo);
 	int i;
 	for (i = 0; i < cantElementos; ++i) {
 		memcpy(pcb->indice_codigo+i, buffer+10, 8);
	}

 	return pcb;
}

t_stream* pcb_serializer(t_PCB* pcb){
	t_stream* stream_indiceCodigo = indiceCodigo_serializer(pcb->indice_codigo);
	t_stream* stream_indiceStack = indiceStack_serializer(pcb->indice_stack);
	uint16_t tamElementos = sizeof(int)+sizeof(t_PCB)+stream_indiceCodigo->length+stream_indiceStack->length;
	char* data = malloc(tamElementos);
	int offset = 0;
	memcpy(data+offset, &pcb->pid, offset += sizeof(uint16_t));
	memcpy(data+offset, &pcb->PC, offset += sizeof(uint16_t));
	memcpy(data+offset, &pcb->cantidad_paginas, offset += sizeof(uint16_t));
	memcpy(data+offset, &pcb->exit_code, offset += sizeof(uint16_t));
	memcpy(data+offset, &pcb->cantidad_instrucciones, offset += sizeof(uint16_t));
	memcpy(data+offset, &stream_indiceCodigo->length, offset += sizeof(int));
	memcpy(data+offset, stream_indiceCodigo->data, offset += stream_indiceCodigo->length);
	memcpy(data+offset, &stream_indiceStack->length, offset += sizeof(int));
	memcpy(data+offset, stream_indiceStack->data, offset += stream_indiceStack->length);

	t_stream* stream = malloc(sizeof(t_stream));
	stream->length = tamElementos;
	stream->data = data;

	return stream;
}

t_stream* indiceCodigo_serializer(t_indice_codigo* indiceCodigo){
	int cantElementos = 1; //indice_codigo_size(indiceCodigo);
	int tamElementos = (sizeof(t_indice_codigo)*cantElementos);
	char* data = malloc(tamElementos);
	//t_indice_codigo* iCodigo = list_get(indiceCodigo, 0);
	int i;
	for (i = 0; i < cantElementos; ++i) {
		memcpy(data, indiceCodigo+i, tamElementos);
	}

	t_stream* stream = malloc(sizeof(t_stream));
	stream->length = tamElementos;
	stream->data = data;

	return stream;
}

t_stream* indiceStack_serializer(t_stack* indiceStack){
	int cantElementos = list_size(indiceStack);

	//t_indice_codigo* iCodigo = list_get(indiceCodigo, 0);
	int i;
	int offset = 0;
	char* data;
	char* dataAux;
	for (i = 0; i < cantElementos; ++i) {
		t_element_stack* elementStack = list_get(indiceStack, i);
		t_stream* stream_elementStack = elementStack_serializer(elementStack);
		//int offset = (sizeof(uint16_t)+stream_elementStack->length)*i;
		dataAux = malloc(offset+sizeof(int)+stream_elementStack->length);
		if(offset > 0){
			memcpy(dataAux, data, offset);
			free(data);
		}
		memcpy(dataAux+offset, &stream_elementStack->length, offset += sizeof(int));
		memcpy(dataAux+offset, stream_elementStack->data, offset += stream_elementStack->length);

		data = malloc(offset);
		memcpy(data, dataAux, offset);
		free(dataAux);
	}
	int zero = 0;
	t_stream* stream = malloc(sizeof(t_stream));
	stream->length = offset+sizeof(int);
	stream->data = malloc(offset+sizeof(int));
	memcpy(stream->data, data, offset);
	memcpy(stream->data+offset, &zero, sizeof(int));//se agrega "length = 0" como fin de la lista
	//free(data);

	return stream;
}

t_stream* elementStack_serializer(t_element_stack* elementStack){
	t_stream* stream_args = argsVars_serializer(elementStack->args);
	t_stream* stream_vars = argsVars_serializer(elementStack->vars);
	int tamElementos = 3*sizeof(int)+sizeof(posicion_memoria)+stream_args->length+stream_vars->length;
	int offset = 0;
	char* data = malloc(tamElementos);
	memcpy(data, &elementStack->retPos, offset+=sizeof(int));
	memcpy(data+offset, &elementStack->retVar, offset+=sizeof(posicion_memoria));
	memcpy(data+offset, &stream_args->length, offset+=sizeof(int));
	memcpy(data+offset, stream_args->data, offset+=stream_args->length);
	memcpy(data+offset, &stream_vars->length, offset+=sizeof(int));
	memcpy(data+offset, stream_vars->data, offset+=stream_vars->length);

	t_stream* stream = malloc(sizeof(t_stream));
	stream->length = offset;
	stream->data = data;

	return stream;
}

t_stream* argsVars_serializer(t_list* argsVars){
	int cantElementos = list_size(argsVars);
	int tamElementos = (sizeof(t_args_vars)*cantElementos);
	char* data = malloc(tamElementos);
	//t_indice_codigo* iCodigo = list_get(indiceCodigo, 0);
	int i;
	for (i = 0; i < cantElementos; ++i) {
		memcpy(data+(i*sizeof(t_args_vars)), list_get(argsVars, i), sizeof(t_args_vars));
	}

	t_stream* stream = malloc(sizeof(t_stream));
	stream->length = tamElementos;
	stream->data = data;

	return stream;
}

/*int indice_codigo_size(t_indice_codigo* indiceCodigo){
	int i = 0;
	while(indiceCodigo[i] != "\0\0\0\0\0\0\0\0"){
		i++;
	}
	return i;
}*/

