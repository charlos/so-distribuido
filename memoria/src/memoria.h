/*
 * memoria.h
 *
 *  Created on: 12/4/2017
 *     Authors: Carlos Flores, Gustavo Tofaletti, Dante Romero
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

typedef struct {
	uint32_t port;
	uint32_t frames;
	uint32_t frame_size;
	uint32_t cache_entries;
	uint32_t cache_x_process;
	char *   cache_algorithm;
	uint32_t memory_delay;
	char *   logfile;
	char *   consolefile;
} t_memory_conf;

typedef struct {
	uint32_t frame;
	int32_t  pid;
	uint32_t page;
} t_reg_invert_table;

typedef struct {
	uint32_t pid;
	uint32_t pages_count;
} t_reg_pages_process_table;

typedef struct {
	int32_t pid;
	int32_t page;
	void *  content;
} t_cache_memory;

typedef struct {
	int32_t pid;
	int16_t entries;
} t_cache_x_process;

#endif /* MEMORIA_H_ */
