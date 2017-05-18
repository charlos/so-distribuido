/*
 * funcionesParser.c
 *
 *  Created on: 30/4/2017
 *      Author: utnso
 */

#include "funcionesParser.h"

t_queue* llamadas = NULL;
t_queue* retornos = NULL;
t_log* logger = NULL;

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

t_puntero definirVariable(t_nombre_variable identificador_variable) {
	log_trace(logger, "Definir Variable [%c]", identificador_variable);
	    queue_push(llamadas, crearLlamada("definirVariable", 1, identificador_variable));
	    CON_RETORNO_PUNTERO;
}

t_puntero obtenerPosicionVariable(t_nombre_variable nombre_variable){
    log_trace(logger, "Obtener posicion variable [%c]", nombre_variable);

    queue_push(llamadas, crearLlamada("obtenerPosicionVariable", 1, nombre_variable));
    CON_RETORNO_PUNTERO;
}

t_valor_variable dereferenciar(t_puntero puntero){
    log_trace(logger, "Dereferenciar [%p]", puntero);

    queue_push(llamadas, crearLlamada("dereferenciar", 1, puntero));
    CON_RETORNO_VALOR;
}

void asignar(t_puntero puntero, t_valor_variable valor_variable){
    log_trace(logger, "Asignar a [%p] el valor [%d]", puntero, valor_variable);
    queue_push(llamadas, crearLlamada("asignar", 2, puntero, valor_variable));
}

void irAlLabel(t_nombre_etiqueta nombre_etiqueta) {
    log_trace(logger, "Ir al Label [%s]", nombre_etiqueta);
    queue_push(llamadas, crearLlamada("irAlLabel", 1, string_duplicate(nombre_etiqueta)));
}

t_puntero alocar(t_valor_variable espacio){
    log_trace(logger, "Reserva [%d] espacio", espacio);

    queue_push(llamadas, crearLlamada("alocar", 1, espacio));
    CON_RETORNO_PUNTERO;
}

void liberar(t_puntero puntero){
    log_trace(logger, "Reserva [%p] espacio", puntero);

    queue_push(llamadas, crearLlamada("liberar", 1, puntero));
}

char* boolToChar(bool boolean) {
    return boolean ? "✔" : "✖";
}
t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas banderas){
    log_trace(logger, "Abrir [%s] Lectura: %s. Escritura: %s, Creacion: %s", direccion,
              boolToChar(banderas.lectura), boolToChar(banderas.escritura), boolToChar(banderas.creacion));

    queue_push(llamadas, crearLlamada("abrir", 2, direccion, banderas));

    void * buffer = malloc(sizeof(t_direccion_archivo)+sizeof(t_banderas));
    memcpy(buffer, &direccion, sizeof(t_direccion_archivo));
    memcpy(buffer + sizeof(t_direccion_archivo), &banderas, sizeof(t_banderas));
    connection_send(server_socket_kernel, OC_FUNCION_ABRIR, buffer);

    free(buffer);
    CON_RETORNO_DESCRIPTOR;
}
void borrar(t_descriptor_archivo descriptor){
    log_trace(logger, "Borrar [%d]", descriptor);
    queue_push(llamadas, crearLlamada("borrar", 1, descriptor));
}

void cerrar(t_descriptor_archivo descriptor){
    log_trace(logger, "Cerrar [%d]", descriptor);
    queue_push(llamadas, crearLlamada("cerrar", 1, descriptor));
}

void moverCursor(t_descriptor_archivo descriptor, t_valor_variable posicion){
    log_trace(logger, "Mover descriptor [%d] a [%d]", descriptor, posicion);
    queue_push(llamadas, crearLlamada("mover", 2, descriptor, posicion));

}

void escribir(t_descriptor_archivo desc, void * informacion, t_valor_variable tamanio){
    log_trace(logger, "Escribir [%.*s]:%d a [%d]", tamanio, informacion, tamanio, desc);

    queue_push(llamadas, crearLlamada("escribir", 3, desc, string_duplicate(informacion), tamanio));
}

void leer(t_descriptor_archivo descriptor, t_puntero informacion, t_valor_variable tamanio){
    log_trace(logger, "Leer desde [%d] a [%p] con tamaño [%d]", descriptor, informacion, tamanio);

    queue_push(llamadas, crearLlamada("leer", 3, descriptor, informacion, tamanio));
}
