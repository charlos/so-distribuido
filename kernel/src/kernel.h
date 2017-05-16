/*
 * kernel.h
 *
 * Created on: 9/4/2017
 *    Authors: Carlos Flores, Gustavo Tofaletti, Dante Romero
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/collections/list.h>
#include <shared-library/generales.h>
#include <shared-library/socket.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <semaphore.h>

#define PUERTO_DE_ESCUCHA 53000

#define CPU 5


typedef struct{
	int program_port;
	int cpu_port;
	char* memory_ip;
	char* memory_port;
	char* filesystem_ip;
	char* filesystem_port;
	int quantum;
	int quantum_sleep;
	char* algoritmo;
	int grado_multiprog;
	char** sem_ids;
	t_list* sem_init;
	char** shared_vars;
	int stack_size;
}t_kernel_conf;

typedef struct{
	int file_descriptor;
	t_PCB* proceso_asignado;
	int quantum;
}t_cpu;

int registro_pid = 0;
t_log* logger;
t_queue* cola_listos;
t_queue* cola_bloqueados;
t_queue* cola_exit;
t_queue* cola_cpu;

/**
 * @NAME:  manage_select
 * @DESC:  Permite monitoriar sets de file descriptors. Acepta conexiones nuevas y responde a pedidos de los fd que esta escuchando
 *
 * @PARAMS char * port: puerto de escucha
 */
void manage_select(int port);

/**
 * @NAME: crear_PCB
 * @DESC: Crea instancia de pcb y le asigna pid unico.
 */
t_PCB* crear_PCB();

/**
 * @NAME: load_kernel_properties
 * @DESC: Carga los atributos de configuracion leidos
 */
void load_kernel_properties(void);
void solicitar_progama_nuevo(int file_descriptor, char* codigo);
uint8_t handshake_memory(int socket);
void handshake_filsesystem(int socket);

#endif /* KERNEL_H_ */
