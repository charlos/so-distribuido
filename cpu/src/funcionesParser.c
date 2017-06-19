/*
 * funcionesParser.c
 *
 *  Created on: 30/4/2017
 *      Author: utnso
 */

#include "funcionesParser.h"
#include <shared-library/generales.h>
#include <shared-library/memory_prot.h>
#include "cpu.h"
#include "funcionesCPU.h"

t_queue* llamadas = NULL;
t_queue* retornos = NULL;
t_log* logger = NULL;

extern int pagesize;
extern t_page_offset* nextPageOffsetInStack;
extern int server_socket_kernel, server_socket_memoria;
extern t_PCB* pcb;

void finalizar(void){
	log_trace(logger, "Finalizar (en construcción)");
	t_link_element* regIndicestack = stack_pop(pcb->indice_stack);
	//todo terminar esto, para quitar del todo el elemento del stack y volver a cambiar PC
}


t_puntero definirVariable(t_nombre_variable identificador_variable) {
	log_trace(logger, "Definir Variable [%c]", identificador_variable);
	if (identificador_variable >= '0' && identificador_variable <= '9') {
		t_args_vars * newarg = malloc(sizeof(t_args_vars));
		newarg->id = identificador_variable;

		updatePageOffsetAvailable(sizeof(t_puntero)); //actualizo la pagina y offset que usaré revisando si hay espacio en la pagina actual para la nueva variable
												//si no lo hay incremento el nro de pagina para pasar a usar la siguiente
												//no hay que validar ahora, cuando mandemos a escribir en memoria ésta nos dará error

		newarg->pagina = nextPageOffsetInStack->page;
		newarg->offset = nextPageOffsetInStack->offset;
		newarg->size = sizeof(t_puntero);
		agregarAStack(newarg,ARG_STACK);


	} else {
		t_args_vars * newvar = malloc(sizeof(t_args_vars));
		newvar->id = identificador_variable;

		updatePageOffsetAvailable(sizeof(t_puntero)); //actualizo la pagina y offset que usaré revisando si hay espacio en pa pagina actual para la nueva variable
												//si no lo hay incremento el nro de pagina para pasar a usar la siguiente
												//no hay que validar ahora, cuando mandemos a escribir en memoria ésta nos dará error

		newvar->pagina = nextPageOffsetInStack->page;
		newvar->offset = nextPageOffsetInStack->offset;
		newvar->size = sizeof(t_puntero);
		agregarAStack(newvar,VAR_STACK);

	}
	return nextPageOffsetInStack->page*pagesize+nextPageOffsetInStack->offset; //valor de la posicion de la variable en memoria respecto del comienzo del stack
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	nuevoContexto();
    int* posicion = (int*)dictionary_get(pcb->indice_etiquetas, etiqueta);
    pcb->PC = *posicion;
    log_trace(logger, "Llamar sin Retorno. Ir a la linea [%d]", pcb->PC);
    pcb->PC--; //decremento porque al terminar de ejecutar se incrementará PC
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	log_trace(logger, "Llamar con Retorno a [%d] y id [%s]", donde_retornar,etiqueta);
	t_element_stack* regIndicestack = nuevoContexto();

	posicion_memoria* retVar = malloc(sizeof(posicion_memoria));
	retVar->offset = getOffsetofPos(donde_retornar);
	retVar->pagina = getPageofPos(donde_retornar);
	retVar->size = sizeof(posicion_memoria);

	regIndicestack->retVar = retVar;
	regIndicestack->retPos = pcb->PC;

    int* posicion = (int*)dictionary_get(pcb->indice_etiquetas, etiqueta);
    pcb->PC = *posicion;
    pcb->PC--; //decremento porque al terminar de ejecutar se incrementará PC
    log_trace(logger, "Ir a la linea [%d]", pcb->PC);
}

