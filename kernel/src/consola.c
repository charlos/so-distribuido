/*
 * consola.c
 *
 *  Created on: 4/7/2017
 *      Author: utnso
 */

#include "kernel_generales.h"
#include "solicitudes.h"

int leer_comando(char* command);

void iniciar_consola(){
	while(true){
		char* command = malloc(sizeof(char)*256);

		printf("/*******************************************************\\ \n");
		printf("| multiprogramming		: cambiar multiprogramacion 	  |\n");
		printf("| process_list (state)  : procesos en sistema	 		  |\n");
		printf("| process pid 			: obtener informacion de proceso  |\n");
		printf("| global_file_table     : tabla global de archivos	      |\n");
		printf("| kill pid         		: finalizar proceso   			  |\n");
		printf("| stop         			: detener planificacion		      |\n");
		printf("\\********************************************************/\n");

		fgets(command, 256, stdin);

		int ret = leer_comando(command);
	}
}

int leer_comando(char* command) {

	int caracter = 0;
	while (command[caracter] != '\n') caracter++;
	command[caracter] = '\0';

	char** palabras = string_n_split(command, 2, " ");

	int i=0;
		while(palabras[i]) {
		i++;
	}

	if(strcmp(palabras[0], "process_list") == 0) {
		if(i > 1){
			// TODO Listar procesos de una cola
		}
	}
	else if(strcmp(palabras[0], "process")==0) {
//	TODO mostrar informacion de proceso
	}
	else if(strcmp(palabras[0], "global_file_table") ==0 ) {
		printf("Tabla global de Archivos:\n");
		imprimir_tabla_global_de_archivos();
	}
	else if(strcmp(palabras[0], "kill")==0) {
	//
		t_PCB* pcbEncontrado = NULL;
		int pid = atoi(palabras[1]);
		t_PCB* pcbASacar = malloc(sizeof(t_PCB));
		pcbASacar->pid = pid;
		sem_wait(semColaNuevos);
		// lo busco en la cola new
		pcbEncontrado = sacar_pcb(cola_nuevos, pcbASacar);
		sem_post(semColaNuevos);
		if(pcbEncontrado == NULL){
			sem_wait(semColaListos);
			// lo busco en la cola ready
			pcbEncontrado = sacar_pcb(cola_listos, pcbASacar);
			sem_post(semColaListos);
			if(pcbEncontrado == NULL){
				// lo busco en la cola blocked
				sem_wait(semColaBloqueados);
				pcbEncontrado = sacar_pcb(cola_bloqueados, pcbASacar);
				sem_post(semColaBloqueados);
				if(pcbEncontrado == NULL){
					// si se llegó hasta acá es porque el pid o no existe o se está ejecutando
					t_cpu* cpu = buscar_pcb_en_lista_cpu(pcbASacar);
					if(cpu == NULL){
						printf("No existe programa con el PID (%d)\n", &pid);
						return -1;
					} else {
						// si existe cpu se le setea "matar_proceso" para que al momento de terminar la instriccion la cpu lo mande a la cola exit
						cpu->matar_proceso = 1;
					}
				}
			}
		}
		// se settea mensaje de error cuando se mata un proceso desde consola de kernel
		pcbEncontrado->exit_code = -77;
		// se agrega a la cola de finalizados
		queue_push(cola_finalizados, pcbEncontrado);
	}
	else if(strcmp(palabras[0], "multiprogramming")==0) {
		kernel_conf->grado_multiprog = atoi(palabras[1]);
		sem_post(semPlanificarLargoPlazo);
	}
	else if(strcmp(palabras[0], "play")==0) {
		pthread_mutex_unlock(&mutex_planificar_corto_plazo);
		pthread_mutex_unlock(&mutex_planificar_largo_plazo);
	}
	else if(strcmp(palabras[0], "stop")==0) {
		pthread_mutex_lock(&mutex_planificar_corto_plazo);
		pthread_mutex_lock(&mutex_planificar_largo_plazo);
	}

	else return -2;
}

void listar_procesos_de_cola(t_queue* cola_de_estado){
	void _imprimir_proceso(t_PCB* pcb){
		printf("Proceso id: %d\n", pcb->pid);
	}
	list_iterate(cola_de_estado->elements, (void*) _imprimir_proceso);
}

void imprimir_tabla_global_de_archivos(){
	void _imprimir_entrada(t_global_file* f){
		printf("%s	|	%d", f->file, f->open);
	}
	list_iterate(tabla_global_archivos, (void*) _imprimir_entrada);
}
