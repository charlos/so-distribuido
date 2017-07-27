/*
 * consola.c
 *
 *  Created on: 4/7/2017
 *      Author: utnso
 */

#include "kernel_generales.h"
#include "solicitudes.h"
#include <pthread.h>

int leer_comando(char* command);
char* obtener_estado(int pid);
t_par_socket_pid* buscar_info_proceso(int pid);
void imprimir_tabla_global_de_archivos();
void listar_procesos_de_cola(t_queue* cola_de_estado);
void _imprimir_proceso(t_PCB* pcb);

void iniciar_consola(){
	while(true){
		char* command = malloc(sizeof(char)*256);

		printf("/*********************************************************\\ \n");
		printf("| multiprogramming      : cambiar multiprogramacion       |\n");
		printf("| process_list (state)  : procesos en sistema             |\n");
		printf("| process pid           : obtener informacion de proceso  |\n");
		printf("| global_file_table     : tabla global de archivos        |\n");
		printf("| kill pid              : finalizar proceso               |\n");
		printf("| stop                  : detener planificacion           |\n");
		printf("\\*********************************************************/\n");

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
			if(strcmp(palabras[1], "nuevos") == 0){
				listar_procesos_de_cola(cola_nuevos);
			}else if(strcmp(palabras[1], "listos") == 0){
				listar_procesos_de_cola(cola_listos);
			}else if(strcmp(palabras[1], "bloqueados") == 0){
				listar_procesos_de_cola(cola_bloqueados);
			}else if(strcmp(palabras[1], "ejecutando") == 0){
				listar_procesos_de_cola(cola_ejecutando);
			}else if(strcmp(palabras[1], "finalizados") == 0){
				listar_procesos_de_cola(cola_finalizados);
			}
		}else{
			listar_procesos_de_cola(cola_nuevos);
			listar_procesos_de_cola(cola_listos);
			listar_procesos_de_cola(cola_bloqueados);
			listar_procesos_de_cola(cola_ejecutando);
			listar_procesos_de_cola(cola_finalizados);
		}
	}
	else if(strcmp(palabras[0], "process")==0) {
		int pid = atoi(palabras[1]);
		t_par_socket_pid* info_proceso = buscar_info_proceso(pid);
		printf("Proceso pid: %d\n", pid);
		char* estado = obtener_estado(pid);
		printf("Estado: %s\n", estado);
		printf("Cantidad de Syscalls: %d\n", info_proceso->cantidad_syscalls);
		printf("Cantidad de memoria alocada: %d\n", info_proceso->memoria_reservada);
		printf("Cantidad de memoria liberada: %d\n", info_proceso->memoria_liberada);
		free(estado);
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
			pthread_mutex_lock(&semColaListos);
			// lo busco en la cola ready
			pcbEncontrado = sacar_pcb(cola_listos, pcbASacar);
			pthread_mutex_unlock(&semColaListos);
			if(pcbEncontrado == NULL){
				// lo busco en la cola blocked
				sem_wait(semColaBloqueados);
				pcbEncontrado = sacar_pcb(cola_bloqueados, pcbASacar);//TODO cuando se saca de bloqueados, tambien se debe sacar de la cola del semaforo
				if(pcbEncontrado!=NULL){
					sem_wait(semSemaforos);
					void _semaforo(char* key, t_semaphore* semaforo){
						bool _is_pid(uint16_t* pid){
							if(*pid == pcbEncontrado->pid){
								semaforo->cuenta++;
								return true;
							}else{
								return false;
							}
						}
						list_remove_and_destroy_by_condition(semaforo->cola->elements, _is_pid, free);
					}
					dictionary_iterator(semaforos, _semaforo);
					sem_post(semSemaforos);
				}
				sem_post(semColaBloqueados);
				if(pcbEncontrado == NULL){
					// si se llegó hasta acá es porque el pid o no existe o se está ejecutando
					t_cpu* cpu = buscar_pcb_en_lista_cpu(pcbASacar);
					if(cpu == NULL){
						printf("No existe programa con el PID (%d)\n", pid);
						return -1;
					} else {
						// si existe cpu se le setea "matar_proceso" para que al momento de terminar la instriccion la cpu lo mande a la cola exit
						cpu->matar_proceso = 1;
						return -1;
					}
				}
			}
		}
		// se settea mensaje de error cuando se mata un proceso desde consola de kernel
		pcbEncontrado->exit_code = -77;

		t_par_socket_pid* parEncontrado = encontrar_consola_de_pcb(pcbEncontrado->pid);
		int status = 1;
		connection_send(parEncontrado->socket, OC_MUERE_PROGRAMA, &status);
		memory_finalize_process(memory_socket, pcbEncontrado->pid, logger);

		// se agrega a la cola de finalizados
		queue_push(cola_finalizados, pcbEncontrado);
		sem_wait(semCantidadProgramasPlanificados);
		// TODO: Hacer sem_post del planificador largo plazo (?)
	}
	else if(strcmp(palabras[0], "multiprogramming")==0) {
		kernel_conf->grado_multiprog = atoi(palabras[1]);
		sem_post(semPlanificarLargoPlazo);
	}
	else if(strcmp(palabras[0], "play")==0) {
		planificar = true;
		int cortoPlazo;
		int largoPlazo;
		int ret;

		ret = sem_getvalue(&sem_planificar_corto_plazo, &cortoPlazo);
		ret = sem_getvalue(&sem_planificar_largo_plazo, &largoPlazo);
		if(cortoPlazo == 0) sem_post(&sem_planificar_corto_plazo);
		else sem_init(&sem_planificar_corto_plazo, 0, 0);
		if(largoPlazo == 0) sem_post(&sem_planificar_largo_plazo);
		else sem_init(&sem_planificar_largo_plazo, 0, 0);
	}
	//sssem_getvalue()
	else if(strcmp(palabras[0], "stop")==0) {
		planificar = false;
	}

	else return -2;
}

