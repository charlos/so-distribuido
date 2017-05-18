/*
 * memory_protocol.c
 *
 *  Created on: 22/4/2017
 *      Author: Dante Romero
 */
#include <commons/log.h>
#include <stdint.h>
#include <stdlib.h>
#include "memory_prot.h"
#include "socket.h"

/**	╔═════════════════════════════════╗
	║ MEMORY - RECEIVE OPERATION CODE ║
	╚═════════════════════════════════╝ **/
t_ope_code * recv_operation_code(int * client_socket, t_log * logger) {
	t_ope_code * request_ope_code = malloc(sizeof(t_ope_code));
	uint8_t prot_ope_code_size = 1;
	int received_bytes = socket_recv(client_socket, &(request_ope_code->ope_code), prot_ope_code_size);
	if (received_bytes <= 0) {
		request_ope_code->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request_ope_code;
	}
	request_ope_code->exec_code = SUCCESS;
	return request_ope_code;
}



/**	╔═══════════════════════╗
	║ MEMORY - INIT PROCESS ║
	╚═══════════════════════╝ **/
uint8_t memory_init_process(int server_socket, int pid, int pages, t_log * logger) {
	/**	╔═════════════════════════╦═══════════════╦═════════════════╗
		║ operation_code (1 byte) ║ pid (4 bytes) ║ pages (4 bytes) ║
		╚═════════════════════════╩═══════════════╩═════════════════╝ **/

	uint8_t prot_ope_code = 1;
	uint8_t prot_pid = 4;
	uint8_t prot_pages = 4;

	uint8_t  req_ope_code = INIT_PROCESS_OC;
	uint32_t req_pid = pid;
	uint32_t req_pages = pages;

	int msg_size = sizeof(char) * (prot_ope_code + prot_pid + prot_pages);
	void * request = malloc(msg_size);
	memcpy(request, &req_ope_code, prot_ope_code);
	memcpy(request + prot_ope_code, &req_pid, prot_pid);
	memcpy(request + prot_ope_code + prot_pid, &req_pages, prot_pages);
	socket_send(&server_socket, request, msg_size, 0);
	free(request);

//	t_init_process_response * response = malloc(sizeof(t_init_process_response));
	uint8_t resp_prot_code = 1;
	uint8_t response;
	int received_bytes = socket_recv(&server_socket, &response, resp_prot_code);
	if (received_bytes <= 0) {
		uint8_t error = 202;
		if (logger) log_error(logger, "------ SERVER %d >> disconnected", server_socket);
		return error;
	}
	return response;
};

t_init_process_request * init_process_recv_req(int * client_socket, t_log * logger) {
	t_init_process_request * request = malloc(sizeof(t_init_process_request));
	uint8_t prot_req_pid = 4;
	int received_bytes = socket_recv(client_socket, &(request->pid), prot_req_pid);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	uint8_t prot_req_pages = 4;
	received_bytes = socket_recv(client_socket, &(request->pages), prot_req_pages);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	request->exec_code = SUCCESS;
	return request;
}

void init_process_send_resp(int * client_socket, int resp_code) {
	uint8_t resp_prot_code = 1;
	int response_size = sizeof(char) * (resp_prot_code);
	void * response = malloc(response_size);
	memcpy(response, &resp_code, resp_prot_code);
	socket_write(client_socket, response, response_size);
	free(response);
}



/**	╔════════════════╗
	║ MEMORY - WRITE ║
	╚════════════════╝ **/
