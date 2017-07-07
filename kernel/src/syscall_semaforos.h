/*
 * syscall_semaforos.h
 *
 *  Created on: 6/7/2017
 *      Author: utnso
 */

#ifndef SYSCALL_SEMAFOROS_H_
#define SYSCALL_SEMAFOROS_H_

#include <stdio.h>

typedef struct {
	t_nombre_semaforo semaforo;
	uint8_t valor;
} t_esther_semaforo;

t_list* tabla_semaforos;

t_esther_semaforo * traerSemaforo(t_nombre_semaforo nombre_semaforo);
t_esther_semaforo * crearSemaforo(t_nombre_semaforo nombre_semaforo);
void semaforoSignal(t_esther_semaforo * semaforo);
void semaforoWait(t_esther_semaforo * semaforo);

#endif /* SYSCALL_SEMAFOROS_H_ */
