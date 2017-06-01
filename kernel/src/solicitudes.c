/*
 * solicitudes.c
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include "solicitudes.h"
#include <parser/metadata_program.h>
#include <parser/parser.h>

void solve_request(int socket){
	uint8_t operation_code;
	char* buffer;
	connection_recv(socket, &operation_code, &buffer);

	switch(operation_code){
	case OC_SOLICITUD_PROGRAMA_NUEVO:{
		t_PCB* pcb;
		uint32_t cant_paginas;
		cant_paginas = calcular_paginas_necesarias(buffer);
		pcb = crear_PCB();
		int response = 0;
		response = memory_init_process(memory_socket, pcb->pid, cant_paginas, logger);

		if(response == -1){
			exit(1);
		}

		mandar_codigo_a_memoria(buffer, pcb->pid);
		pcb->cantidad_paginas += cant_paginas;
		pcb->PC = 0;


		connection_send(socket, OC_NUEVA_CONSOLA_PID, &(pcb->pid));

		t_metadata_program* metadata = metadata_desde_literal(buffer);


		pcb->indice_codigo = obtener_indice_codigo(metadata);
		pcb->indice_etiquetas = obtener_indice_etiquetas(metadata);
//		queue_push(cola_listos, pcb);
		metadata_destruir(metadata);
		// Mandar Codigo a memoria
	}


	}
}

int calcular_paginas_de_codigo(char* codigo){
	int tamanio_codigo, paginas;
	tamanio_codigo = strlen(codigo);
	paginas = tamanio_codigo / TAMANIO_PAGINAS;
	if((paginas * TAMANIO_PAGINAS) < tamanio_codigo) return paginas++;
	return paginas;
}

int calcular_paginas_necesarias(char* codigo){
	int paginas_de_codigo = calcular_paginas_de_codigo(codigo);
	return paginas_de_codigo + kernel_conf->stack_size;
}


void mandar_codigo_a_memoria(char* codigo, int pid){
	int paginas_codigo = calcular_paginas_de_codigo(codigo);
	int i, offset = 0;
	void* buffer = malloc(strlen(codigo));
	for(i=0; i < (paginas_codigo-1); i++){
		memcpy(buffer, codigo + offset, TAMANIO_PAGINAS);
		memory_write(memory_socket, pid, i, 0, TAMANIO_PAGINAS, TAMANIO_PAGINAS, buffer, logger);
//		connection_send(memory_socket, OC_CODIGO, buffer);
		offset += TAMANIO_PAGINAS;
	}
	if(strlen(codigo) % TAMANIO_PAGINAS){
		int bytes_restantes = strlen(codigo) - (paginas_codigo * TAMANIO_PAGINAS);
		memcpy(buffer, codigo + offset, bytes_restantes);
		memory_write(memory_socket, pid, i+1, 0, bytes_restantes, bytes_restantes, buffer, logger);
	} else {
		memcpy(buffer, codigo + offset, TAMANIO_PAGINAS);
		memory_write(memory_socket, pid, i+1, 0, TAMANIO_PAGINAS, TAMANIO_PAGINAS, buffer, logger);
	}
//	connection_send(memory_socket, OC_CODIGO, buffer);
}


t_indice_codigo* obtener_indice_codigo(t_metadata_program* metadata){
	int i = 0;
	t_indice_codigo* indice_codigo = malloc(sizeof(t_indice_codigo) * metadata->instrucciones_size);
	for(i = 0; i < metadata->instrucciones_size; i++){
		memcpy((indice_codigo + i), (metadata->instrucciones_serializado )+ i, sizeof(t_indice_codigo));
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
		dictionary_put(indice_etiquetas, key, *value);
	}
	return indice_etiquetas;
}
