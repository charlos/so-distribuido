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
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include <shared-library/file_system_prot.h>
#include <shared-library/socket.h>
#include <commons/log.h>
#include "file-system.h"
#include <sys/types.h>
#include <unistd.h>


#define	SOCKET_BACKLOG 			100
int listenning_socket;
t_file_system_conf * file_system_conf;

void * bitmap_mfile_ptr;
size_t size;
t_bitarray * bitmap;

t_log * logger;

void load_file_system_properties(void);
void print_file_system_properties(void);
void load(void);
int unmap_bitmap_f(void);
int get_available_block(void);

void handshake(int *);
void validate_file(int *);
void create_file(int *);
void delete_file(int *);
void read_file(int *);
void write_file(int *);

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
	// files directory
	char * files_dir_path = string_from_format("%s/Archivos", (file_system_conf->mount_point));
	mkdir(files_dir_path, S_IRWXU | S_IRWXG | S_IRWXO);

	// files directory
	char * blocks_dir_path = string_from_format("%s/Bloques", (file_system_conf->mount_point));
	mkdir(blocks_dir_path, S_IRWXU | S_IRWXG | S_IRWXO);

	// metadata
	char * metadata_dir_path = string_from_format("%s/Metadata", (file_system_conf->mount_point));
	mkdir(metadata_dir_path, S_IRWXU | S_IRWXG | S_IRWXO);

	char * metadata_file_path = string_from_format("%s/Metadata.bin", metadata_dir_path);
	FILE * metadata_file = fopen(metadata_file_path, "w");
	fprintf(metadata_file,"TAMANIO_BLOQUES=%d\nCANTIDAD_BLOQUES=%d\nMAGIC_NUMBER=SADICA", BLOCK_SIZE, BLOCKS);
	fclose(metadata_file);

	// bitmap
	char * bitmap_file_path = string_from_format("%s/Bitmap.bin", metadata_dir_path);
	FILE * bitmap_file = fopen(bitmap_file_path, "wb");
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

	free(bitmap_file_path);
	free(metadata_file_path);
	free(metadata_dir_path);
	free(blocks_dir_path);
	free(files_dir_path);

	logger = log_create("/home/utnso/fs.log", "memory_process", true, LOG_LEVEL_TRACE); // TODO : ver donde va el log
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
			handshake(client_socket);
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
			read_file(client_socket);
			break;
		case FS_WRITE_FILE_OC:
			write_file(client_socket);
			break;
		default:;
		}
		ope_code = recv_operation_code(client_socket, logger);
	}
	close_client(* client_socket);
	free(client_socket);
	return;
}

void handshake(int * client_socket) {
	fs_handshake_resp(client_socket, SUCCESS);
}

void validate_file(int * client_socket) {
	t_v_file_req * v_req = v_file_recv_req(client_socket, logger);
	char * path = string_from_format("%s/Archivos%s", (file_system_conf->mount_point), v_req->path);
	int resp_code;
	struct stat s; // information about the file
	int status = stat(path, &s);
	if (status < 0) {
		resp_code = ENOENT;
	} else {
		switch (s.st_mode & S_IFMT) {
		case S_IFREG:
			resp_code = ISREG;
			break;
		default:
			resp_code = ISNOTREG;
			break;
		}
	}
	free(path);
	free(v_req->path);
	free(v_req);
	v_file_send_resp(client_socket, resp_code);
}

void create_file(int * client_socket) {
	t_c_file_req * c_req = c_file_recv_req(client_socket, logger);
	int resp_code;
	char * path = string_from_format("%s/Archivos%s", (file_system_conf->mount_point), c_req->path);
	FILE * file = fopen(path, "w");
	int available_block = get_available_block();
	if (available_block < 0) {
		fclose(file);
		remove(path);
		resp_code = ENOSPC;
	} else {
		fprintf(file,"TAMANIO=0\n", BLOCK_SIZE);
		fprintf(file,"BLOQUES=[%d]\0", available_block);
		char * b_path = string_from_format("%s/Bloques/%d.bin", (file_system_conf->mount_point), available_block);
		FILE * block = fopen(b_path, "w");
		fclose(block);
		fclose(file);
		free(b_path);
		resp_code = SUCCESS;
	}
	free(path);
	free(c_req->path);
	free(c_req);
	c_file_send_resp(client_socket, resp_code);
}

