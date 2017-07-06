/*
 * solicitudes.c
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include "solicitudes.h"
#include <parser/metadata_program.h>
#include <parser/parser.h>

extern t_list* tabla_archivos;

void solve_request(t_info_socket_solicitud* info_solicitud){
	uint8_t operation_code;
	uint32_t cant_paginas, direcc_logica, direcc_fisica;
	t_puntero bloque_heap_ptr;
	int status, resp;
	char* buffer;
	t_pedido_reservar_memoria* pedido;
	t_pedido_liberar_memoria* liberar;
	t_pagina_heap* pagina;
	t_read_response* respuesta_pedido_pagina;
	t_PCB* pcb;
	t_metadata_program* metadata;
	t_table_file* tabla_proceso;
	t_heapMetadata* metadata_bloque;
	t_codigo_proceso* info_proceso;
	t_shared_var* variable_recibida;
	t_archivo * escritura;
	int socket_consola;
	char * informacion;

    int length_direccion, pid, size_nombre;
    char* direccion;
    t_banderas flags;
    char* nombre_variable;

	status = connection_recv(info_solicitud->file_descriptor, &operation_code, &buffer);
	if(status <= 0){
		FD_CLR(info_solicitud->file_descriptor, (info_solicitud->set));
		operation_code = 555;
	}

	switch(operation_code){
	case OC_SOLICITUD_PROGRAMA_NUEVO: {

		cant_paginas = calcular_paginas_necesarias(buffer);
		pcb = crear_PCB();

		int saved_socket = info_solicitud->file_descriptor;
		t_par_socket_pid * parnuevo = malloc(sizeof(t_par_socket_pid));
		parnuevo->pid = pcb->pid;
		parnuevo->socket = saved_socket;
		list_add(tabla_sockets_procesos, parnuevo);

		status = 0;
		status = memory_init_process(memory_socket, pcb->pid, cant_paginas, logger);

		log_trace(logger, "SOCKET DEL PID %d: %d", pcb->pid, saved_socket);
		if(status == -1){
			log_error(logger, "Se desconecto Memoria");
			exit(1);
		}

		mandar_codigo_a_memoria(buffer, pcb->pid);
		pcb->cantidad_paginas += cant_paginas;
		pcb->PC = 0;

		t_codigo_proceso* paginas_de_codigo = malloc(sizeof(t_codigo_proceso));
		paginas_de_codigo->pid = pcb->pid;
		paginas_de_codigo->paginas_codigo = cant_paginas;
		list_add(tabla_paginas_por_proceso, paginas_de_codigo);

		log_trace(logger, "Mandando PID");
		printf("Mandando PID\n");

		connection_send(info_solicitud->file_descriptor, OC_NUEVA_CONSOLA_PID, &(pcb->pid));

		metadata = metadata_desde_literal(buffer);

		pcb->SP = 0;
		pcb->cantidad_instrucciones = metadata->instrucciones_size;
		pcb->indice_codigo = obtener_indice_codigo(metadata);
		pcb->indice_etiquetas = obtener_indice_etiquetas(metadata);
		metadata_destruir(metadata);

		//Creo tabla de archivos del proceso
		tabla_proceso = malloc(sizeof(t_table_file));
		tabla_proceso->pid = pcb->pid;
		tabla_proceso->tabla_archivos = crearTablaArchProceso();

		break;
	}
	case OC_FUNCION_RESERVAR:
		log_trace(logger, "OP OC_FUNCION_RESERVAR dentro de kernel-solicitudes.c");
		pedido = (t_pedido_reservar_memoria*)buffer;
		info_proceso = buscar_codigo_de_proceso(pedido->pid);
		pagina = obtener_pagina_con_suficiente_espacio(pedido->pid, pedido->espacio_pedido);
		if(pagina == NULL){
			memory_assign_pages(memory_socket, pedido->pid, 1, logger);

			tabla_heap_agregar_pagina(pedido->pid);
			pagina = obtener_pagina_con_suficiente_espacio(pedido->pid, pedido->espacio_pedido);

			t_heapMetadata* meta_pag_nueva =crear_metadata_libre(TAMANIO_PAGINAS);

			// Escribimos la metadata de la nueva pagina en Memoria
			memory_write(memory_socket, pedido->pid, (pagina->nro_pagina + info_proceso->paginas_codigo), 0, sizeof(t_heapMetadata), sizeof(t_heapMetadata), meta_pag_nueva, logger);

			free(meta_pag_nueva);
		}
		respuesta_pedido_pagina = memory_read(memory_socket, pedido->pid, (pagina->nro_pagina + info_proceso->paginas_codigo), 0, TAMANIO_PAGINAS, logger);
		bloque_heap_ptr = buscar_bloque_disponible(respuesta_pedido_pagina->buffer, pedido->espacio_pedido);
		log_trace(logger, "primer puntero: %d", bloque_heap_ptr);
		marcar_bloque_ocupado(bloque_heap_ptr, respuesta_pedido_pagina->buffer, pedido->espacio_pedido);
		// Mandamos la pagina de heap modificada
		memory_write(memory_socket, pedido->pid, (pagina->nro_pagina + info_proceso->paginas_codigo), 0, TAMANIO_PAGINAS, TAMANIO_PAGINAS, respuesta_pedido_pagina->buffer, logger);
		log_trace(logger, "Escribe heap en memoria, en pagina: %d", (pagina->nro_pagina + info_proceso->paginas_codigo));
		modificar_pagina(pagina, pedido->espacio_pedido); // actualizamos los datos de la pagina en la tabla de heap

		// Mandamos puntero al programa que lo pidio
		obtener_direccion_relativa(&bloque_heap_ptr, pagina->nro_pagina, info_proceso->paginas_codigo);	//sumo el offset de las paginas de codigo, stack y heap
		connection_send(info_solicitud->file_descriptor, OC_RESP_RESERVAR, &bloque_heap_ptr);
		log_trace(logger, "Mando a cpu puntero de malloc pedido. Posicion: %d", bloque_heap_ptr);
		printf("Mandando heap\n");

		break;
	case OC_FUNCION_LIBERAR:
		liberar = buffer;		// liberar buffer
		log_trace(logger, "pedido de liberar punter. Posicion: %d", liberar->posicion);
		info_proceso = buscar_codigo_de_proceso(liberar->pid);

		obtener_direccion_logica(liberar, info_proceso->paginas_codigo); //le saco las paginas de codigo y stack

		log_trace(logger, "Yendo a leer pagina de memoria: %d", (liberar->nro_pagina + info_proceso->paginas_codigo));
		respuesta_pedido_pagina = memory_read(memory_socket, liberar->pid, (liberar->nro_pagina + info_proceso->paginas_codigo), 0, TAMANIO_PAGINAS, logger);
		metadata_bloque = leer_metadata((char*)respuesta_pedido_pagina->buffer + liberar->posicion);
		marcar_bloque_libre(metadata_bloque, (char*)respuesta_pedido_pagina->buffer + liberar->posicion);	// Marcamos la metadata del bloque como LIBRE en la pagina de heap
		tabla_heap_cambiar_espacio_libre(liberar, metadata_bloque->size);		// registramos el nuevo espacio libre en la tabla de paginas de heap que tiene kernel
		defragmentar(respuesta_pedido_pagina->buffer, liberar);
		if(pagina_vacia(liberar->pid, liberar->nro_pagina)){
			tabla_heap_sacar_pagina(liberar);
			liberar_pagina(liberar);
			break;
		}
		memory_write(memory_socket, liberar->pid, (pagina->nro_pagina + info_proceso->paginas_codigo), 0, TAMANIO_PAGINAS, TAMANIO_PAGINAS, respuesta_pedido_pagina->buffer, logger);
		break;
	case OC_FUNCION_ABRIR:
	    memcpy(pid, buffer, sizeof(int));
	    memcpy(length_direccion, buffer + sizeof(int),sizeof(int));
	    direccion = malloc(length_direccion);
	    memcpy(direccion, buffer + sizeof(int) + sizeof(int),length_direccion * sizeof(t_nombre_variable));
	    memcpy(&flags, buffer + sizeof(int) + sizeof(int) + length_direccion * sizeof(t_nombre_variable), sizeof(t_banderas));
	    resp = abrir_archivo(pid, direccion, flags);
	    //TODO respuesta al pedido de abrir archivo
	    connection_send(info_solicitud->file_descriptor, OC_RESP_ABRIR, &resp);
	    break;
	case OC_FUNCION_ESCRIBIR: {

		escritura = malloc(sizeof(t_archivo));
		//TODO si es FD 1 enviar a consola para imprimir
		escritura = (t_archivo *) buffer;
		log_trace(logger, "Llamada a escritura. FD: %d.	informacion: %s", escritura->descriptor_archivo, (char*)escritura->informacion);
		if(escritura->descriptor_archivo == 0)
		{
			//void * informacion_a_imprimir = obtener_informacion_a_imprimir(escritura->informacion, escritura->pid);
			//int socket_proceso = *(int*) dictionary_get(tabla_sockets_procesos, string_itoa(escritura->pid));

			int * _mismopid(t_par_socket_pid * target) {
				return escritura->pid == target->pid;
			}
			t_par_socket_pid * parEncontrado = (t_par_socket_pid*)list_find(tabla_sockets_procesos, _mismopid);
			int socket_proceso = parEncontrado->socket;
			char * inf = malloc(strlen((char*)escritura->informacion));
			strcpy(inf, (char*)escritura->informacion);

			connection_send(socket_proceso, OC_RESP_ESCRIBIR, inf);
		}
		break;
	}
	case OC_FUNCION_ESCRIBIR_VARIABLE:
		size_nombre = (int)*buffer;

		variable_recibida->nombre = malloc( size_nombre*sizeof(char)+1);
		memcpy(variable_recibida->nombre, (void*)buffer+sizeof(int), size_nombre*sizeof(char));
		variable_recibida->nombre[size_nombre]='\0';
		memcpy(&(variable_recibida->valor), (void*)buffer+sizeof(int)+size_nombre*sizeof(char),sizeof(int));

		log_trace(logger, "pedido de asignar el valor %d a la variable %s", variable_recibida->valor,variable_recibida->nombre);
		asignarValorVariable(variable_recibida);
		break;
	case OC_FUNCION_LEER_VARIABLE:
		nombre_variable	= (char*)buffer;
		log_trace(logger, "pedido de leer la variable %s",nombre_variable);
		t_valor_variable valor = leerValorVariable(nombre_variable);
		connection_send(info_solicitud->file_descriptor, OC_RESP_LEER_VARIABLE, &valor);
		break;
	default:
		fprintf(stderr, "Desconexion\n");
		return;
//		FD_CLR(info_solicitud->file_descriptor, (info_solicitud->set));
		//TODO Ver que hacer con cada desconexion
	}
	FD_SET(info_solicitud->file_descriptor, info_solicitud->set);
}

int calcular_paginas_de_codigo(char* codigo){
	int tamanio_codigo, paginas;
	tamanio_codigo = strlen(codigo);
	paginas = tamanio_codigo / TAMANIO_PAGINAS;
	if((paginas * TAMANIO_PAGINAS) < tamanio_codigo) return ++paginas;
	return paginas;
}

int calcular_paginas_necesarias(char* codigo){
	int paginas_de_codigo = calcular_paginas_de_codigo(codigo);
	return paginas_de_codigo + kernel_conf->stack_size;
}


void mandar_codigo_a_memoria(char* codigo, int pid){
	int i = 0, offset = 0, cant_a_mandar = strlen(codigo);
	while(cant_a_mandar > TAMANIO_PAGINAS){
		memory_write(memory_socket, pid, i, 0, TAMANIO_PAGINAS, TAMANIO_PAGINAS, codigo + offset, logger);
		offset += TAMANIO_PAGINAS;
		cant_a_mandar -= TAMANIO_PAGINAS;
		i++;
	}
	if(cant_a_mandar > 0){
		memory_write(memory_socket, pid, i, 0, cant_a_mandar, cant_a_mandar, codigo + offset, logger);
	}
}


t_indice_codigo* obtener_indice_codigo(t_metadata_program* metadata){
	int i = 0;
	log_trace(logger, "Dentro de obtener_indice_codigo");
	t_indice_codigo* indice_codigo = malloc(sizeof(t_indice_codigo) * metadata->instrucciones_size);
	for(i = 0; i < metadata->instrucciones_size; i++){
		memcpy((indice_codigo + i), (metadata->instrucciones_serializado )+ i, sizeof(t_indice_codigo));
		log_trace(logger, "Instrucción nro %d: offset %d, size %d", i,(indice_codigo + i)->offset, (indice_codigo + i)->size);
	}
	return indice_codigo;
}

t_dictionary* obtener_indice_etiquetas(t_metadata_program* metadata){
	t_dictionary* indice_etiquetas = dictionary_create();
	char* key;
	int *value, offset = 0;
	value = malloc(sizeof(t_puntero_instruccion));
	int i, cantidad_etiquetas_total = metadata->cantidad_de_etiquetas + metadata->cantidad_de_funciones;	// cantidad de tokens que espero sacar del bloque de bytes
	for(i=0; i < cantidad_etiquetas_total; i++){
		int cant_letras_token = 0;
		while(metadata->etiquetas[cant_letras_token + offset] != '\0')cant_letras_token++;
		key = malloc(cant_letras_token + 1);
		memcpy(key, metadata->etiquetas + offset, cant_letras_token + 1);		// copio los bytes de metadata->etiquetas desplazado las palabras que ya copie
		offset += cant_letras_token + 1;										// el offset suma el largo de la palabra + '\0'
		memcpy(value, metadata->etiquetas+offset,sizeof(t_puntero_instruccion));// copio el puntero de instruccion
		offset += sizeof(t_puntero_instruccion);
		dictionary_put(indice_etiquetas, key, *value);
	}
	return indice_etiquetas;
}

t_pagina_heap* obtener_pagina_con_suficiente_espacio(int pid, int espacio){
	bool tiene_mismo_pid_y_espacio_disponible(t_pagina_heap* pagina){
		return (pagina->pid == pid && pagina->espacio_libre >= (espacio + sizeof(t_heapMetadata)));
	}
	return list_find(tabla_paginas_heap, (void*)tiene_mismo_pid_y_espacio_disponible);
}

t_pagina_heap* crear_pagina_heap(int pid, int nro_pagina){
	t_pagina_heap* new = malloc(sizeof(t_pagina_heap));
	new->pid = pid;
	new->nro_pagina = nro_pagina;
	new->espacio_libre = TAMANIO_PAGINAS - sizeof(t_heapMetadata);
	return new;
}

void tabla_heap_agregar_pagina(int pid){
	bool _mismo_pid(t_pagina_heap* p){
		return p->pid == pid;
	}
	int ultimo_nro_pag;
	t_list* filtrados = list_filter(tabla_paginas_heap, (void*) _mismo_pid);
	t_pagina_heap* ultima_pagina = list_get(filtrados, list_size(filtrados) - 1);
	if(list_size(filtrados) == 0){
		ultimo_nro_pag = -1;
	} else {
		ultimo_nro_pag = ultima_pagina->nro_pagina;
	}
	t_pagina_heap* nueva_pag = crear_pagina_heap(pid, ultimo_nro_pag + 1);
	list_add(tabla_paginas_heap, nueva_pag);
}

t_heapMetadata* leer_metadata(void* pagina){
	t_heapMetadata* new = malloc(sizeof(t_heapMetadata));
	memcpy(&(new->size), pagina, sizeof(uint32_t));
	memcpy(&(new->isFree), ((char*)pagina) + sizeof(uint32_t), sizeof(bool));
	return new;
}

t_puntero buscar_bloque_disponible(void* pagina, int espacio_pedido){			// Devuelve el puntero a la metadata del bloque con espacio suficiente
	t_heapMetadata* metadata;
	t_puntero posicion_bloque;
	int espacio_total_bloque;
	t_puntero offset = 0;
	while(offset < TAMANIO_PAGINAS){
		metadata = leer_metadata(pagina + offset);
		if((metadata->isFree) && metadata->size >= (espacio_pedido + sizeof(t_heapMetadata))){		// hay un bloque con suficiente espacio libre
			free(metadata);
			return offset;
		}
		offset += sizeof(t_heapMetadata);			//avanza la cantidad de bytes de la metadata
		offset += metadata->size;					//avanza la cantidad de bytes del bloque de datos
	}
	return NULL;
}

void cambiar_metadata(t_heapMetadata* metadata, int espacio_pedido){
	metadata->isFree = 0;
	metadata->size = espacio_pedido;

}

void agregar_bloque_libre(char* pagina, int offset){
	int espacio_libre;
	espacio_libre = TAMANIO_PAGINAS - offset;
	t_heapMetadata* metadata_libre = crear_metadata_libre(espacio_libre);
	memcpy(pagina + offset, metadata_libre, sizeof(t_heapMetadata));
	free(metadata_libre);
}

t_pagina_heap* buscar_pagina_heap(int pid, int nro_pagina){
	bool _pagina_de_programa(t_pagina_heap* pagina){
		return (pagina->pid == pid && pagina->nro_pagina == nro_pagina);
	}
	return list_find(tabla_paginas_heap, (void*) _pagina_de_programa);
}

void marcar_bloque_libre(t_heapMetadata* metadata, char* pagina){
//	int offset = pedido_free->posicion % TAMANIO_PAGINAS;
//	t_heapMetadata* metadata = leer_metadata(pagina + offset);
	metadata->isFree = 1;
	memcpy(pagina, metadata, sizeof(t_heapMetadata));

}

void tabla_heap_cambiar_espacio_libre(t_pedido_liberar_memoria* pedido_free, int espacio_liberado){
	t_pagina_heap* pagina_de_tabla = buscar_pagina_heap(pedido_free->pid, pedido_free->nro_pagina);
	pagina_de_tabla->espacio_libre += espacio_liberado;
}

t_heapMetadata* crear_metadata_libre(uint32_t espacio){
	t_heapMetadata* new = malloc(sizeof(t_heapMetadata));
	new->isFree = 1;
	new->size = espacio - sizeof(t_heapMetadata);
	return new;
}

void marcar_bloque_ocupado(t_puntero bloque_heap_ptr, char* pagina, int espacio_pedido){
	t_heapMetadata* metadata = leer_metadata(pagina + bloque_heap_ptr);
	t_heapMetadata* metadata2;
	int sobrante = metadata->size - espacio_pedido;
	metadata->isFree = 0;
	metadata->size = espacio_pedido;
	memcpy(pagina + bloque_heap_ptr, metadata, sizeof(t_heapMetadata));
	metadata2 = crear_metadata_libre(sobrante);
	memcpy(pagina + bloque_heap_ptr + sizeof(t_heapMetadata) + metadata->size, metadata2, sizeof(t_heapMetadata));
	free(metadata);
	free(metadata2);
}

void modificar_pagina(t_pagina_heap* pagina, int espacio_ocupado){
	pagina->espacio_libre = pagina->espacio_libre - (espacio_ocupado + sizeof(t_heapMetadata));
}

int abrir_archivo(int pid, char* direccion, t_banderas flags){
	//TODO busco direccion en la tabla global: si está tomo posición y incremento open, lo agrego a la tabla del proceso con los permisos indicados
	//si no está en la global y hay permiso de creacion se agrega a la tabla global y a la del proceso
	//si no está en la global y no hay permiso de creación, se devuelve mensaje de error
	int fd_proceso;
	int fd_global; //guarda la posición del archivo en la tabla global
	fd_global = buscarArchivoTablaGlobal(direccion);

	if(fd_global>=0){
		fd_proceso = cargarArchivoTablaProceso(pid, fd_global, flags);
	}else{
		if(flags.creacion){
			fd_global = crearArchivoTablaGlobal(direccion);
		}else{
			//TODO enviar mensaje a consola: "No existe archivo" + el nombre del archivo
			fd_global = -1;
		}
	}
	return fd_global;
}

int crearArchivoTablaGlobal(char* direccion){
	int fd_global;
	t_global_file * filereg = malloc(sizeof(t_global_file));
	filereg->file = malloc(string_length(direccion));
	memcpy(filereg->file, direccion,string_length(direccion));
	filereg->open=1;
	//TODO semaforos!
	list_add(tabla_global_archivos,filereg);
	fd_global = list_size(tabla_global_archivos)-1;

	return fd_global;
}


int buscarArchivoTablaGlobal(char* direccion){
	//TODO semaforos!
	t_global_file * filereg;

	int i, resp;
	for (i=0;i<list_size(tabla_global_archivos);i++){
		filereg = list_get(tabla_global_archivos,i);
		if(strcmp(filereg->file, direccion)==0){
			break;
		}
	}

	if (i>=0){
		resp = i;
	}else{
		resp = -1;
	}
	return resp;

}

int cargarArchivoTablaProceso(int pid, int fd_global, t_banderas flags){
	t_process_file* file = malloc(sizeof(t_process_file));
	file->global_fd = fd_global;
	file->proceso_fd = nuevoFD_PID(pid);
	file->flags = flags;
	return file->proceso_fd;
}

int nuevoFD_PID(pid){
	t_table_file* tabla;
	tabla = getTablaArchivo(pid);
	int i;
	t_process_file* file;
	for(i=3;i<=list_size(tabla->tabla_archivos)+2;i++){
		file = list_get(tabla->tabla_archivos,i);
	}
	return i;
}

t_table_file* getTablaArchivo(int pid){

	   bool _findbyPID(t_table_file* reg){
		   return reg->pid==pid;
	   }
	   t_table_file* tabla;
	   tabla = list_find(tabla_archivos, (void*) _findbyPID);
	   return tabla;
}

t_list* crearTablaArchProceso(){
	t_list* tabla_archivo_proceso = list_create();

	t_process_file* file = malloc(sizeof(t_process_file));
	file->global_fd = 0;
	file->proceso_fd = 1;

	list_add(tabla_archivo_proceso,file);

	return tabla_archivo_proceso;
}

void defragmentar(char* pagina, t_pedido_liberar_memoria* pedido_free){
	int offset = 0;
	t_pagina_heap* pag_heap;
	t_heapMetadata* metadata, *metadata2;
	metadata = leer_metadata(pagina);
	offset += sizeof(t_heapMetadata);
	offset += metadata->size;
	while(offset < TAMANIO_PAGINAS){
		metadata2 = leer_metadata(pagina + offset);
		if(metadata->isFree && metadata2->isFree){
			juntar_bloques(metadata, metadata2);
			memcpy(pagina + offset - sizeof(t_heapMetadata) - metadata->size, metadata2, sizeof(t_heapMetadata));
			tabla_heap_cambiar_espacio_libre(pedido_free, sizeof(t_heapMetadata));
			break;
		}
		metadata = metadata2;
		offset += sizeof(t_heapMetadata);
		offset += metadata2->size;
	}
}

void juntar_bloques(t_heapMetadata* metadata1, t_heapMetadata* metadata2){
	metadata2->size += sizeof(t_heapMetadata) + metadata1->size;
}

void tabla_heap_sacar_pagina(t_pedido_liberar_memoria* pedido_free){		// TODO PREGUNTAR a gus si descubrio como sacar elemento de lista
	bool _pagina_de_programa(t_pagina_heap* pagina){
		return (pagina->pid == pedido_free->pid && pagina->nro_pagina == pedido_free->nro_pagina);
	}
	list_remove_by_condition(tabla_paginas_heap, (void*) _pagina_de_programa);
}

void liberar_pagina(t_pedido_liberar_memoria* pedido_free){
	memory_delete_page(memory_socket, pedido_free->pid, pedido_free->nro_pagina, logger);
}

bool pagina_vacia(int pid, int nro_pagina){
	t_pagina_heap* pag = buscar_pagina_heap(pid, nro_pagina);
	return (pag->espacio_libre == (TAMANIO_PAGINAS - sizeof(t_heapMetadata)));
}

t_codigo_proceso* buscar_codigo_de_proceso(int pid){
	bool _mismo_pid(t_codigo_proceso* c){
		return c->pid == pid;
	}
	return list_find(tabla_paginas_por_proceso, (void*) _mismo_pid);
}

void asignarValorVariable(t_shared_var* variable_recibida){
	bool _porNombreVarComp(t_shared_var* var){
		return strcmp(var->nombre,variable_recibida->nombre)==0;
	}
	t_shared_var* variable = list_find(tabla_variables_compartidas,(void*) _porNombreVarComp);
	variable->valor = variable_recibida->valor;
}

t_valor_variable leerValorVariable(char* nombre_variable){
	bool _porNombreVarComp(t_shared_var* var){
		return strcmp(var->nombre,nombre_variable)==0;
	}
	t_shared_var* variable = list_find(tabla_variables_compartidas,(void*) _porNombreVarComp);
	return variable->valor;
}

void * obtener_informacion_a_imprimir(t_puntero puntero, int pid) {

	int pagina = puntero/TAMANIO_PAGINAS;
	int offset = puntero % TAMANIO_PAGINAS;

	t_read_response * respuesta_memoria = memory_read(memory_socket, pid, pagina, offset, sizeof(t_puntero), logger);

	return respuesta_memoria->buffer;

}

void obtener_direccion_relativa(t_puntero* puntero, int nro_pagina_heap, int cantidad_paginas_codigo){
	*puntero += sizeof(t_heapMetadata);
	*puntero += TAMANIO_PAGINAS * nro_pagina_heap;
	*puntero += TAMANIO_PAGINAS * cantidad_paginas_codigo;
}

void obtener_direccion_logica(t_pedido_liberar_memoria* pedido_free, int cantidad_paginas_codigo){

	pedido_free->nro_pagina =(pedido_free->posicion / TAMANIO_PAGINAS) - cantidad_paginas_codigo;
	pedido_free->posicion = pedido_free->posicion % TAMANIO_PAGINAS;

}