t_write_response * memory_write(int server_socket, int pid, int page, int offset, int size, int buffer_size, void * buffer, t_log * logger) {

	/**	╔═════════════════════════╦═══════════════╦════════════════╦══════════════════╦════════════════╦══════════════╦════════╗
		║ operation_code (1 byte) ║ pid (4 bytes) ║ page (4 bytes) ║ offset (4 bytes) ║ size (4 bytes) ║ buffer size  ║ buffer ║
		╚═════════════════════════╩═══════════════╩════════════════╩══════════════════╩════════════════╩══════════════╩════════╝ **/

	uint8_t prot_ope_code = 1;
	uint8_t prot_pid = 4;
	uint8_t prot_page = 4;
	uint8_t prot_offset = 4;
	uint8_t prot_size = 4;
	uint8_t prot_buffer_size = 4;

	uint8_t  req_ope_code = WRITE_OC;
	uint32_t req_pid = pid;
	uint32_t req_page = page;
	uint32_t req_offset = offset;
	uint32_t req_size = size;
	uint32_t req_buffer_size = buffer_size;

	int msg_size = sizeof(char) * (prot_ope_code + prot_pid + prot_page + prot_offset + prot_size + prot_buffer_size + buffer_size);
	void * request = malloc(msg_size);
	memcpy(request, &req_ope_code, prot_ope_code);
	memcpy(request + prot_ope_code, &req_pid, prot_pid);
	memcpy(request + prot_ope_code + prot_pid, &req_page, prot_page);
	memcpy(request + prot_ope_code + prot_pid + prot_page, &req_offset, prot_offset);
	memcpy(request + prot_ope_code + prot_pid + prot_page + prot_offset, &req_size, prot_size);
	memcpy(request + prot_ope_code + prot_pid + prot_page + prot_offset + prot_size, &req_buffer_size, prot_buffer_size);
	memcpy(request + prot_ope_code + prot_pid + prot_page + prot_offset + prot_size + prot_buffer_size, buffer, buffer_size);
	socket_send(&server_socket, request, msg_size, 0);
	free(request);

	t_write_response * response = malloc(sizeof(t_write_response));
	uint8_t resp_prot_code = 1;
	int received_bytes = socket_recv(&server_socket, &(response->resp_code), resp_prot_code);
	if (received_bytes <= 0) {
		response->exec_code = DISCONNECTED_SERVER;
		if (logger) log_error(logger, "------ SERVER %d >> disconnected", server_socket);
		return response;
	}
	response->exec_code = SUCCESS;
	return response;
};

t_write_request * write_recv_req(int * client_socket, t_log * logger) {
	t_write_request * request = malloc(sizeof(t_write_request));
	uint8_t prot_req_pid = 4;
	int received_bytes = socket_recv(client_socket, &(request->pid), prot_req_pid);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	uint8_t prot_req_page = 4;
	received_bytes = socket_recv(client_socket, &(request->page), prot_req_page);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	uint8_t prot_req_offset = 4;
	received_bytes = socket_recv(client_socket, &(request->offset), prot_req_offset);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	uint8_t prot_req_size = 4;
	received_bytes = socket_recv(client_socket, &(request->size), prot_req_size);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	uint8_t prot_buffer_size = 4;
	uint32_t buffer_size;
	received_bytes = socket_recv(client_socket, &buffer_size, prot_buffer_size);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	request->buffer = malloc(sizeof(char) * buffer_size);
	received_bytes = socket_recv(client_socket, request->buffer, buffer_size);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	request->exec_code = SUCCESS;
	return request;
}

void write_send_resp(int * client_socket, int resp_code) {
	uint8_t resp_prot_code = 1;
	int response_size = sizeof(char) * (resp_prot_code);
	void * response = malloc(response_size);
	memcpy(response, &resp_code, resp_prot_code);
	socket_write(client_socket, response, response_size);
	free(response);
}



/**	╔═══════════════╗
	║ MEMORY - READ ║
	╚═══════════════╝ **/
