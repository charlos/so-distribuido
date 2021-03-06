/*
 * solicitudes.h
 *
 *  Created on: 16/5/2017
 *      Author: utnso
 */

#ifndef SOLICITUDES_H_
#define SOLICITUDES_H_

#include <shared-library/file_system_prot.h>
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
t_list* tabla_variables_compartidas;
uint8_t contador_fd_global;
void solve_request(t_info_socket_solicitud* info_solicitud);
void mandar_codigo_a_memoria(char* codigo, int pid);
t_pagina_heap* obtener_pagina_con_suficiente_espacio(int pid, int espacio);
t_indice_codigo* obtener_indice_codigo(t_metadata_program* metadata);
t_dictionary* obtener_indice_etiquetas(t_metadata_program* metadata);
t_puntero buscar_bloque_disponible(void* pagina, int espacio_pedido);
t_heapMetadata* crear_metadata_libre(uint32_t espacio);
void cambiar_metadata(t_heapMetadata* metadata, int espacio_pedido);
void juntar_bloques(t_heapMetadata* metadata1, t_heapMetadata* metadata2);
int abrir_archivo(uint16_t, char*, t_banderas);
void agregar_bloque_libre(char* pagina, int offset);
int buscarArchivoTablaGlobal(char*);
t_process_file* buscarArchivoTablaProceso(t_table_file* tabla, int);
int crearArchivoTablaGlobal(char*);
int cargarArchivoTablaProceso(int pid, int fd_global, t_banderas flags);
int nuevoFD_PID(int);
t_list* crearTablaArchProceso();

t_heapMetadata* leer_metadata(void* pagina);
bool pagina_vacia(int pid, int nro_pagina);
void tabla_heap_sacar_pagina(t_pedido_liberar_memoria* pedido_free);
void liberar_pagina(t_pedido_liberar_memoria* pedido_free, int offset_de_paginas);
t_codigo_proceso* buscar_codigo_de_proceso(int pid);
void asignarValorVariable(t_shared_var*);
t_valor_variable leerValorVariable(char*);
t_read_response * obtener_informacion_a_imprimir(t_puntero puntero, int pid);
void obtener_direccion_relativa(t_puntero* puntero, int nro_pagina_heap, int cantidad_paginas_codigo);
void obtener_direccion_logica(t_pedido_liberar_memoria* pedido_free, int cantidad_paginas_codigo);
int calcular_paginas_necesarias(char* codigo);
char* getPathFrom_PID_FD(int pid, int fdProceso);
t_global_file * getFileFromGlobal(int global_fd);
char* getPath_Global(int fdGlobal);
int notificar_memoria_inicio_programa(int pid, int cant_paginas, char* codigo_completo);
void descontarDeLaTablaGlobal(int global_fd);
t_par_socket_pid* encontrar_consola_de_pcb(int pid);
#endif /* SOLICITUDES_H_ */
