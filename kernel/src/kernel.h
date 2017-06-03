/*
 * kernel.h
 *
 * Created on: 9/4/2017
 *    Authors: Carlos Flores, Gustavo Tofaletti, Dante Romero
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <shared-library/generales.h>
#include <shared-library/socket.h>
#include <semaphore.h>
#include "kernel_generales.h"


#define PUERTO_DE_ESCUCHA 53000

#define CPU 5



/**
 * @NAME:  manage_select
 * @DESC:  Permite monitoriar sets de file descriptors. Acepta conexiones nuevas y responde a pedidos de los fd que esta escuchando
 *
 * @PARAMS int port: puerto de escucha
 */
void manage_select(t_aux* estructura);



/**
 * @NAME: load_kernel_properties
 * @DESC: Carga los atributos de configuracion leidos
 */

void solicitar_progama_nuevo(int file_descriptor, char* codigo);
uint8_t handshake_memory(int socket);
void handshake_filsesystem(int socket);

#endif /* KERNEL_H_ */
