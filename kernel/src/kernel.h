/*
 * kernel.h
 *
 * Created on: 9/4/2017
 *    Authors: Carlos Flores, Gustavo Tofaletti, Dante Romero
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/collections/list.h>
#include <shared-library/generales.h>
#include <shared-library/socket.h>

#define PUERTO_DE_ESCUCHA 53000

typedef t_list t_stack;

typedef struct{
	pid_t pid;
	int PC;
	int cantidad_paginas;
	t_stack* indice_stack;
	t_list* indice_codigo;
	// tabla de archivos
	int exit_code;
}t_PCB;

typedef struct{
	int pagina;
	int offset;
	int size;
}posicion_memoria;

typedef struct{
	t_list* args;
	t_list* vars;
	int retPos;
	posicion_memoria* retVar;
}t_element_stack;

int registro_pid = 0;
t_log* logger;

t_PCB* crear_PCB();
void solicitar_progama_nuevo(int file_descriptor, char* codigo);


#endif /* KERNEL_H_ */
