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

    if (resp==DISCONNECTED_SERVER){
    	pcb->exit_code=EC_ERROR_CONEXION;
    	log_error(logger, "Error al escribir en memoria (Page [%p] | Offset [%d] | Valor [%d]) ERROR_CONEXION", page, offset,valor_variable);
    }else if (resp==PAGE_FAULT || resp==SEGMENTATION_FAULT){
    	pcb->exit_code=EC_STACKOVERFLOW;
    	log_error(logger, "Error al escribir en memoria (Page [%p] | Offset [%d] | Valor [%d]) STACKOVERFLOW", page, offset,valor_variable);
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

void finalizar(void){
	log_trace(logger, "Finalizar");
	t_element_stack* contexto = stack_pop(pcb->indice_stack);
	if(list_size(pcb->indice_stack)==0){
		pcb->exit_code = OC_TERMINA_PROGRAMA;
	}else{
		pcb->PC = contexto->retPos;
	}
	eliminarContexto(contexto);
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	 log_trace(logger, "Leer valor variable compartida [%s]", variable);
	 connection_send(server_socket_kernel, OC_FUNCION_LEER_VARIABLE, variable);

	 t_valor_variable * buffer = malloc(sizeof(t_valor_variable));
	 uint8_t operation_code;
	 connection_recv(server_socket_kernel, &operation_code, &buffer);

	 return *buffer;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	log_trace(logger, "Asignar el valor [%d] a la variable compartida [%s]", valor, variable);
	int size_nombre =  string_length(variable);
	void * buffer = malloc(sizeof(int)+size_nombre*sizeof(t_nombre_variable)+sizeof(t_valor_variable));
	memcpy(buffer,&size_nombre, sizeof(int));
	memcpy(buffer+sizeof(int),variable, size_nombre*sizeof(t_nombre_variable));
	memcpy(buffer+sizeof(int)+size_nombre*sizeof(t_nombre_variable),&valor,sizeof(t_valor_variable));

	connection_send(server_socket_kernel, OC_FUNCION_ESCRIBIR_VARIABLE, buffer);

	return valor;
}


t_puntero alocar(t_valor_variable espacio){
    log_trace(logger, "Reserva [%d] espacio en Heap", espacio);

//    void * buff = malloc(sizeof(int) + sizeof(t_valor_variable));
    t_pedido_reservar_memoria* reservar = malloc(sizeof(t_pedido_reservar_memoria));
    reservar->espacio_pedido = espacio;
    reservar->pid = pcb->pid;
    connection_send(server_socket_kernel, OC_FUNCION_RESERVAR, reservar);

    free(reservar);

    t_puntero * buffer = malloc(sizeof(t_puntero));
    uint8_t operation_code;
    connection_recv(server_socket_kernel, &operation_code, &buffer);

    return *buffer;

}

void liberar(t_puntero puntero){
    log_trace(logger, "Libera el espacio alocado en [%p] ", puntero);

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

    uint16_t p_pid = pcb->pid;
    char * dir = malloc(strlen(direccion));
    memcpy(dir, direccion, strlen(direccion));
    int length_direccion = strlen(dir);
    void * buffer = malloc(sizeof(int) + sizeof(uint16_t) + strlen(dir) +sizeof(t_banderas));

    memcpy(buffer, &length_direccion, sizeof(int));
    memcpy(buffer + sizeof(int), &p_pid, sizeof(uint16_t));
    memcpy(buffer + sizeof(int) + sizeof(uint16_t), dir, length_direccion);
    memcpy(buffer + sizeof(int) + sizeof(uint16_t) + length_direccion, &banderas, sizeof(t_banderas));
    connection_send(server_socket_kernel, OC_FUNCION_ABRIR, buffer);

    free(buffer);
    int fd_proceso;
    uint8_t operation_code;
    connection_recv(server_socket_kernel, &operation_code, &fd_proceso);

    return fd_proceso;

}

void borrar(t_descriptor_archivo descriptor){
    log_trace(logger, "Borrar [%d]", descriptor);

    connection_send(server_socket_kernel, OC_FUNCION_BORRAR, &descriptor);
}

void cerrar(t_descriptor_archivo descriptor){
    log_trace(logger, "Cerrar [%d]", descriptor);

    t_archivo * archivo;
    archivo->pid = pcb->pid;
    archivo->descriptor_archivo = descriptor;

    connection_send(server_socket_kernel, OC_FUNCION_CERRAR, archivo);
}

void moverCursor(t_descriptor_archivo descriptor, t_valor_variable posicion){
    log_trace(logger, "Mover descriptor [%d] a [%d]", descriptor, posicion);

    void * buffer = malloc(sizeof(t_descriptor_archivo)+sizeof(t_valor_variable)+sizeof(int));
    memcpy(buffer, &(pcb->pid), sizeof(int));
    memcpy(buffer + sizeof(int), &descriptor, sizeof(t_descriptor_archivo));
    memcpy(buffer + sizeof(int) + sizeof(t_descriptor_archivo), &posicion, sizeof(t_valor_variable));
    connection_send(server_socket_kernel, OC_FUNCION_MOVER_CURSOR, buffer);

    free(buffer);
}

void escribir(t_descriptor_archivo desc, void * informacion, t_valor_variable tamanio){
    log_trace(logger, "Escribir [%.*s]:%d a [%d]", tamanio, informacion, tamanio, desc);

    log_trace(logger, "VALOR DE INFORMACION: %s", (char *)informacion);

    size_t size_desc = sizeof(t_descriptor_archivo);
    size_t size_tam = sizeof(t_valor_variable);
    size_t size_pid = sizeof(int);
    size_t size_inf = tamanio;
    size_t size_siz = sizeof(size_t);
    int pid = pcb->pid;

    size_t size_tot = size_desc + size_tam + size_pid + size_inf + size_siz;
    void * buffer = malloc(size_tot);
    memcpy(buffer, &size_tot, size_siz);
    memcpy(buffer + size_siz, &desc, size_desc);
    memcpy(buffer + size_siz + size_desc, &pid, size_pid);
    memcpy(buffer + size_siz + size_desc + size_pid, &tamanio, size_tam);
    memcpy(buffer + size_siz + size_desc + size_pid + size_tam, informacion, size_inf);

    connection_send(server_socket_kernel, OC_FUNCION_ESCRIBIR, buffer);

    uint8_t * operation_code = malloc(sizeof(uint8_t));
    uint8_t *resp;
    connection_recv(server_socket_kernel, operation_code, &resp);
    if(*resp<0){
    	pcb->exit_code=*resp;
    }
}

void leer(t_descriptor_archivo descriptor, t_puntero informacion, t_valor_variable tamanio){
    log_trace(logger, "Leer desde [%d] a [%p] con tamaño [%d]", descriptor, informacion, tamanio);

    t_pedido_archivo_leer * arch = malloc(sizeof(t_archivo));

    arch->descriptor_archivo = descriptor;
    arch->informacion = informacion;
    arch->tamanio = tamanio;
    arch->pid = pcb->pid;

    connection_send(server_socket_kernel, OC_FUNCION_LEER, arch);

    void * buffer = malloc(tamanio);
    uint8_t * operation_code = malloc(sizeof(uint8_t));
    connection_recv(server_socket_kernel, operation_code, &buffer);

    //TODO: ver como retornar la informacion devuelta por kernel
}
void signalParser(t_nombre_semaforo identificador_semaforo) {
	log_trace(logger, "Signal del semaforo %s", identificador_semaforo);

	connection_send(server_socket_kernel, OC_FUNCION_SIGNAL, identificador_semaforo);
	// agregar recv para que quede bloquedado OC_RESP_WAIT
	uint8_t * buffer = malloc(sizeof(int));
    uint8_t * operation_code = malloc(sizeof(uint8_t));
    connection_recv(server_socket_kernel, operation_code, &buffer);
    free(buffer);
    free(operation_code);

}

void waitParser(t_nombre_semaforo identificador_semaforo) {
	log_trace(logger, "Wait del semaforo %s", identificador_semaforo);
	connection_send(server_socket_kernel, OC_FUNCION_WAIT, &identificador_semaforo);
	// agregar recv para que quede bloquedado OC_RESP_WAIT
	uint8_t * buffer = malloc(sizeof(int));
    uint8_t * operation_code = malloc(sizeof(uint8_t));
    connection_recv(server_socket_kernel, operation_code, &buffer);
    free(buffer);
    free(operation_code);
}
