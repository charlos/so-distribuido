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

void mandar_codigo_a_memoria(char* codigo, int pid);
t_indice_codigo* obtener_indice_codigo(t_metadata_program* metadata);
t_dictionary* obtener_indice_etiquetas(t_metadata_program* metadata);
#endif /* SOLICITUDES_H_ */
