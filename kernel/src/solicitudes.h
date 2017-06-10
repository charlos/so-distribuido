/*
 * solicitudes.h
 *
 *  Created on: 16/5/2017
 *      Author: utnso
 */

#ifndef SOLICITUDES_H_
#define SOLICITUDES_H_

#include <shared-library/memory_prot.h>
#include <shared-library/socket.h>
#include <shared-library/generales.h>
#include "kernel_generales.h"

typedef struct{
	int pid;
	int nro_pagina;
	int espacio_libre;
}t_pagina_heap;

typedef struct{
	uint32_t size;
	bool isFree;
}t_heapMetadata;

typedef struct{
	uint8_t pid;
	uint16_t espacio_pedido;
}t_pedido_reservar_memoria;

typedef struct{
	uint8_t pid;
	uint8_t nro_pagina;
	uint16_t posicion;
}t_pedido_liberar_memoria;

t_list* tabla_paginas_heap;
void mandar_codigo_a_memoria(char* codigo, int pid);
t_pagina_heap* obtener_pagina_con_suficient_espacio(int pid, int espacio);
t_indice_codigo* obtener_indice_codigo(t_metadata_program* metadata);
t_dictionary* obtener_indice_etiquetas(t_metadata_program* metadata);
t_puntero buscar_bloque_disponible(void* pagina,int nro_pagina, int espacio_pedido);
t_heapMetadata* crear_metadata_libre(uint32_t espacio);
void cambiar_metadata(t_heapMetadata* metadata, int espacio_pedido);
void agregar_bloque_libre(void* pagina, int offset);
#endif /* SOLICITUDES_H_ */
