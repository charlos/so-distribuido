/*
 * cpu.h
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include "funcionesParser.h"
#include <shared-library/generales.h>
#include <shared-library/memory_prot.h>
#include <commons/collections/list.h>

typedef struct{
	char* memory_ip;
	char* memory_port;
	char* kernel_ip;
	char* kernel_port;
}t_cpu_conf;

void load_properties(void);
void inicializarFuncionesParser(void);
void procesarMsg(char* ) ;
void stack_push(t_stack* , t_element_stack* );
t_link_element* stack_pop(t_stack* );
int calcularPagina(t_indice_codigo*);
uint8_t handshake_kernel(int);
#endif /* CPU_H_ */