t_puntero obtenerPosicionVariable(t_nombre_variable nombre_variable){
   log_trace(logger, "Obtener posicion variable [%c]", nombre_variable);


   bool _findbyname(t_args_vars * reg){
	   return reg->id==nombre_variable;
   }
   t_args_vars * reg;
   t_element_stack* regContextStack = list_get(pcb->indice_stack,pcb->SP);

   if (nombre_variable >= '0' && nombre_variable <= '9') {
	   reg = list_find(regContextStack->args, (void*) _findbyname);
	}else{
	   reg = list_find(regContextStack->vars, (void*) _findbyname);
	}
   log_trace(logger, "encontrado en Stack (id pagina offset size): [%c] [%d] [%d] [%d]", reg->id,reg->pagina,reg->offset,reg->size);
   log_trace(logger, "Retorno el t_puntero [%d]", ((reg->pagina)*(pagesize))+reg->offset);
   return ((reg->pagina)*(pagesize))+reg->offset;

}

t_valor_variable dereferenciar(t_puntero direccion_variable){
    log_trace(logger, "Dereferenciar [%p]", direccion_variable);

    int page = getPageofPos(direccion_variable);
    int offset =  getOffsetofPos(direccion_variable);
    //Voy a la direccion de la variable. Comunico con Memoria
    t_read_response * respuesta_memoria = memory_read(server_socket_memoria, pcb->pid, page, offset, sizeof(t_puntero), logger);
    //Recibo contenido
    t_valor_variable * buffer = malloc(respuesta_memoria->buffer_size);

	memcpy(buffer, respuesta_memoria->buffer, respuesta_memoria->buffer_size);
	t_valor_variable resp = *buffer;

	//free(buffer);

	return resp;
}

void asignar(t_puntero puntero, t_valor_variable valor_variable){
    log_trace(logger, "Asignar a [%p] el valor [%d]", puntero, valor_variable);
    int resp, page, offset, size;

    page = getPageofPos(puntero);
    offset =  getOffsetofPos(puntero);
    size = sizeof(t_valor_variable);

    resp = memory_write(server_socket_memoria,  pcb->pid, page, offset, size, size, &valor_variable, logger);

    if (resp!=1){
    	log_error(logger, "Error al escribir en memoria (Page [%p] | Offset [%d] | Valor [%d])", page, offset,valor_variable);
    }
}

void irAlLabel(t_nombre_etiqueta nombre_etiqueta) {

    int* posicion = (int*)dictionary_get(pcb->indice_etiquetas, nombre_etiqueta);
    pcb->PC = *posicion;
    log_trace(logger, "Ir a la linea [%d]", pcb->PC);
    pcb->PC--; //decremento porque al terminar de ejecutar se incrementará PC
}

void retornar(t_valor_variable retorno){
	 /*
	  * Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está ejecutando,
	  * recuperando el Cursor de Contexto Actual, el Program Counter y la direccion donde retornar,
	  * asignando el valor de retorno en esta, previamente apilados en el Stack.
	 */
	int resp;
	t_element_stack* contexto = stack_pop(pcb->indice_stack);

	posicion_memoria* retVar =contexto->retVar;

    resp = memory_write(server_socket_memoria,  pcb->pid, retVar->pagina, retVar->offset, retVar->size, retVar->size, &retorno, logger);

    if (resp!=1){
    	log_error(logger, "PID:%d Error al escribir en memoria (Page [%p] | Offset [%d] | Valor [%d])", pcb->pid, retVar->pagina, retVar->offset,retorno);
    }

    pcb->PC = contexto->retPos;

    eliminarContexto(contexto);

}


t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){

}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){

}


t_puntero alocar(t_valor_variable espacio){
    log_trace(logger, "Reserva [%d] espacio en Heap", espacio);

//    void * buff = malloc(sizeof(int) + sizeof(t_valor_variable));
    t_pedido_reservar_memoria* reservar = malloc(sizeof(t_pedido_reservar_memoria));
    reservar->espacio_pedido = espacio;
    reservar->pid = pcb->pid;
    connection_send(server_socket_kernel, OC_FUNCION_RESERVAR, reservar);

    t_puntero * buffer = malloc(sizeof(t_direccion_archivo));
    connection_recv(server_socket_kernel, OC_RESP_RESERVAR, &buffer);

    return *buffer;

}

void liberar(t_puntero puntero){
    log_trace(logger, "Reserva [%p] espacio", puntero);

    t_pedido_liberar_memoria* liberar = malloc(sizeof(t_pedido_liberar_memoria));
    liberar->pid = pcb->pid;
    liberar->posicion = puntero;
    connection_send(server_socket_kernel, OC_FUNCION_LIBERAR, liberar);

}

