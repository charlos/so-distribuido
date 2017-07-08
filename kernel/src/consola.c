/*
 * consola.c
 *
 *  Created on: 4/7/2017
 *      Author: utnso
 */

#include "kernel_generales.h"
#include "solicitudes.h"

int iniciar_consola(){
	char* command = malloc(sizeof(char)*256);

	printf("/*******************************************************\\ \n");
	printf("| multiprogramming		: cambiar multiprogramacion 	  |\n");
	printf("| process_list (state)  : procesos en sistema	 		  |\n");
	printf("| process pid 			: obtener informacion de proceso  |\n");
	printf("| global_file_table     : tabla global de archivos	      |\n");
	printf("| kill pid         		: finalizar proceso   			  |\n");
	printf("| stop         			: detener planificacion		      |\n");
	printf("\\********************************************************/\n");

	fgets(command, 256, stdin);

	int ret = read_command(command);
}

int read_command(char* command) {

	int caracter = 0;
	while (command[caracter] != '\n') caracter++;
	command[caracter] = '\0';

	char** palabras = string_n_split(command, 2, " ");

	int i=0;
		while(palabras[i]) {
		i++;
	}

	if(strcmp(palabras[0], "process_list") == 0) {
		if(i > 1){
			// TODO Listar procesos de una cola
		}
	}
	else if(strcmp(palabras[0], "process")==0) {
//	TODO mostrar informacion de proceso
	}
	else if(strcmp(palabras[0], "global_file_table") ==0 ) {
		printf("Tabla global de Archivos:\n");
		imprimir_tabla_global_de_archivos();
	}
	else if(strcmp(palabras[0], "kill")==0) {
//
	}
	else if(strcmp(palabras[0], "multiprogramming")==0) {
//		kernel_conf->grado_multiprog = atoi(palabras[1]);
	}
	else if(strcmp(palabras[0], "stop")==0) {

	}
	else return -2;
}

void listar_procesos_de_cola(t_queue* cola_de_estado){
	void _imprimir_proceso(t_PCB* pcb){
		printf("Proceso id: %d\n", pcb->pid);
	}
	list_iterate(cola_de_estado, (void*) _imprimir_proceso);
}

void imprimir_tabla_global_de_archivos(){
	void _imprimir_entrada(t_global_file* f){
		printf("%s	|	%d", f->file, f->open);
	}
	list_iterate(tabla_global_archivos, (void*) _imprimir_entrada);
}
