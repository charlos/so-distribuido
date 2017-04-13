/*
 * generales.c
 *
 *  Created on: 12/4/2017
 *      Author: utnso
 */
#include <commons/log.h>

// Crea el logger con el nombre del proceso
void crear_logger(char* nombreProceso, t_log** logger, bool active_console, t_log_level level){
	char* nombre_log = string_new();
	string_append(&nombre_log, nombreProceso);
	string_append(&nombre_log, ".log");
	*logger = log_create(nombre_log, "<NOMBRE-DE-PROCESO>", active_console, level); //TODO Hacer la funcion para sacar el nombre del proceso de la ruta
}