char* boolToChar(bool boolean) {
    return boolean ? "✔" : "✖";
}

t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas banderas){
    log_trace(logger, "Abrir [%s] Lectura: %s. Escritura: %s, Creacion: %s", direccion,
              boolToChar(banderas.lectura), boolToChar(banderas.escritura), boolToChar(banderas.creacion));
//TODO revisar arreglo del calculo del tamaño de la direccion para tener en cuenta todos los caracteres de char* (t_nombre_variable)

    int length_direccion = string_length(direccion);
    void * buffer = malloc(sizeof(int) + sizeof(int) + length_direccion * sizeof(t_nombre_variable)+sizeof(t_banderas));
    memcpy(buffer, &(pcb->pid), sizeof(int));
    memcpy(buffer + sizeof(int), &length_direccion,sizeof(int));
    memcpy(buffer + sizeof(int) + sizeof(int), &direccion, length_direccion * sizeof(t_nombre_variable));
    memcpy(buffer + sizeof(int) + sizeof(int) + length_direccion * sizeof(t_nombre_variable), &banderas, sizeof(t_banderas));
    connection_send(server_socket_kernel, OC_FUNCION_ABRIR, buffer);

    free(buffer);

}

void borrar(t_descriptor_archivo descriptor){
    log_trace(logger, "Borrar [%d]", descriptor);

    connection_send(server_socket_kernel, OC_FUNCION_BORRAR, descriptor);
}

void cerrar(t_descriptor_archivo descriptor){
    log_trace(logger, "Cerrar [%d]", descriptor);

    connect_send(server_socket_kernel, OC_FUNCION_CERRAR, descriptor);
}

void moverCursor(t_descriptor_archivo descriptor, t_valor_variable posicion){
    log_trace(logger, "Mover descriptor [%d] a [%d]", descriptor, posicion);

    void * buffer = malloc(sizeof(t_descriptor_archivo)+sizeof(t_valor_variable));
    memcpy(buffer, &descriptor, sizeof(t_descriptor_archivo));
    memcpy(buffer + sizeof(t_descriptor_archivo), &posicion, sizeof(t_valor_variable));
    connect_send(server_socket_kernel, OC_FUNCION_MOVER_CURSOR, buffer);

    free(buffer);
}

void escribir(t_descriptor_archivo desc, void * informacion, t_valor_variable tamanio){
    log_trace(logger, "Escribir [%.*s]:%d a [%d]", tamanio, informacion, tamanio, desc);

    t_archivo * arch = malloc(sizeof(t_archivo));

    arch->descriptor_archivo = desc;
    arch->informacion = informacion;
    arch->tamanio = tamanio;

    connection_send(server_socket_kernel, OC_FUNCION_ESCRIBIR, arch);

    void * buffer = malloc(tamanio);
    connection_recv(server_socket_kernel, OC_RESP_ESCRIBIR, buffer);
}

void leer(t_descriptor_archivo descriptor, t_puntero informacion, t_valor_variable tamanio){
    log_trace(logger, "Leer desde [%d] a [%p] con tamaño [%d]", descriptor, informacion, tamanio);

    t_archivo * arch = malloc(sizeof(t_archivo));

    arch->descriptor_archivo = descriptor;
    arch->informacion = informacion;
    arch->tamanio = tamanio;

    connection_send(server_socket_kernel, OC_FUNCION_LEER, arch);

    void * buffer = malloc(tamanio);
    connection_recv(server_socket_kernel, OC_RESP_LEER, buffer);

    //TODO: ver como retornar la informacion devuelta por kernel
}
void signal(t_nombre_semaforo identificador_semaforo) {
	log_trace(logger, "Signal del semaforo %s", identificador_semaforo);

	connection_send(server_socket_kernel, OC_FUNCION_SIGNAL, identificador_semaforo);

}

void wait(t_nombre_semaforo identificador_semaforo) {
	log_trace(logger, "Wait del semaforo %s", identificador_semaforo);

	connection_send(server_socket_kernel, OC_FUNCION_WAIT, identificador_semaforo);
}
