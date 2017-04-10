/*
 ============================================================================
 Name        : memoria.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <shared-library/connect.h>

int main(void) {
	char* timeStart = temporal_get_string_time(); //usando commons
	printf("Tiempo de Inicio del Proceso: %s\n", timeStart);
	saludo();
	connect_send("mi primer mensaje enviado :)"); //usando nuestra shared library
	return EXIT_SUCCESS;
}

int saludo() {
	puts("¡Hola MEMORIA!");
	return EXIT_SUCCESS;
}
