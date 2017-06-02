/*
 * funcionesParser.h
 *
 *  Created on: 30/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESPARSER_H_
#define FUNCIONESPARSER_H_

#include <parser/parser.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <shared-library/socket.h>
#include <shared-library/generales.h>


typedef union {
	t_nombre_variable nombre_variable;
	t_puntero puntero;
	t_valor_variable valor_variable;
	t_nombre_etiqueta nombre_etiqueta;
	char* string;
	t_descriptor_archivo descriptor_archivo;
	t_direccion_archivo direccion_archivo;
	t_banderas banderas;
} Parametro;

typedef struct {
	char* nombre;
	Parametro* parametros;
} Llamada;

t_puntero definirVariable(t_nombre_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable);
t_valor_variable dereferenciar(t_puntero);
void asignar(t_puntero, t_valor_variable);
void irAlLabel(t_nombre_etiqueta nombre_etiqueta);
t_puntero alocar(t_valor_variable);
void liberar(t_puntero);
void llamarSinRetorno(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
t_descriptor_archivo abrir(t_direccion_archivo, t_banderas);
void borrar(t_descriptor_archivo);
void cerrar(t_descriptor_archivo);
void moverCursor(t_descriptor_archivo, t_valor_variable);
void escribir(t_descriptor_archivo, void *, t_valor_variable);
void leer(t_descriptor_archivo, t_puntero, t_valor_variable);
void wait(t_nombre_semaforo identificador_semaforo);
void signal(t_nombre_semaforo identificador_semaforo);

int server_socket_kernel, server_socket_memoria;
t_PCB* pcb;




posicion_memoria pedirStackMemoria();

#endif /* FUNCIONESPARSER_H_ */
