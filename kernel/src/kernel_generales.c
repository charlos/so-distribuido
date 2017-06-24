/*
 * kernel_generales.c
 *
 *  Created on: 17/5/2017
 *      Author: utnso
 */

#include "kernel_generales.h"


void load_kernel_properties(void) {
	t_config * conf = config_create("/home/utnso/workspace/tp-2017-1c-Stranger-Code/kernel/Debug/kernel.cfg");
	kernel_conf = malloc(sizeof(t_kernel_conf));
	kernel_conf->program_port = config_get_int_value(conf, "PUERTO_PROG");
	kernel_conf->cpu_port = config_get_int_value(conf, "PUERTO_CPU");
	kernel_conf->memory_ip = config_get_string_value(conf, "IP_MEMORIA");
	kernel_conf->memory_port = config_get_string_value(conf, "PUERTO_MEMORIA");
	kernel_conf->filesystem_ip = config_get_string_value(conf, "IP_FS");
	kernel_conf->filesystem_port = config_get_string_value(conf, "PUERTO_FS");
	kernel_conf->grado_multiprog = config_get_int_value(conf, "GRADO_MULTIPROG");
	kernel_conf->algoritmo = config_get_string_value(conf, "ALGORITMO");
	kernel_conf->quantum = config_get_int_value(conf, "QUANTUM");
	kernel_conf->quantum_sleep = config_get_int_value(conf, "QUANTUM_SLEEP");
	kernel_conf->stack_size = config_get_int_value(conf, "STACK_SIZE");
	kernel_conf->sem_ids = config_get_array_value(conf, "SEM_IDS");
	kernel_conf->sem_init = config_get_array_value(conf, "SEM_INIT");
	kernel_conf->shared_vars = config_get_string_value(conf, "SHARED_VARS");

	crearVariablesCompartidas();
}

t_PCB* crear_PCB(){
	t_PCB* PCB = malloc(sizeof(t_PCB));
	PCB->pid = registro_pid++;
	PCB->cantidad_paginas = 0;
	return PCB;
}

void crearVariablesCompartidas(){
	int i=0;

    int length_value = strlen(kernel_conf->shared_vars) - 2;
    char* value_without_brackets = string_substring(kernel_conf->shared_vars, 1, length_value);

	char** variables = string_split(value_without_brackets, ",");
	while(variables[i]!=NULL){
		t_shared_var* variable = malloc(sizeof(t_shared_var));
		string_trim(&(variables[i]));
		char* nombre = string_substring(variables[i], 1, strlen(variables[i]) - 1);
		variable->nombre=nombre;
		variable->valor=0;
		list_add(tabla_variables_compartidas,variable);
		i++;
	}
}
