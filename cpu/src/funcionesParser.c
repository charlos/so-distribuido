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
extern t_page_offset* lastPageOffset;
extern int stackPointer;
extern int server_socket_kernel, server_socket_memoria;
extern t_PCB* pcb;
/*

Llamada* crearLlamada(char* nombre, int cantidadParametros, ...){
    va_list parametrosVa;
    Parametro* parametros = malloc(10*sizeof(Parametro));

    va_start (parametrosVa , cantidadParametros);
    int x;
    for (x = 0; x < cantidadParametros; x++ ){
        parametros[x] = va_arg (parametrosVa, Parametro);
    }
    va_end ( parametrosVa );

    Llamada* retorno = malloc(sizeof(Llamada));
    retorno->nombre = nombre;
    retorno->parametros = parametros;
    return retorno;
}

Parametro* crearRetorno(){
    static int num = 4;
    Parametro* ret = malloc(sizeof(Parametro));
    ret->valor_variable = num++;
    return ret;
}

#define CON_RETORNO_PUNTERO     CON_RETORNO("p", puntero)
#define CON_RETORNO_VALOR     CON_RETORNO("d", valor_variable)
#define CON_RETORNO_DESCRIPTOR CON_RETORNO_VALOR

#define CON_RETORNO(FORMATO, TIPO) \
Parametro *retorno = crearRetorno(); \
queue_push(retornos, retorno);\
log_trace(logger, "\tdevuelve [%" FORMATO "]", retorno->TIPO);\
return retorno->puntero
*/
void finalizar(void){
	t_link_element* regIndicestack = stack_pop(pcb->indice_stack);
	//todo terminar esto, para quitar del todo el elemento del stack y volver a cambiar PC
}


t_puntero definirVariable(t_nombre_variable identificador_variable) {
	log_trace(logger, "Definir Variable [%c]", identificador_variable);
	    //queue_push(llamadas, crearLlamada("definirVariable", 1, identificador_variable));
	    //CON_RETORNO_PUNTERO;
	if (identificador_variable >= '0' && identificador_variable <= '9') {
		t_args_vars * newarg = malloc(sizeof(t_args_vars));
		newarg->id = identificador_variable;

		updatePageAvailable(sizeof(t_puntero)); //actualizo la pagina que usaré revisando si hay espacio en la pagina actual para la nueva variable
												//si no lo hay incremento el nro de pagina para pasar a usar la siguiente
												//no hay que validar ahora, cuando mandemos a escribir en memoria ésta nos dará error

		newarg->pagina = lastPageOffset->page;
		newarg->offset = lastPageOffset->offset;
		newarg->size = sizeof(t_puntero);
		agregarAStack(newarg,ARG_STACK);

	} else {
		t_args_vars * newvar = malloc(sizeof(t_args_vars));
		newvar->id = identificador_variable;

		updatePageAvailable(sizeof(t_puntero)); //actualizo la pagina que usaré revisando si hay espacio en pa pagina actual para la nueva variable
												//si no lo hay incremento el nro de pagina para pasar a usar la siguiente
												//no hay que validar ahora, cuando mandemos a escribir en memoria ésta nos dará error

		newvar->pagina = lastPageOffset->page;
		newvar->offset = lastPageOffset->offset;
		newvar->size = sizeof(t_puntero);
		agregarAStack(newvar,VAR_STACK);

	}
	return lastPageOffset->page*pagesize+lastPageOffset->offset; //valor de la posicion de la variable en memoria respecto del comienzo del stack
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	nuevoContexto();
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	log_trace(logger, "Llamar con Retorno [%d]", donde_retornar);
	t_element_stack* regIndicestack = nuevoContexto();

	posicion_memoria* retVar = malloc(sizeof(posicion_memoria));
	retVar->offset = getOffsetofPos(donde_retornar);
	retVar->pagina = getPageofPos(donde_retornar);
	retVar->size = sizeof(posicion_memoria);

	regIndicestack->retVar = retVar;
	regIndicestack->retPos = pcb->PC;
}

t_puntero obtenerPosicionVariable(t_nombre_variable nombre_variable){
   log_trace(logger, "Obtener posicion variable [%c]", nombre_variable);


   int _findbyname(t_args_vars * reg){
	   return reg->id==nombre_variable;
   }
   t_args_vars * reg;
   t_element_stack* regContextStack = list_get(pcb->indice_stack,stackPointer);

   if (nombre_variable >= '0' && nombre_variable <= '9') {
	   reg = list_find(regContextStack->args, (void*) _findbyname);
	}else{
		reg = list_find(regContextStack->vars, (void*) _findbyname);
	}

   return reg->pagina*pagesize+reg->offset;

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
    return (t_valor_variable) buffer;

    free(buffer);
}

void asignar(t_puntero puntero, t_valor_variable valor_variable){
    log_trace(logger, "Asignar a [%p] el valor [%d]", puntero, valor_variable);
    //queue_push(llamadas, crearLlamada("asignar", 2, puntero, valor_variable));
}

void irAlLabel(t_nombre_etiqueta nombre_etiqueta) {
    log_trace(logger, "Ir al Label [%s]", nombre_etiqueta);
    int posicion = dictionary_get(pcb->indice_etiquetas, nombre_etiqueta);

    int i = list_size(pcb->indice_stack) - 1;
    t_element_stack * element = list_get(pcb->indice_stack, i);
    element->retPos = pcb->PC;

    pcb->PC = posicion;
}

t_puntero alocar(t_valor_variable espacio){
    log_trace(logger, "Reserva [%d] espacio", espacio);

    void * buff = malloc(sizeof(int) + sizeof(t_valor_variable));
    memcpy(buff, &(pcb->pid), sizeof(int));
    memcpy(buff + sizeof(int), &espacio, sizeof(t_valor_variable));
    connection_send(server_socket_kernel, OC_FUNCION_RESERVAR, buff);

    t_puntero * buffer = malloc(sizeof(t_direccion_archivo));
    connection_recv(server_socket_kernel, OC_RESP_RESERVAR, &buffer);

    return *buffer;
    //CON_RETORNO_PUNTERO;
}

void liberar(t_puntero puntero){
    log_trace(logger, "Reserva [%p] espacio", puntero);

    connection_send(server_socket_kernel, OC_FUNCION_LIBERAR, puntero);

}

char* boolToChar(bool boolean) {
    return boolean ? "✔" : "✖";
}

t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas banderas){
    log_trace(logger, "Abrir [%s] Lectura: %s. Escritura: %s, Creacion: %s", direccion,
              boolToChar(banderas.lectura), boolToChar(banderas.escritura), boolToChar(banderas.creacion));

    void * buffer = malloc(sizeof(t_direccion_archivo)+sizeof(t_banderas));
    memcpy(buffer, &direccion, sizeof(t_direccion_archivo));
    memcpy(buffer + sizeof(t_direccion_archivo), &banderas, sizeof(t_banderas));
    connection_send(server_socket_kernel, OC_FUNCION_ABRIR, buffer);

    free(buffer);
    //CON_RETORNO_DESCRIPTOR;
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
