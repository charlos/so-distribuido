/*
 ============================================================================
 Name        : cpu.c
 Authors     : Carlos Flores, Gustavo Tofaletti, Dante Romero
 Version     :
 Description : Kernel Proccess
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <shared-library/socket.h>
#include <shared-library/memory_prot.h>
#include "kernel.h"

int memory_socket;

int main(void) {

	char * server_ip = "127.0.0.1";
	char * server_port = "5003";
	memory_socket = connect_to_socket(server_ip, server_port);

	// initializing process
	// << sending message >>
	uint8_t prot_ope_code = 1;
	uint8_t prot_pid = 4;
	uint8_t prot_req_pages = 4;

	uint8_t ope_code = INIT_PROCESS_OC;
	uint32_t pid = 15;
	uint32_t req_pages = 1;

	int buffer_size = sizeof(char) * (prot_ope_code + prot_pid + prot_req_pages);
	void * buffer = malloc(buffer_size);
	memcpy(buffer, &ope_code, prot_ope_code);
	memcpy(buffer + prot_ope_code, &pid, prot_pid);
	memcpy(buffer + prot_ope_code + prot_pid, &req_pages, prot_req_pages);
	socket_send(&memory_socket, buffer, buffer_size, 0);
	free(buffer);

	// << receiving message >>
	uint8_t resp_code;
	uint8_t prot_resp_code = 1;
	int received_bytes = socket_recv(&memory_socket, &resp_code, prot_resp_code);
	if (received_bytes <= 0) {
		printf("KERNEL : memory socket %d disconnected\n", memory_socket);
		return EXIT_FAILURE;
	}


	// writing memory
	// << sending message >>
	prot_ope_code = 1;
	prot_pid = 4;
	uint8_t prot_page = 4;
	uint8_t prot_offset = 4;
	uint8_t prot_size = 4;
	uint8_t prot_buf_size = 4;

	ope_code = WRITE_OC;
	pid = 15;
	uint32_t page = 1;
	uint32_t offset = 0;
	uint32_t size = 36;
	uint32_t buf_size = 36;
	char * buf = "Message to write and show in memory\0";


	buffer_size = sizeof(char) * (prot_ope_code + prot_pid + prot_page + prot_offset + prot_size + prot_buf_size + buf_size);
	buffer = malloc(buffer_size);
	memcpy(buffer, &ope_code, prot_ope_code);
	memcpy(buffer + prot_ope_code, &pid, prot_pid);
	memcpy(buffer + prot_ope_code + prot_pid, &page, prot_page);
	memcpy(buffer + prot_ope_code + prot_pid + prot_page, &offset, prot_offset);
	memcpy(buffer + prot_ope_code + prot_pid + prot_page + prot_offset, &size, prot_size);
	memcpy(buffer + prot_ope_code + prot_pid + prot_page + prot_offset + prot_size, &buf_size, prot_buf_size);
	memcpy(buffer + prot_ope_code + prot_pid + prot_page + prot_offset + prot_size + prot_buf_size, buf, buf_size);
	send(memory_socket, buffer, buffer_size, 0);
	free(buffer);

	return EXIT_SUCCESS;
}

t_stack* stack_create(){
	t_stack* stack = list_create();
	return stack;
}

t_link_element* stack_pop(t_stack* stack){
	t_link_element* elemento = list_remove(stack, list_size(stack) - 1);
	return elemento;
}

void stack_push(t_stack* stack, void* element){
	list_add(stack, element);
}
