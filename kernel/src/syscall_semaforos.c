/*
 * syscall_semaforos.c
 *
 *  Created on: 6/7/2017
 *      Author: utnso
 */
#include "syscall_semaforos.h"

t_esther_semaforo * traerSemaforo(t_nombre_semaforo semaforo) {

	int _mismoNombre(t_esther_semaforo * elemento) {
		return elemento->semaforo == semaforo;
	}
	t_esther_semaforo * resultado = list_find(tabla_semaforos, (void *) _mismoNombre);

	if(resultado == NULL) {
		resultado = crearSemaforo(semaforo);
	}
	return resultado;
}
t_esther_semaforo * crearSemaforo(t_nombre_semaforo nombre_semaforo) {

	t_esther_semaforo * semaforo = malloc(sizeof(t_esther_semaforo));
	semaforo->semaforo = nombre_semaforo;
	semaforo->valor = 0;

	return semaforo;
}

void semaforoSignal(t_esther_semaforo * semaforo) {
	semaforo->valor++;
}
void semaforoWait(t_esther_semaforo * semaforo) {
	if(semaforo->valor) semaforo->valor--;
}
