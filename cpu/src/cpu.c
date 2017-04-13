/*
 ============================================================================
 Name        : cpu.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <shared-library/socket.h>

int server_socket;

int main(void) {

	char * server_ip = "127.0.0.1";
	char * server_port = "5003";
	server_socket = connect_to_socket(server_ip, server_port);

	char * msg = NULL;
	size_t len = 0;
	ssize_t read;
	printf ("CPU : enter message ([ctrl + d] to quit)\n");
	while ((read = getline(&msg, &len, stdin)) != -1) {
		if (read > 0) {
			msg[read-1] = '\0';
			printf ("CPU : read %zd chars from stdin, allocated %zd bytes for message : %s\n", read, len, msg);
		}
		printf("CPU : sending message to server >> msg : %s\n", msg);
		// << sending message >>
		// operation code
		uint8_t prot_ope_code_size = 1;
		uint8_t req_ope_code = rand() % 20; // random int between 0 and 19
		// msg
		uint8_t prot_msg_size = 4;
		uint32_t req_msg_size = strlen(msg);

		int buffer_size = sizeof(char) * (prot_ope_code_size + prot_msg_size + req_msg_size);
		void * buffer = malloc(buffer_size);
		memcpy(buffer, &req_ope_code, prot_ope_code_size);
		memcpy(buffer + prot_ope_code_size, &req_msg_size, prot_msg_size);
		memcpy(buffer + prot_ope_code_size + prot_msg_size, msg, req_msg_size);
		socket_send(&server_socket, buffer, buffer_size, 0);
		free(buffer);


		// << receiving message >>
		// response code
		uint8_t prot_resp_code_size = 1;
		uint8_t resp_code;
		int received_bytes = socket_recv(&server_socket, &resp_code, prot_resp_code_size);
		if (received_bytes <= 0) {
			printf("CPU : server %d disconnected\n", server_socket);
			return EXIT_FAILURE;
		}
		printf("CPU : receiving message from server >> resp code : %d\n", resp_code);
		// msg response
		uint8_t prot_resp_size = 4;
		uint32_t resp_size;
		received_bytes = socket_recv(&server_socket, &resp_size, prot_resp_size);
		printf("CPU : receiving message from server >> resp size : %d\n", resp_size);
		if (received_bytes <= 0) {
			printf("CPU : server %d disconnected\n", server_socket);
			return EXIT_FAILURE;
		}
		char * resp = malloc(resp_size);
		received_bytes = socket_recv(&server_socket, resp, resp_size);
		printf("CPU : receiving message from server >> msg : %s\n", resp);
		if (received_bytes <= 0) {
			printf("CPU : server %d disconnected\n", server_socket);
			return EXIT_FAILURE;
		}
		free(resp);

		printf ("CPU : enter message ([ctrl + d] to quit)\n");

	}
	free(msg);
	return EXIT_SUCCESS;

}

