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
	int start;
	int offset;
}t_indice_codigo;

t_indice_codigo* obtener_indice_codigo(char* codigo);
#endif /* SOLICITUDES_H_ */