int get_available_block(void) {
	int pos = 0;
	bool its_busy;
	while (pos < BLOCKS) {
		its_busy = bitarray_test_bit(bitmap, pos);
		if (!its_busy) {
			bitarray_set_bit(bitmap, pos);
			return pos;
		}
		pos++;
	}
	return -1;
}

void delete_file(int * client_socket) {
	t_d_file_req * d_req = d_file_recv_req(client_socket, logger);

	char * path = string_from_format("%s/Archivos%s", (file_system_conf->mount_point), d_req->path);
	FILE * file = fopen(path, "r");
	t_config * file_metadata = config_create(path);
	char ** used_blocks = config_get_array_value(file_metadata, "BLOQUES");

	unsigned int block;
	int pos = 0;
	while (used_blocks[pos] != NULL) {
		block = atoi(used_blocks[pos]);
		bitarray_clean_bit(bitmap, block);
		free(used_blocks[pos]);
		pos++;
	}

	fclose(file);
	remove(path);

	free(path);
	free(d_req->path);
	free(d_req);
	d_file_send_resp(client_socket, SUCCESS);
}

void read_file(int * client_socket) {
	t_fs_read_req * r_req = fs_read_recv_req(client_socket, logger);

	char * path = string_from_format("%s/Archivos%s", (file_system_conf->mount_point), r_req->path);
	FILE * file = fopen(path, "r");

	if (file == NULL) {
		//En caso de que se soliciten datos o se intenten guardar datos en un archivo inexistente el File System
		//deberá retornar un error de Archivo no encontrado
	}

	t_config * file_metadata = config_create(path);
	char ** used_blocks = config_get_array_value(file_metadata, "BLOQUES");
	int file_size = config_get_int_value(file_metadata, "TAMANIO");
	int offset = (r_req->offset);
	int size = (r_req->size);

	void * buff;
	int resp_code;
	int bytes_transferred = 0;

	if (offset < (file_size - 1)) {
		if (offset + size > file_size)
			size = file_size - offset;

		buff = malloc((sizeof(char)) * size);
		int movs = offset / BLOCK_SIZE;
		used_blocks += movs;
		offset = offset - (BLOCK_SIZE * movs);
		int bytes_reading = ((BLOCK_SIZE - offset) >= size) ? size : BLOCK_SIZE - offset;

		// reading block
		char * b_path = string_from_format("%s/Bloques/%s.bin", (file_system_conf->mount_point), used_blocks);
		FILE * block = fopen(b_path, "r");
		fseek(block, offset, SEEK_SET);
		fread(buff, bytes_reading, 1, block);
		fclose(block);
		free(b_path);
		// END reading block

		int buff_pos = bytes_reading;
		int bytes_to_read = size - bytes_reading;

		while (bytes_to_read > 0) {
			used_blocks++;
			bytes_reading = (bytes_to_read >= BLOCK_SIZE) ? BLOCK_SIZE : bytes_to_read;

			// reading block
			b_path = string_from_format("%s/Bloques/%s.bin", (file_system_conf->mount_point), used_blocks);
			block = fopen(b_path, "r");
			fseek(block, 0, SEEK_SET);
			fread(buff + buff_pos, bytes_reading, 1, block);
			fclose(block);
			free(b_path);
			// END reading block

			buff_pos = buff_pos + bytes_reading;
			bytes_to_read = bytes_to_read - bytes_reading;
		}
		bytes_transferred = size;
		resp_code = SUCCESS;
	} else {
		resp_code = ERROR; // TODO : error handler
	}

	fs_read_send_resp(client_socket, resp_code, bytes_transferred, buff);

	int pos = 0;
	while (used_blocks[pos] != NULL) {
		free(used_blocks[pos]);
		pos++;
	}
	free(buff);
	free(r_req->path);
	free(r_req);
}

