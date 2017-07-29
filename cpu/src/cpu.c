/*
 ============================================================================
 Name        : cpu.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <shared-library/socket.h>
#include <parser/metadata_program.h>
#include "cpu.h"
#include "funcionesCPU.h"

AnSISOP_funciones * funciones;
AnSISOP_kernel * func_kernel;
t_cpu_conf* cpu_conf;
t_log* logger;
int pagesize;
int flagDesconeccion = 0;
t_page_offset* nextPageOffsetInStack;
t_PCB* pcb;

int server_socket_kernel, server_socket_memoria;

void procesarMsg(char * msg);

int stack_size;
int main(void) {

	int *continuar = malloc(sizeof(int));
	*continuar  = -1;
	uint16_t quantum_sleep = 0;

	signal(SIGUSR1, &handlerDesconexion);

	crear_logger("./cpu", &logger, true, LOG_LEVEL_TRACE);
	log_trace(logger, "Log Creado!!");

	load_properties();

	int op_code;
	void * buffer;
	server_socket_kernel = connect_to_socket(cpu_conf->kernel_ip, cpu_conf->kernel_port);
	connection_recv(server_socket_kernel, &op_code, &buffer);
	stack_size = *(int*)buffer;
	server_socket_memoria = connect_to_socket(cpu_conf->memory_ip, cpu_conf->memory_port);
	inicializarFuncionesParser();

	pagesize = handshake(server_socket_memoria,'c',0, logger);

	log_trace(logger, "Socket kernel: %i \n", server_socket_kernel);

	if (pagesize>0){
		log_trace(logger, "Handshake con Memoria. El tamaño de la página es %d",pagesize);
	} else {
		log_error(logger, "Problema con Handshake con Memoria.");
	}

	int pc, page, offset, pageend, size_to_read;
	uint8_t operation_code;
	char* instruccion;
	char* pcb_serializado;

	while(1){
		pageend=0;
		page=0;
		offset=0;
		size_to_read=0;

		if(*continuar==-1){
			log_trace(logger, "A la espera de recibir PCB");
			connection_recv(server_socket_kernel, &operation_code, &pcb_serializado);
			if(operation_code!=OC_PCB){
				log_error(logger, "Problema al intentar recibir PCB");
				exit(0);
			}
			pcb = deserializer_pcb(pcb_serializado);
			free(pcb_serializado);
		}else{
			quantum_sleep = *continuar;
		}

		usleep(1000 * (quantum_sleep));
		*continuar = -1;

		if(list_size(pcb->indice_stack)==0){
			//Si el indice del stack está vacio es porque estamos en la primera línea de código, creo la primera línea del scope
			nuevoContexto();
			pcb->SP--; //Al crear un nuevo contexto se incrementó el StackPointer, pero en este caso, cuando no había contexto alguno, corresponde que SP
					   //quede en 0 al ser el primer contexto de ejecución.
		}

		nextPageOffsetInStack = malloc(sizeof(t_page_offset));
		getNextPosStack();  // Actualizo la variable nextPageOffsetInStack guardando page/offset de la proxima ubicación a utilizar en el stack

		t_indice_codigo* icodigo = malloc(sizeof(t_indice_codigo));
		t_read_response * read_response;
		t_read_response * read_response2;
		memcpy(icodigo, ((t_indice_codigo*) pcb->indice_codigo)+pcb->PC, sizeof(t_indice_codigo));

		instruccion = malloc(icodigo->size);

		page = icodigo->offset/pagesize;
		//calcularPaginaProxInstruccion();
		offset = icodigo->offset-(page*pagesize);
		pageend = (icodigo->offset+icodigo->size)/pagesize;

		//para el caso en que la instrucción enté partida en dos páginas
		if(page<pageend){
			size_to_read = (page+1)*pagesize-icodigo->offset;
		}else{
			size_to_read = icodigo->size;
		}

		//pido leer la instruccion a la memoria
		read_response = memory_read(server_socket_memoria, pcb->pid, page,offset , size_to_read, logger);
		if (read_response->exec_code!=1){
			log_error(logger, "Error al leer código de memoria (Page [%d] |Offset [%d] |Size [%d])", page, icodigo->offset, size_to_read);
		}

		memcpy(instruccion,read_response->buffer,read_response->buffer_size);

		if (page<pageend){
			read_response2 = memory_read(server_socket_memoria, pcb->pid, pageend,0 , icodigo->size-size_to_read, logger);
			if (read_response2->exec_code!=1){
				log_error(logger, "Error al leer de memoria (Page [%d] |Offset [%d] |Size [%d])", pageend, 0,icodigo->size-size_to_read);
			}
			memcpy(instruccion+read_response->buffer_size,read_response2->buffer,read_response2->buffer_size);
		}

		instruccion[(icodigo->size) - 1] = '\0';
		string_trim(&instruccion);

		log_trace(logger, "Evaluando instruccion: %s",instruccion);

		if(instruccion[0]=='\t'){
			procesarMsg(instruccion+1);
		}else{
			procesarMsg(instruccion);
		}

		free(instruccion);
		free(read_response->buffer);
		free(read_response);
		if(page<pageend){
			free(read_response2->buffer);
			free(read_response2);
		}
		free(icodigo);
		free(nextPageOffsetInStack);

		pcb->PC++;

		if(pcb->exit_code == OC_TERMINA_PROGRAMA){
			serializar_y_enviar_PCB(pcb, server_socket_kernel, OC_TERMINA_PROGRAMA);
			pcb_destroy(pcb);
		} else if(flagDesconeccion){
			serializar_y_enviar_PCB(pcb, server_socket_kernel, OC_DESCONEX_CPU);
			pcb_destroy(pcb);
			exit(0);
		} else if(pcb->exit_code<0){
			serializar_y_enviar_PCB(pcb, server_socket_kernel, OC_ERROR_EJECUCION_CPU);
			pcb_destroy(pcb);
		}else {
			serializar_y_enviar_PCB(pcb, server_socket_kernel, OC_TERMINO_INSTRUCCION);
			log_trace(logger, "------------PID %d Antes de recv de OC_TERMINO_INSTRUCCION",pcb->pid);
			connection_recv(server_socket_kernel, &operation_code, &continuar);
			log_trace(logger, "------------PID %d Despues de recv de OC_TERMINO_INSTRUCCION: Respuesta %d",pcb->pid,*continuar);
			if(operation_code==OC_RESP_TERMINO_INSTRUCCION && *continuar==-1){
				pcb_destroy(pcb);
			}
		}

	}
	free(continuar);
	return EXIT_SUCCESS;

}
