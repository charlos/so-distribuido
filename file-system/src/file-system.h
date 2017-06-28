/*
 * file-system.h
 *
 *  Created on: 15/6/2017
 *      Author: utnso
 */

#include <stdint.h>

#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#define BLOCK_SIZE 	64
#define BLOCKS 		5192

typedef struct {
	uint32_t port;
	char *   mount_point;
	char *   logfile;
} t_file_system_conf;

#endif /* FILE_SYSTEM_H_ */
