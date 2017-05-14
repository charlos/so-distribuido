/*
 * memory_protocol.h
 *
 *  Created on: 22/4/2017
 *      Author: Dante Romero
 */

#include <commons/log.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SHARED_LIBRARY_MEMORY_PROTOCOL_H_
#define SHARED_LIBRARY_MEMORY_PROTOCOL_H_

#define INIT_PROCESS_OC     1
#define	ASSIGN_PAGE_OC 		2
#define	READ_OC			 	3
#define	WRITE_OC			4
#define	END_PROCESS_OC		5

#define	SUCCESS				1

/**	╔═════════════════════════════════╗
	║ MEMORY - RECEIVE OPERATION CODE ║
	╚═════════════════════════════════╝ **/
typedef struct {
	uint32_t received_bytes;
	uint32_t ope_code;
} t_ope_code;

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
t_ope_code * recv_operation_code(t_log *, int *);

/**	╔═══════════════════════╗
	║ MEMORY - INIT PROCESS ║
	╚═══════════════════════╝ **/
typedef struct {
	uint32_t received_bytes;
	uint32_t pid;
	uint32_t pages;
} t_init_process_request;

typedef struct {
	uint8_t received_bytes;
	uint8_t resp_code;
} t_init_process_response;

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
void memory_init_process(int, int, int);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
t_init_process_request * init_process_recv_req(t_log *, int *);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
void init_process_send_resp(int *, int);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
t_init_process_response * memory_init_process_recv_resp(int);

/**	╔════════════════╗
	║ MEMORY - WRITE ║
	╚════════════════╝ **/
typedef struct {
	uint32_t received_bytes;
	uint32_t pid;
	uint32_t page;
	uint32_t offset;
	uint32_t size;
	void * buffer;
} t_write_request;

typedef struct {
	uint8_t received_bytes;
	uint8_t resp_code;
} t_write_response;

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
void memory_write(int, int, int, int, int, int, void *);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
t_write_request * write_recv_req(t_log *, int *);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
void write_send_resp(int *, int);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
t_write_response * memory_write_recv_resp(int);

/**	╔═══════════════╗
	║ MEMORY - READ ║
	╚═══════════════╝ **/
typedef struct {
	uint32_t received_bytes;
	uint32_t pid;
	uint32_t page;
	uint32_t offset;
	uint32_t size;
} t_read_request;

typedef struct {
	uint8_t received_bytes;
	uint8_t resp_code;
	uint8_t buffer_size;
	void * buffer;
} t_read_response;

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
void memory_read(int, int, int, int, int);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
t_read_request * read_recv_req(t_log *, int *);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
void read_send_resp(int *, int, int, void *);

/**
 * @NAME
 * @DESC
 *
 * @PARAMS
 */
t_read_response * memory_read_recv_resp(int);


#endif /* SHARED_LIBRARY_MEMORY_PROT_H_ */
