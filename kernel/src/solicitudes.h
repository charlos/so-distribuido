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



t_list* tabla_paginas_heap;
t_list* tabla_global_archivos;
void mandar_codigo_a_memoria(char* codigo, int pid);
t_pagina_heap* obtener_pagina_con_suficiente_espacio(int pid, int espacio);
t_indice_codigo* obtener_indice_codigo(t_metadata_program* metadata);
t_dictionary* obtener_indice_etiquetas(t_metadata_program* metadata);
t_puntero buscar_bloque_disponible(void* pagina, int espacio_pedido);
t_heapMetadata* crear_metadata_libre(uint32_t espacio);
void cambiar_metadata(t_heapMetadata* metadata, int espacio_pedido);
void juntar_bloques(t_heapMetadata* metadata1, t_heapMetadata* metadata2);
int abrir_archivo(int, char*, t_banderas);
void agregar_bloque_libre(char* pagina, int offset);
int buscarArchivoTablaGlobal(char*);
int crearArchivoTablaGlobal(char*);
int cargarArchivoTablaProceso(int pid, int fd_global, t_banderas flags);
int nuevoFD_PID(int);
t_list* crearTablaArchProceso();
t_table_file* getTablaArchivo(int pid);
t_heapMetadata* leer_metadata(void* pagina);
bool pagina_vacia(int pid, int nro_pagina);
void tabla_heap_sacar_pagina(t_pedido_liberar_memoria* pedido_free);
void liberar_pagina(t_pedido_liberar_memoria* pedido_free);
t_codigo_proceso* buscar_codigo_de_proceso(int pid);
void * obtener_informacion_a_imprimir(t_puntero puntero, int pid);

#endif /* SOLICITUDES_H_ */