void write_file(int * client_socket) {
	t_fs_write_req * w_req = fs_write_recv_req(client_socket, logger);

	char * path = string_from_format("%s/Archivos%s", (file_system_conf->mount_point), w_req->path);
	FILE * file = fopen(path, "r+");

	if (file == NULL) {
		//En caso de que se soliciten datos o se intenten guardar datos en un archivo inexistente el File System
		//deberá retornar un error de Archivo no encontrado
	}

	t_config * file_metadata = config_create(path);
	char ** used_blocks = config_get_array_value(file_metadata, "BLOQUES");
	t_list * blocks_list = list_create();
	int pos = 0;
	int block_used;
	while (used_blocks[pos] != NULL) {
		list_add(blocks_list, atoi(used_blocks[pos]));
		free(used_blocks[pos]);
		pos++;
	}
	int b_elements_count = (blocks_list->elements_count);

	int file_size = config_get_int_value(file_metadata, "TAMANIO");
	int offset = (w_req->offset);
	int size = (w_req->size);

	int bytes_availables_in_block;

	if ((offset + size) > file_size) {
		// expanding file
		int bytes_to_expand = (offset + size) - file_size;
		bytes_availables_in_block = BLOCK_SIZE - (file_size - (((blocks_list->elements_count) - 1) * BLOCK_SIZE));
		bytes_to_expand = bytes_to_expand - bytes_availables_in_block;

		while (bytes_to_expand > 0) {
			// adding block
			int available_block = get_available_block();
			if (available_block >= 0) {
				list_add(blocks_list, available_block);
				bytes_to_expand -= BLOCK_SIZE;
			} else {
				pos = b_elements_count;
				while (pos < (blocks_list->elements_count)) {
					bitarray_clean_bit(bitmap, (int) list_remove(blocks_list, pos));
					pos++;
				}
				fs_write_send_resp(client_socket, ENOSPC);
				return;
			}
		}
	}

	int block_n = (offset / BLOCK_SIZE);
	offset = offset - (block_n * BLOCK_SIZE);
	bytes_availables_in_block = BLOCK_SIZE - offset;
	int bytes_to_write = size;
	int buff_pos = 0;
	int bytes_writing;
	char * b_path;
	FILE * block;

	while (bytes_to_write > 0) {
		bytes_writing = (bytes_to_write >= bytes_availables_in_block) ? bytes_availables_in_block : bytes_to_write;

		// writing block
		b_path = string_from_format("%s/Bloques/%s.bin", (file_system_conf->mount_point), list_get(blocks_list, block_n));
		block = fopen(b_path, "r+");
		fseek(block, offset, SEEK_SET);
		fwrite((w_req->buffer) + buff_pos, bytes_writing, 1, block);
		fclose(block);
		free(b_path);
		// END writing block

		block_n++;
		bytes_to_write = bytes_to_write - bytes_writing;
		buff_pos = buff_pos + bytes_writing;
		bytes_availables_in_block = BLOCK_SIZE;
		offset = 0;
	}

	free(w_req->path);
	free(w_req->buffer);
	free(w_req);

	fs_write_send_resp(client_socket, SUCCESS);

}

// https://techoverflow.net/2013/04/05/how-to-use-mkdir-from-sysstat-h/
// https://www.lemoda.net/c/mmap-example/
// https://www.codingunit.com/c-tutorial-file-io-using-text-files
// https://github.com/dromero-7854/sisoputnfrba-tp-2016-2c-Stranger-Code/blob/master/entrenador/src/Entrenador/entrenador.c
// https://www.tutorialspoint.com/c_standard_library/c_function_fread.htm
// https://www.tutorialspoint.com/c_standard_library/c_function_fseek.htm
// https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
// https://www.tutorialspoint.com/c_standard_library/c_function_remove.htm
