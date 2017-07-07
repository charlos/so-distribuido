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
#include <commons/collections/dictionary.h>


#define PUERTO_DE_ESCUCHA 53000
#define PLANIFICACION_FIFO 0
#define PLANIFICACION_ROUND_ROBIN 1

#define CPU 5

sem_t* semPlanificarLargoPlazo;
sem_t* semPlanificarCortoPlazo;
sem_t* semCantidadProgramsPlanificados;
sem_t* semColaListos;
sem_t* semCantidadProgramasPlanificados;
sem_t* semColaBloqueados;
sem_t* semColaFinalizados;
int grado_multiprogramacion, cantidad_programas_planificados;

int algoritmo_planificacion;
int quantum_planificacion;


typedef struct{
	t_cpu* cpu;
	t_PCB* pcb;
} t_cpu_pcb;

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
void planificador_largo_plazo();
void planificador_corto_plazo();
void enviar_a_ejecutar(t_cpu* cpu);
t_cpu* cpu_obtener_libre(t_list* lista_cpu);
bool continuar_procesando(t_cpu* cpu);

#endif /* KERNEL_H_ */
