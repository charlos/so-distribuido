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

typedef t_list t_stack;

typedef struct{
	pid_t pid;
	int PC;
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
#endif /* KERNEL_H_ */