t_read_response * memory_read(int server_socket, int pid, int page, int offset, int size, t_log * logger) {

	/**	╔═════════════════════════╦═══════════════╦════════════════╦══════════════════╦════════════════╗
		║ operation_code (1 byte) ║ pid (4 bytes) ║ page (4 bytes) ║ offset (4 bytes) ║ size (4 bytes) ║
		╚═════════════════════════╩═══════════════╩════════════════╩══════════════════╩════════════════╝ **/

	uint8_t prot_ope_code = 1;
	uint8_t prot_pid = 4;
	uint8_t prot_page = 4;
	uint8_t prot_offset = 4;
	uint8_t prot_size = 4;

	uint8_t  req_ope_code = READ_OC;
	uint32_t req_pid = pid;
	uint32_t req_page = page;
	uint32_t req_offset = offset;
	uint32_t req_size = size;

	int msg_size = sizeof(char) * (prot_ope_code + prot_pid + prot_page + prot_offset + prot_size);
	void * request = malloc(msg_size);
	memcpy(request, &req_ope_code, prot_ope_code);
	memcpy(request + prot_ope_code, &req_pid, prot_pid);
	memcpy(request + prot_ope_code + prot_pid, &req_page, prot_page);
	memcpy(request + prot_ope_code + prot_pid + prot_page, &req_offset, prot_offset);
	memcpy(request + prot_ope_code + prot_pid + prot_page + prot_offset, &req_size, prot_size);
	socket_send(&server_socket, request, msg_size, 0);
	free(request);

	t_read_response * response = malloc(sizeof(t_read_response));
	uint8_t resp_prot_code = 1;
	int received_bytes = socket_recv(&server_socket, &(response->resp_code), resp_prot_code);
	if (received_bytes <= 0) {
		response->exec_code = DISCONNECTED_SERVER;
		if (logger) log_error(logger, "------ SERVER %d >> disconnected", server_socket);
		return response;
	}
	uint8_t resp_prot_buff_size = 1;
	received_bytes = socket_recv(&server_socket, &(response->buffer_size), resp_prot_buff_size);
	if (received_bytes <= 0) {
		response->exec_code = DISCONNECTED_SERVER;
		if (logger) log_error(logger, "------ SERVER %d >> disconnected", server_socket);
		return response;
	}
	if ((response->buffer_size) > 0) {
		received_bytes = socket_recv(&server_socket, response->buffer, response->buffer_size);
		if (received_bytes <= 0) {
			response->exec_code = DISCONNECTED_SERVER;
			if (logger) log_error(logger, "------ SERVER %d >> disconnected", server_socket);
			return response;
		}
	}
	response->exec_code = SUCCESS;
	return response;
};

t_read_request * read_recv_req(int * client_socket, t_log * logger) {
	t_read_request * request = malloc(sizeof(t_read_request));
	uint8_t prot_req_pid = 4;
	int received_bytes = socket_recv(client_socket, &(request->pid), prot_req_pid);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	uint8_t prot_req_page = 4;
	received_bytes = socket_recv(client_socket, &(request->page), prot_req_page);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	uint8_t prot_req_offset = 4;
	received_bytes = socket_recv(client_socket, &(request->offset), prot_req_offset);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	uint8_t prot_req_size = 4;
	received_bytes = socket_recv(client_socket, &(request->size), prot_req_size);
	if (received_bytes <= 0) {
		request->exec_code = DISCONNECTED_CLIENT;
		if (logger) log_error(logger, "------ CLIENT %d >> disconnected", * client_socket);
		return request;
	}
	request->exec_code = SUCCESS;
	return request;
}

void read_send_resp(int * client_socket, int resp_code, int buffer_size, void * buffer) {
	uint8_t resp_prot_code = 1;
	uint8_t resp_prot_buff_size = 4;
	int response_size = sizeof(char) * (resp_prot_code + resp_prot_buff_size + ((buffer_size > 0) ? buffer_size : 0));
	void * response = malloc(response_size);
	memcpy(response, &resp_code, resp_prot_code);
	memcpy(response + resp_prot_code, &buffer_size, resp_prot_buff_size);
	if (buffer_size > 0) {
		memcpy(response + resp_prot_code + resp_prot_buff_size, buffer, buffer_size);
	}
	socket_write(client_socket, response, response_size);
	free(response);
}
