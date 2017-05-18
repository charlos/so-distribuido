/*
 * solicitudes.c
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include "solicitudes.h"

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
		memory_init_process(memory_socket, pcb->pid, cant_paginas);
		int response = 0;
		response = memory_init_process_recv_resp(memory_socket);
		if(response == -1){
			exit(1);
		}
		pcb->cantidad_paginas += cant_paginas;
		// Mandar Codigo a memoria
	}


	}
}

int calcular_paginas_necesarias(char* codigo){
	int tamanio_codigo, paginas_de_codigo;
	tamanio_codigo = strlen(codigo);
	paginas_de_codigo = tamanio_codigo / TAMANIO_PAGINAS;
	return paginas_de_codigo + kernel_conf->stack_size;
}
