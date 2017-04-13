/*
 * memoria.h
 *
 *  Created on: 12/4/2017
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdint.h>

void create_logger(void);
void load_memory_properties(void);
void print_memory_properties(void);
void process_request(int *);

typedef struct {
	uint32_t puerto;
	uint32_t marcos;
	uint32_t marco_size;
	uint32_t entradas_cache;
	uint32_t cache_x_proc;
	char *   reemplazo_cache;
	uint32_t retardo;
	char *   logfile;
} memoria_config;

#endif /* MEMORIA_H_ */
