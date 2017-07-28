/*
 * kernel_generales.c
 *
 *  Created on: 17/5/2017
 *      Author: utnso
 */

#include "kernel_generales.h"


void load_kernel_properties(char * ruta) {
	t_config * conf;
	if(ruta == NULL)
		conf = config_create("./kernel.cfg");
	else conf = config_create(ruta);

	kernel_conf = malloc(sizeof(t_kernel_conf));
	kernel_conf->program_port = config_get_int_value(conf, "PUERTO_PROG");
	kernel_conf->cpu_port = config_get_int_value(conf, "PUERTO_CPU");
	kernel_conf->memory_ip = config_get_string_value(conf, "IP_MEMORIA");
	kernel_conf->memory_port = config_get_string_value(conf, "PUERTO_MEMORIA");
	kernel_conf->filesystem_ip = config_get_string_value(conf, "IP_FS");
	kernel_conf->filesystem_port = config_get_string_value(conf, "PUERTO_FS");
	kernel_conf->grado_multiprog = config_get_int_value(conf, "GRADO_MULTIPROG");
	kernel_conf->algoritmo = config_get_string_value(conf, "ALGORITMO");
	kernel_conf->quantum = config_get_int_value(conf, "QUANTUM");
	kernel_conf->quantum_sleep = config_get_int_value(conf, "QUANTUM_SLEEP");
	kernel_conf->stack_size = config_get_int_value(conf, "STACK_SIZE");
	kernel_conf->sem_ids = config_get_string_value(conf, "SEM_IDS");
	kernel_conf->sem_init = config_get_array_value(conf, "SEM_INIT");
	kernel_conf->shared_vars = config_get_string_value(conf, "SHARED_VARS");

	crearVariablesCompartidas();
	crearSemaforos();
}

t_PCB* crear_PCB(){
	t_PCB* PCB = malloc(sizeof(t_PCB));
	pthread_mutex_lock(&registro_pid_mutex);
	PCB->pid = registro_pid++;
	pthread_mutex_unlock(&registro_pid_mutex);
	PCB->cantidad_paginas = 0;
	PCB->exit_code = 0;
	PCB->SP = 0;
	PCB->indice_stack = list_create();
	return PCB;
}

void crearVariablesCompartidas(){
	int i=0;

    int length_value = strlen(kernel_conf->shared_vars) - 2;
    char* value_without_brackets = string_substring(kernel_conf->shared_vars, 1, length_value);

	char** variables = string_split(value_without_brackets, ",");
	while(variables[i]!=NULL){
		t_shared_var* variable = malloc(sizeof(t_shared_var));
		string_trim(&(variables[i]));
		char* nombre = string_substring(variables[i], 1, strlen(variables[i]) - 1);
		variable->nombre=nombre;
		variable->valor=0;
		list_add(tabla_variables_compartidas,variable);
		i++;
	}
}
/*
t_cpu* obtener_cpu(int socket){
	return find_by_fd(socket);
}*/

/*t_cpu* find_by_fd(int fd) {
	int _is_fd(t_cpu *cpu) {
		return (cpu->file_descriptor == fd);
	}
	sem_wait(semListaCpu);
	t_cpu* cpu = list_find(lista_cpu, (void*) _is_fd);
	sem_post(semListaCpu);
	return cpu;
}*/

void crearSemaforos(){
	int i=0;
	semaforos = dictionary_create();
    int length_value = strlen(kernel_conf->sem_ids) - 2;
    char* value_without_brackets = string_substring(kernel_conf->sem_ids, 1, length_value);

	char** semaforos_cfg = string_split(value_without_brackets, ",");
	while(semaforos_cfg[i]!=NULL){
		string_trim(&(semaforos_cfg[i]));
		t_semaphore* semaforo = malloc(sizeof(t_semaphore));
		semaforo->cola = queue_create();
		semaforo->cuenta = atoi(kernel_conf->sem_init[i]);
		dictionary_put(semaforos, semaforos_cfg[i], semaforo);
		i++;
	}
}

/**
 * si el pid se encuentra en la cola de bloqueados, devuelve TRUE
 * pero ademas tira la magia de actualizarlo (si... es cualquiera, pero me ahorra tener que recorrer la cola dos veces)
 */
bool proceso_bloqueado(t_PCB* pcb){
	t_PCB* aux = NULL;
	bool response = 0;
	sem_wait(semColaBloqueados);
	bool _is_pcb(t_PCB* p) {
		return p->pid == pcb->pid;
	}
	// esto es un asco, pero bueno... elimino el viejo pcb de la cola de bloqueados y pongo el nuevo
	if(queue_size(cola_bloqueados)){
		aux = list_find(cola_bloqueados->elements, (void*) _is_pcb);
	}
	if(aux != NULL){
		//queue_push(cola_bloqueados, pcb);
		response = 1;
	}
	sem_post(semColaBloqueados);

	return response;
}

t_PCB* sacar_pcb(t_queue* cola, t_PCB* pcb){
	bool _is_pcb(t_PCB* p) {
		return (p->pid == pcb->pid);
	}
	t_PCB* pcbEncontrado = list_remove_by_condition(cola->elements, (void*) _is_pcb);
	return pcbEncontrado;
}