void _imprimir_proceso(t_PCB* pcb){
	if(pcb){
		char *estado;
		printf("Proceso id: %d -----", pcb->pid);
		estado = obtener_estado(pcb->pid);
		printf("Estado: %s -----", estado);
		if(strcmp(estado, "Finalizado") == 0) printf(" Exit Code: %d", pcb->exit_code);
		printf("\n\n");
	}
}

void listar_procesos_de_cola(t_queue* cola_de_estado){
	if(cola_de_estado == cola_nuevos){
		void _imprimir_nuevo_proceso(t_nuevo_proceso* p){
			_imprimir_proceso(p->pcb);
		}
		list_iterate(cola_nuevos->elements, (void*)_imprimir_nuevo_proceso);
	}else if(cola_de_estado == cola_ejecutando){
		void _imprimir_ejecutando(t_cpu* c){
			_imprimir_proceso(c->proceso_asignado);
		}
		list_iterate(lista_cpu, (void*)_imprimir_ejecutando);
	}else list_iterate(cola_de_estado->elements, (void*) _imprimir_proceso);
}

void imprimir_tabla_global_de_archivos(){
	void _imprimir_entrada(t_global_file* f){
		printf("%s	|	%d", f->file, f->open);
	}
	list_iterate(tabla_global_archivos, (void*) _imprimir_entrada);
}


char* obtener_estado(int pid){
	bool tiene_mismo_pid(t_PCB* pcb){
		return pcb->pid == pid;
	}
	bool tiene_proceso_asignado(t_cpu* cpu){
		if(cpu->proceso_asignado){
			return cpu->proceso_asignado->pid == pid;
		}else return 0;
	}
	bool tiene_proces_nuevo(t_nuevo_proceso* p){
		return p->pcb->pid == pid;
	}

	if(list_any_satisfy(cola_nuevos->elements, (void *)tiene_proces_nuevo)){
		return string_duplicate("Nuevo");
	}else if(list_any_satisfy(cola_listos->elements, (void *)tiene_mismo_pid)){
		return string_duplicate("Listo");
	}else if (list_any_satisfy(lista_cpu, (void *)tiene_proceso_asignado)){
		return string_duplicate("Ejecutando");
	}else if (list_any_satisfy(cola_bloqueados->elements, (void *)tiene_mismo_pid)){
		return string_duplicate("Bloqueado");
	}else if (list_any_satisfy(cola_finalizados->elements, (void *)tiene_mismo_pid)){
		return string_duplicate("Finalizado");
	}else return string_duplicate("No se encontro el Proceso");
}

t_par_socket_pid* buscar_info_proceso(int pid){
	bool tiene_mismo_pid(t_par_socket_pid* p){
		return p->pid == pid;
	}
	return list_find(tabla_sockets_procesos, (void*) tiene_mismo_pid);
}
