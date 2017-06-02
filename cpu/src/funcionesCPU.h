/*
 * funcionesCPU.h
 *
 *  Created on: 1/6/2017
 *      Author: utnso
 */

#ifndef FUNCIONESCPU_H_
#define FUNCIONESCPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <shared-library/socket.h>
#include <shared-library/generales.h>
#include <shared-library/memory_prot.h>
#include "cpu.h"

int nuevoContexto();
int agregarAStack(t_args_vars*, int );
t_link_element* stack_pop(t_stack*);
void stack_push(t_stack*, t_element_stack* );
t_args_vars* arg_var_pop(t_list*);
void arg_var_push(t_list*, t_args_vars*);
void inicializarFuncionesParser(void);
void procesarMsg(char *);
void load_properties(void);
int calcularPagina();
void loadlastPosStack();
void updatePageAvailable(u_int32_t);

#endif /* FUNCIONESCPU_H_ */
