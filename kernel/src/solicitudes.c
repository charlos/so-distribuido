/*
 * solicitudes.c
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include "solicitudes.h"
#include <parser/metadata_program.h>

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

		pcb->cantidad_paginas += cant_paginas;

		connection_send(socket, OC_NUEVA_CONSOLA_PID, &(pcb->pid));
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


void mandar_codigo_a_memoria(char* codigo){
	int paginas_codigo = calcular_paginas_de_codigo(codigo);
	int i, offset = 0;
	void* buffer = malloc(strlen(codigo));
	for(i=0; i < (paginas_codigo-1); i++){
		memcpy(buffer, codigo + offset, TAMANIO_PAGINAS);
		connection_send(memory_socket, OC_CODIGO, buffer);
		offset += TAMANIO_PAGINAS;
	}
	if(strlen(codigo) % TAMANIO_PAGINAS){
		int bytes_restantes = strlen(codigo) - (paginas_codigo * TAMANIO_PAGINAS);
		memcpy(buffer, codigo + offset, bytes_restantes);
	} else {
		memcpy(buffer, codigo + offset, TAMANIO_PAGINAS);
	}
	connection_send(memory_socket, OC_CODIGO, buffer);
}


t_indice_codigo* obtener_indice_codigo(char* codigo){
	t_metadata_program* metadata = metadata_desde_literal(codigo);
	int i = 0;
	t_indice_codigo* indice_codigo = malloc(sizeof(t_indice_codigo) * metadata->instrucciones_size);
	for(i = 0; i < metadata->instrucciones_size; i++){
		memcpy((indice_codigo + i), (metadata->instrucciones_serializado )+ i, sizeof(t_indice_codigo));
	}
	return indice_codigo;
}
