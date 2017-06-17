/*
 * solicitudes.c
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include "solicitudes.h"
#include <parser/metadata_program.h>
#include <parser/parser.h>

void solve_request(int socket, fd_set* set){
	uint8_t operation_code;
	uint32_t cant_paginas, direcc_logica, direcc_fisica;
	t_puntero bloque_heap;
	int status;
	char* buffer;
	t_pedido_reservar_memoria* pedido;
	t_pedido_liberar_memoria* liberar;
	t_pagina_heap* pagina;
	t_read_response* respuesta_pedido_pagina;
	t_PCB* pcb;
	t_metadata_program* metadata;
	status = connection_recv(socket, &operation_code, &buffer);
	if(status <= 0)	FD_CLR(socket, set);

	switch(operation_code){
	case OC_SOLICITUD_PROGRAMA_NUEVO:

		cant_paginas = calcular_paginas_necesarias(buffer);
		pcb = crear_PCB();
		status = 0;
		status = memory_init_process(memory_socket, pcb->pid, cant_paginas, logger);

		if(status == -1){
			log_error(logger, "Se desconecto Memoria");
			exit(1);
		}

		mandar_codigo_a_memoria(buffer, pcb->pid);
		pcb->cantidad_paginas += cant_paginas;
		pcb->PC = 0;

		log_trace(logger, "Mandando PID");
		printf("Mandando PID");

		connection_send(socket, OC_NUEVA_CONSOLA_PID, &(pcb->pid));

		metadata = metadata_desde_literal(buffer);

		pcb->SP = 0;
		pcb->cantidad_instrucciones = metadata->instrucciones_size;
		pcb->indice_codigo = obtener_indice_codigo(metadata);
		pcb->indice_etiquetas = obtener_indice_etiquetas(metadata);
		metadata_destruir(metadata);
		break;
	case OC_FUNCION_RESERVAR:
		pedido = buffer;
		pagina = obtener_pagina_con_suficient_espacio(pedido->pid, pedido->espacio_pedido);
		if(pagina == NULL){
			memory_assign_pages(memory_socket, pedido->pid, 1, logger);
			// TODO: Agregar pagina nueva a tabla de heap
			pagina = obtener_pagina_con_suficient_espacio(pedido->pid, pedido->espacio_pedido);
		}
		respuesta_pedido_pagina = memory_read(memory_socket, pedido->pid, pagina->nro_pagina, 0, TAMANIO_PAGINAS, logger);
		bloque_heap = buscar_bloque_disponible(respuesta_pedido_pagina->buffer, pagina->nro_pagina, pedido->espacio_pedido);


		// Mandamos la pagina de heap modificada
		memory_write(memory_socket, pedido->pid, pagina->nro_pagina, 0, TAMANIO_PAGINAS, TAMANIO_PAGINAS, respuesta_pedido_pagina->buffer, logger);

		// Mandamos puntero al programa que lo pidio
		connection_send(socket, OC_RESP_RESERVAR, bloque_heap);

		break;
	case OC_FUNCION_LIBERAR:
		liberar = buffer;
		liberar->nro_pagina = liberar->posicion / TAMANIO_PAGINAS;
//		sad = liberar->posicion % TAMANIO_PAGINAS;
		respuesta_pedido_pagina = memory_read(memory_socket, liberar->pid, liberar->nro_pagina, 0, TAMANIO_PAGINAS, logger);
		marcar_bloque_libre(respuesta_pedido_pagina->buffer, liberar);
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
	int i = 0, offset = 0, cant_a_mandar = strlen(codigo);
	while(cant_a_mandar > TAMANIO_PAGINAS){
		memory_write(memory_socket, pid, i, 0, TAMANIO_PAGINAS, TAMANIO_PAGINAS, codigo + offset, logger);
		offset += TAMANIO_PAGINAS;
		cant_a_mandar -= TAMANIO_PAGINAS;
		i++;
	}
	if(cant_a_mandar > 0){
		memory_write(memory_socket, pid, i, 0, cant_a_mandar, cant_a_mandar, codigo + offset, logger);
	}
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
		memcpy(value, metadata->etiquetas+offset,sizeof(t_puntero_instruccion));// copio el puntero de instruccion
		offset += sizeof(t_puntero_instruccion);
		dictionary_put(indice_etiquetas, key, *value);
	}
	return indice_etiquetas;
}

t_pagina_heap* obtener_pagina_con_suficient_espacio(int pid, int espacio){
	bool tiene_mismo_pid_y_espacio_disponible(t_pagina_heap* pagina){
		return (pagina->pid == pid && pagina->espacio_libre >= espacio);
	}
	return list_find(tabla_paginas_heap, (void*)tiene_mismo_pid_y_espacio_disponible);
}

t_heapMetadata* leer_metadata(void* pagina){
	t_heapMetadata* new = malloc(sizeof(t_heapMetadata));
	memcpy(&(new->size), pagina, sizeof(uint32_t));
	memcpy(&(new->isFree), ((char*)pagina) + sizeof(uint32_t), sizeof(bool));
	return new;
}

t_puntero buscar_bloque_disponible(void* pagina, int nro_pagina, int espacio_pedido){
	t_heapMetadata* metadata;
	t_puntero posicion_bloque;
	int espacio_total_bloque;
	int offset = 0;
	while(offset < TAMANIO_PAGINAS){
		metadata = leer_metadata(pagina + offset);
		if(!(metadata->isFree) && metadata->size >= (espacio_pedido + sizeof(t_heapMetadata))){		// hay un bloque con suficiente espacio libre
			espacio_total_bloque = metadata->size;
			cambiar_metadata(metadata, espacio_pedido);
			memcpy((char*)pagina + offset, metadata, sizeof(t_heapMetadata));		//se guarda la nueva metadata en la pagina (para despues mandar a Memoria)
			posicion_bloque = offset + (TAMANIO_PAGINAS*nro_pagina);
			offset += sizeof(t_heapMetadata);		//avanza la cantidad de bytes de la metadata
			offset += metadata->size;				//avanza la cantidad de bytes del bloque de datos
			agregar_bloque_libre(pagina, offset);	//se marca lo que queda de la pagina como espacio libre
			return posicion_bloque;
		}
		offset += sizeof(t_heapMetadata);			//avanza la cantidad de bytes de la metadata
		offset += metadata->size;					//avanza la cantidad de bytes del bloque de datos
	}
	return NULL;
}

void cambiar_metadata(t_heapMetadata* metadata, int espacio_pedido){
	metadata->isFree = 0;
	metadata->size = espacio_pedido;

}

void agregar_bloque_libre(void* pagina, int offset){
	int espacio_libre;
	espacio_libre = TAMANIO_PAGINAS - offset;
	t_heapMetadata* metadata_libre = crear_metadata_libre(espacio_libre);
	memcpy((char*)pagina + offset, metadata_libre, sizeof(t_heapMetadata));
}

t_pagina_heap* buscar_pagina_heap(int pid, int nro_pagina){
	bool _pagina_de_programa(t_pagina_heap* pagina){
		return (pagina->pid == pid && pagina->nro_pagina == nro_pagina);
	}
	return list_find(tabla_paginas_heap, (void*) _pagina_de_programa);
}
void marcar_bloque_libre(void* pagina, t_pedido_liberar_memoria* pedido_free){
	int ultimo_size, offset = pedido_free->posicion % TAMANIO_PAGINAS;
	t_heapMetadata* metadata = leer_metadata((char*) pagina + offset);
	metadata->isFree = 1;
	t_pagina_heap* pagina_de_tabla = buscar_pagina_heap(pedido_free->pid, pedido_free->nro_pagina);
	pagina_de_tabla->espacio_libre += metadata->size;
	//TODO correr todos los bloques
	//TODO ver si se libera la pagina
	int posicion_inicial = offset;
	t_heapMetadata* metadata2;
	offset += sizeof(t_heapMetadata);
	offset += metadata->size;
	while(offset < TAMANIO_PAGINAS){
		metadata2 = leer_metadata(pagina + offset);
		if(metadata2->isFree)break;
//		memmove(pagina + posicion_inicial + )
	}
	offset += sizeof(t_heapMetadata);
	offset += metadata->size;
	ultimo_size = metadata->size;
	correr_bloques_desde((char*)pagina + offset);

}

void correr_bloques_desde(void* posicion_inicial, int espacio_restante){
	t_heapMetadata* metadata = leer_metadata(posicion_inicial);
	int offset_to = 0, offset_from = 0;
	offset_from += sizeof(t_heapMetadata);
	offset_from += metadata->size;
	while(espacio_restante > 5){
		metadata = leer_metadata(posicion_inicial + offset_from);
		if(metadata->isFree)break;
		memmove(posicion_inicial + offset_to, posicion_inicial + offset_from, sizeof(t_heapMetadata) + metadata->size);
		espacio_restante -= sizeof(t_heapMetadata) + metadata->size;
	}
}

t_heapMetadata* crear_metadata_libre(uint32_t espacio){
	t_heapMetadata* new = malloc(sizeof(t_heapMetadata));
	new->isFree = 1;
	new->size = espacio - sizeof(t_heapMetadata);
	return new;
}