t_PCB* sacar_pcb_con_pid(t_queue* cola, uint16_t pid){
	bool _is_pcb(t_PCB* p) {
		return (p->pid == pid);
	}
	t_PCB* pcbEncontrado = list_remove_by_condition(cola->elements, (void*) _is_pcb);
	return pcbEncontrado;
}

t_cpu* obtener_cpu(int file_descriptor){
	bool _mismo_file_descriptor(t_cpu* cpu){
		return cpu->file_descriptor == file_descriptor;
	}
	sem_wait(semListaCpu);
	t_cpu* cpu = list_find(lista_cpu, (void*)_mismo_file_descriptor);
	sem_post(semListaCpu);
	return cpu;
}

void liberar_nuevo_proceso(t_nuevo_proceso* nuevo_proceso){
	free(nuevo_proceso->codigo);
	free(nuevo_proceso);
}

t_cpu* buscar_pcb_en_lista_cpu(t_PCB* pcbABuscar){
	bool _is_cpu_con_pcb_a_buscar(t_cpu* cpu){
		if(cpu->proceso_asignado != NULL){
			return cpu->proceso_asignado->pid == pcbABuscar->pid;
		} else return 0;
	}
	sem_wait(semListaCpu);
	t_cpu* cpu = list_find(lista_cpu, (void*) _is_cpu_con_pcb_a_buscar);
	sem_post(semListaCpu);
	return cpu;
}

t_cpu* obtener_cpu_por_proceso(int pid){
	sem_wait(semListaCpu);
	bool _mismo_file_descriptor(t_cpu* cpu){
		if(cpu->proceso_asignado)return cpu->proceso_asignado->pid == pid;
		return false;
	}
	t_cpu* cpu = list_find(lista_cpu, (void*)_mismo_file_descriptor);
	sem_post(semListaCpu);
	return cpu;
}

void rw_lock_unlock(int action) {
	switch (action) {
	case LOCK_READ :
	//	pthread_rwlock_rdlock(lock_tabla_global_archivos);
		break;
	case LOCK_WRITE :
	//	pthread_rwlock_wrlock(lock_tabla_global_archivos);
		break;
	case UNLOCK :
	//	pthread_rwlock_unlock(lock_tabla_global_archivos);
		break;
	}
	//return EXIT_SUCCESS;
}

void liberar_cpu(t_cpu* cpu){
	sem_wait(semListaCpu);
	cpu->quantum = 0;
	cpu->proceso_asignado = NULL;
	cpu->matar_proceso = 0;
	cpu->proceso_desbloqueado_por_signal = 0;
	sem_post(semListaCpu);
}

t_table_file* getTablaArchivo(int pid){

	   bool _findbyPID(t_table_file* reg){
		   return reg->pid==pid;
	   }
	   t_table_file* tabla;
	   tabla = list_find(listaDeTablasDeArchivosDeProcesos, (void*) _findbyPID);

	   if(tabla == NULL) {
		   tabla = malloc(sizeof(t_table_file));

		   tabla->pid = pid;
		   tabla->tabla_archivos = list_create();
		   tabla->contador_fd = 10;

		   list_add(listaDeTablasDeArchivosDeProcesos, tabla);
	   }
	   return tabla;
}

void pasar_proceso_a_exit(int pid){
	t_PCB* pcbEncontrado = NULL;
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
			log_trace(logger, "PID %d - pasar_proceso_a_exit antes de sem_wait ", pcbASacar->pid);
			sem_wait(semColaBloqueados);
			pcbEncontrado = sacar_pcb(cola_bloqueados, pcbASacar);
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
			log_trace(logger, "PID %d - pasar_proceso_a_exit despues del sem_post ", pcbASacar->pid);
			if(pcbEncontrado == NULL){
				// si se llegó hasta acá es porque el pid o no existe o se está ejecutando
				t_cpu* cpu = buscar_pcb_en_lista_cpu(pcbASacar);
				if(cpu == NULL){
					printf("No existe programa con el PID (%d)\n", &pid);
					return;
				} else {
					// si existe cpu se le setea "matar_proceso" para que al momento de terminar la instriccion la cpu lo mande a la cola exit
					cpu->matar_proceso = 1;
					return;
				}
			}
		}
	}
	// se settea mensaje de error cuando se mata un proceso desde consola de kernel
	pcbEncontrado->exit_code = -77;
	// se agrega a la cola de finalizados
	t_par_socket_pid* parEncontrado = encontrar_consola_de_pcb(pcbEncontrado->pid);
	int status = 1;
	connection_send(parEncontrado->socket, MUERE_PROGRAMA, &status);

	pthread_mutex_lock(&mutex_pedido_memoria);
	memory_finalize_process(memory_socket, pid, logger);
	pthread_mutex_unlock(&mutex_pedido_memoria);
	// se agrega a la cola de finalizados
	queue_push(cola_finalizados, pcbEncontrado);
	sem_wait(semCantidadProgramasPlanificados);
	// TODO: Hacer sem_post del planificador largo plazo (?)
}
