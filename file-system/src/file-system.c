/*
 ============================================================================
 Name        : file-system.c
 Author      : Carlos Flores
 Version     :
 Copyright   : GitHub @Charlos
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include "file-system.h"

#define	SOCKET_BACKLOG 			100
int listenning_socket;
t_file_system_conf * file_system_conf;

FILE * metadata_file;
FILE * bitmap_file;
void * bitmap_mfile_ptr;
size_t size;
t_bitarray * bitmap;

void load_file_system_properties(void);
void print_file_system_properties(void);
void load(void);
int unmap_bitmap_f(void);

int main(int argc, char * argv[]) {

	load_file_system_properties();
	print_file_system_properties();
	load();

	// socket thread
	int * new_sock;
	listenning_socket = open_socket(SOCKET_BACKLOG, (file_system_conf->port));
	for (;;) {
		new_sock = malloc(1);
		* new_sock = accept_connection(listenning_socket);
		process_request(new_sock);
	}
	close_socket(listenning_socket);
	unmap_bitmap_f();
	return EXIT_SUCCESS;
}

/**
 * @NAME load_file_system_properties
 */
void load_file_system_properties(void) {
	t_config * conf = config_create("/home/utnso/file-system.cfg"); // TODO : Ver porque no lo toma del workspace
	file_system_conf = malloc(sizeof(t_file_system_conf));
	file_system_conf->port = config_get_int_value(conf, "PUERTO");
	file_system_conf->mount_point = config_get_string_value(conf, "PUNTO_MONTAJE");
}

/**
 * @NAME print_file_system_properties
 */
void print_file_system_properties(void) {
	printf(" >> SADICA file-system" );
	printf(" >> port ---------------> %u" , (file_system_conf->port));
	printf(" >> mount_point --------> %s" , (file_system_conf->mount_point));
}

/**
 * @NAME check
 */
static void check(int test, const char * message, ...) {
	if (test) {
		va_list args;
		va_start(args, message);
		vfprintf(stderr, message, args);
		va_end(args);
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}
}

void load(void) {

	// metadata
	char * metadata_dir_path = string_from_format("%s/Metadata", (file_system_conf->mount_point));
	mkdir(metadata_dir_path, S_IRWXU | S_IRWXG | S_IRWXO);

	char * metadata_file_path = string_from_format("%s/Metadata.bin", metadata_dir_path);
	metadata_file = fopen(metadata_file_path, "w");
	fprintf(metadata_file,"TAMANIO_BLOQUES=64\nCANTIDAD_BLOQUES=5192\nMAGIC_NUMBER=SADICA");
	fclose(metadata_file);

	// bitmap
	char * bitmap_file_path = string_from_format("%s/Bitmap.bin", metadata_dir_path);
	bitmap_file = fopen(bitmap_file_path, "wb");
	int j = BLOCKS / 8; // 1 byte = 8 bits
	char ch = '\0';
	int i = 0;
	while (i < j) {
		fwrite(&ch, sizeof(char), 1, bitmap_file);
		i++;
	}
	fclose(bitmap_file);

	int fd; // file descriptor
	struct stat s; int status; // information about the file

	fd = open(bitmap_file_path, O_RDWR);
	check(fd < 0, "open %s failed: %s", bitmap_file_path, strerror(errno));

	status = fstat(fd, &s);
	check(status < 0, "stat %s failed: %s", bitmap_file_path, strerror (errno));
	size = s.st_size;

	bitmap_mfile_ptr = mmap ((caddr_t) 0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	check((bitmap_mfile_ptr == MAP_FAILED), "mmap %s failed: %s", bitmap_file_path, strerror (errno));

	bitmap = bitarray_create_with_mode(bitmap_mfile_ptr, BLOCKS, MSB_FIRST);

	int pos = 0;
	while (pos < BLOCKS) {
		bitarray_clean_bit(bitmap, pos);
		pos++;
	}

	free(metadata_dir_path);
	free(metadata_file_path);
	free(bitmap_file_path);
}

/**
 * @NAME unmap_bitmap_f
 */
int unmap_bitmap_f(void) {
	check((munmap(bitmap_mfile_ptr, size) == -1), "munmap failed: %s", strerror(errno));
	return EXIT_SUCCESS;
}

/**
 * @NAME process_request
 */
void process_request(int * client_socket) {
	int ope_code = recv_operation_code(client_socket, logger);
	while (ope_code != DISCONNECTED_CLIENT) {
		printf(logger, " >> client %d >> operation code : %d", * client_socket, ope_code);
		switch (ope_code) {
		case FS_HANDSHAKE_OC:
			fs_handshake(client_socket);
			break;
		case FS_VALIDATE_FILE_OC:
			validate_file(client_socket);
			break;
		case FS_CREATE_FILE_OC:
			create_file(client_socket);
			break;
		case FS_DELETE_FILE_OC:
			delete_file(client_socket);
			break;
		case FS_READ_FILE_OC:
			read(client_socket);
			break;
		case FS_WRITE_FILE_OC:
			write(client_socket);
			break;
		default:;
		}
		ope_code = recv_operation_code(client_socket, logger);
	}
	close_client(* client_socket);
	free(client_socket);
	return;
}

void fs_handshake(int * client_socket) {
	fs_handshake_resp(client_socket, SUCCESS);
}

void validate_file(int * client_socket) {
	t_v_file_req * v_req = v_file_recv_req(client_socket, logger);
	// TODO : validate file
	free(v_req->path);
	free(v_req);
	v_file_send_resp(client_socket, SUCCESS);
}

void create_file(int * client_socket) {
	t_c_file_req * c_req = c_file_recv_req(client_socket, logger);
	// TODO : create file
	free(c_req->path);
	free(c_req);
	c_file_send_resp(client_socket, SUCCESS);
}

void delete_file(int * client_socket) {
	t_d_file_req * d_req = d_file_recv_req(client_socket, logger);
	// TODO : delete file
	free(d_req->path);
	free(d_req);
	d_file_send_resp(client_socket, SUCCESS);
}

void read(int * client_socket) {
	t_fs_read_req * r_req = fs_read_recv_req(client_socket, logger);
	// TODO : read file
	void * buffer = malloc(sizeof(char) * (r_req->size));
	fs_read_send_resp(client_socket, SUCCESS, (r_req->size), buffer);
	free(buffer);
	free(r_req->path);
	free(r_req);
}

void write(int * client_socket) {
	t_fs_write_req * w_req = fs_write_recv_req(client_socket, logger);
	// TODO : write file
	free(r_req->path);
	free(r_req->buffer);
	free(r_req);
	fs_write_send_resp(client_socket, SUCCESS);
}

// https://techoverflow.net/2013/04/05/how-to-use-mkdir-from-sysstat-h/
// https://www.lemoda.net/c/mmap-example/
// https://www.codingunit.com/c-tutorial-file-io-using-text-files
