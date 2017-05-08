/*
 * semaforo.c
 *
 *  Created on: 8/5/2017
 *      Author: utnso
 */

#include <semaphore.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

sem_t semaforo_cpu;
void rutina(int n){
	if(n == SIGINT){
		sem_post(&semaforo_cpu);
	}
}
int main(){
	signal(SIGINT, rutina);
	sem_init(&semaforo_cpu, 1, 0);
	while(1){
		sem_wait(&semaforo_cpu);
		printf("anda");
	}
}

