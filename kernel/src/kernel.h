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
#include <shared-library/file_system_prot.h>
#include <semaphore.h>
#include "kernel_generales.h"
#include <commons/collections/dictionary.h>


#define PUERTO_DE_ESCUCHA 53000
#define PLANIFICACION_FIFO "FIFO"
#define PLANIFICACION_ROUND_ROBIN "RR"

#define CPU 5



int grado_multiprogramacion, cantidad_programas_planificados;


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
void planificador_largo_plazo();
void planificador_corto_plazo();
void enviar_a_ejecutar(t_cpu* cpu);
//t_cpu* cpu_obtener_libre(t_list* lista_cpu);
bool continuar_procesando(t_cpu* cpu);


/**
* @NAME: cola_listos_push
* @DESC: Agrega un elemento al final de la cola de listos
*/
void cola_listos_push(t_PCB *element);

void pasarDeNewAReady();
void pasarDeReadyAExecute();
void pasarDeExecuteAReady(t_cpu* cpu);
void pasarDeExecuteAExit(t_cpu* cpu);
void pasarDeExecuteABlocked(t_cpu* cpu);
void pasarDeBlockedAReady(uint16_t pidPcbASacar);

#endif /* KERNEL_H_ */
