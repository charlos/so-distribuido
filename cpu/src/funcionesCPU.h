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
#include <signal.h>
#include <parser/parser.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <shared-library/socket.h>
#include <shared-library/generales.h>
#include <shared-library/memory_prot.h>
#include "cpu.h"

t_element_stack* nuevoContexto();
void eliminarContexto(t_element_stack*);
void args_vars_destroy(t_args_vars*);
int agregarAStack(t_args_vars*, int );
t_element_stack* stack_pop(t_stack*);
void stack_push(t_stack*, t_element_stack* );
t_args_vars* arg_var_pop(t_list*);
void arg_var_push(t_list*, t_args_vars*);
void inicializarFuncionesParser(void);
void procesarMsg(char *);
void load_properties(void);
int calcularPaginaProxInstruccion();
void getNextPosStack();
void updatePageOffsetAvailable(u_int32_t);
u_int32_t getPageofPos(t_puntero);
u_int32_t getOffsetofPos(t_puntero);
void handlerDesconexion(int);

t_indice_codigo* obtener_indice_codigo(t_metadata_program*);
t_dictionary* obtener_indice_etiquetas(t_metadata_program*);
#endif /* FUNCIONESCPU_H_ */
