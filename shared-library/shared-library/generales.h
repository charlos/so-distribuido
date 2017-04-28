/*
 * generales.h
 *
 *  Created on: 18/4/2017
 *      Author: utnso
 */

#ifndef GENERALES_H_
#define GENERALES_H_

#include <commons/log.h>
#include <commons/string.h>

char* obtener_nombre_proceso(char*);

	/**
	 * @NAME: crear_logger
	 * @DESC: crea una instancia de logger
	 *
	 * @PARAMS char* path: path del proceso ejecutado. Tomara su nombre para el archivo .log. Siempre se va a usar argv[0]
	 * 			t_log** logger: puntero a direccion de logger
	 * 			bool console: determina si los logs se mostraran por pantalla
	 * 			t_log_level: tipo de seguimiento que se quiere en el log
	 */
void crear_logger(char*, t_log**, bool, t_log_level);

#endif /* GENERALES_H_ */
