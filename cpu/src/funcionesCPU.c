/*
 * funcionesCPU.c
 *
 *  Created on: 23/5/2017
 *      Author: gtofaletti
 */


#include "funcionesCPU.h"

extern t_PCB* pcb;
extern t_page_offset* nextPageOffsetInStack;
extern AnSISOP_funciones * funciones;
extern AnSISOP_kernel * func_kernel;
extern t_cpu_conf* cpu_conf;
extern t_log* logger;
extern int pagesize;
extern int flagDesconeccion;

t_element_stack* nuevoContexto(){
	t_element_stack* regIndicestack = malloc(sizeof(t_element_stack));

	regIndicestack->args = list_create();
	regIndicestack->vars = list_create();

	regIndicestack->retPos=0;
	regIndicestack->retVar = malloc(sizeof(posicion_memoria));
	regIndicestack->retVar->offset = 0;
	regIndicestack->retVar->pagina = 0;
	regIndicestack->retVar->size = 0;

	stack_push(pcb->indice_stack,regIndicestack);

	return regIndicestack;
}

void agregarAStack(t_args_vars* new_arg_var,int tipo){
	t_element_stack* regContextStack = list_get(pcb->indice_stack,pcb->SP);

	switch (tipo) {
	case VAR_STACK:
		arg_var_push(regContextStack->vars, new_arg_var);
		break;
	case ARG_STACK:
		arg_var_push(regContextStack->args, new_arg_var);
		break;
	}
}

t_element_stack* stack_pop(t_stack* stack){
	t_element_stack* elemento = list_remove(stack, pcb->SP);
	pcb->SP--;
	return elemento;
}

void stack_push(t_stack* stack, t_element_stack* element){
	list_add(stack, element);
	pcb->SP++;
}


t_args_vars* arg_var_pop(t_list* lista){
	t_args_vars *elemento = list_remove(lista, list_size(lista) - 1);
	return elemento;
}

void arg_var_push(t_list* lista, t_args_vars* element){
	list_add(lista, element);
}

void eliminarContexto(t_element_stack* contexto){

	list_destroy_and_destroy_elements(contexto->args, (void*) args_vars_destroy);
	list_destroy_and_destroy_elements(contexto->vars, (void*) args_vars_destroy);

//	free(contexto->retVar);
	free(contexto);
}

void args_vars_destroy(t_args_vars *elem) {
   free(elem);
}

void inicializarFuncionesParser(void) {
	funciones = malloc(sizeof(AnSISOP_funciones));
	funciones->AnSISOP_definirVariable = definirVariable;
	funciones->AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable;
	funciones->AnSISOP_dereferenciar = dereferenciar;
	funciones->AnSISOP_asignar = asignar;
	funciones->AnSISOP_irAlLabel = irAlLabel;
	funciones->AnSISOP_llamarSinRetorno = llamarSinRetorno;
	funciones->AnSISOP_llamarConRetorno = llamarConRetorno;
	funciones->AnSISOP_finalizar = finalizar;
	funciones->AnSISOP_obtenerValorCompartida = obtenerValorCompartida;
	funciones->AnSISOP_asignarValorCompartida = asignarValorCompartida;
	funciones->AnSISOP_retornar = retornar;

	func_kernel = malloc(sizeof(AnSISOP_kernel));
	func_kernel->AnSISOP_abrir = abrir;
	func_kernel->AnSISOP_cerrar = cerrar;
	func_kernel->AnSISOP_borrar = borrar;
	func_kernel->AnSISOP_escribir = escribir;
	func_kernel->AnSISOP_leer = leer;
	func_kernel->AnSISOP_moverCursor = moverCursor;
	func_kernel->AnSISOP_reservar = alocar;
	func_kernel->AnSISOP_liberar = liberar;
	func_kernel->AnSISOP_wait = waitParser;
	func_kernel->AnSISOP_signal = signalParser;

}
void procesarMsg(char * msg) {
		analizadorLinea(msg,funciones, func_kernel);
}

void load_properties(void) {
	t_config * conf = config_create("./cpu.cfg");
	cpu_conf = malloc(sizeof(t_cpu_conf));
	cpu_conf->kernel_ip = config_get_string_value(conf, "IP_KERNEL");
	cpu_conf->kernel_port = config_get_string_value(conf, "PUERTO_KERNEL");
	cpu_conf->memory_ip = config_get_string_value(conf, "IP_MEMORIA");
	cpu_conf->memory_port = config_get_string_value(conf, "PUERTO_MEMORIA");
	free(conf);
}


int calcularPaginaProxInstruccion(){
	int page,pc,bytes_codigo;

	bytes_codigo=0;
	t_indice_codigo* icodigo;
	for( pc = 0 ; pc < pcb->PC ; pc++){
		icodigo = ((t_indice_codigo*) pcb->indice_codigo)+ pc;
		bytes_codigo += icodigo->size;
	}
	page=bytes_codigo/pagesize;

	return page;
}

posicion_memoria buscoPos_recursiva(int sp){
	posicion_memoria resp;
	t_args_vars* lastPosStack;
	if (list_size(pcb->indice_stack)!=0){
		t_element_stack* lastcontext = list_get(pcb->indice_stack,sp);

		if (list_size(lastcontext->args)>0){
			lastPosStack = list_get(lastcontext->args,list_size(lastcontext->args)-1);
			resp.pagina = lastPosStack->pagina;
			resp.offset = lastPosStack->offset;
			resp.size = lastPosStack->size;
		} else{
			resp.pagina = pcb->cantidad_paginas;
			resp.offset = 0;
			resp.size = 0;
		}

		if (list_size(lastcontext->vars)>0){
			lastPosStack = list_get(lastcontext->vars,list_size(lastcontext->vars)-1);
			if(lastPosStack->pagina > resp.pagina){
				resp.pagina = lastPosStack->pagina;
				resp.offset = lastPosStack->offset;
				resp.size = lastPosStack->size;
			}else if(resp.pagina == lastPosStack->pagina){
				if(lastPosStack->offset > resp.offset){
					resp.offset = lastPosStack->offset;
					resp.size = lastPosStack->size;
				}else if(resp.size==0){
					resp.offset+=lastPosStack->size;
				}
			}
		}

		if(resp.pagina==pcb->cantidad_paginas && resp.offset==0 && resp.size==0){
			if (sp>0){
				resp = buscoPos_recursiva(sp-1);
			}else{
				return resp;
			}
		}

	} else {
		resp.pagina = pcb->cantidad_paginas;
		resp.offset = 0;
		resp.size = 0;
	}

	return resp;
}

void getNextPosStack(){

	posicion_memoria resp = buscoPos_recursiva(pcb->SP);

	nextPageOffsetInStack->page=resp.pagina;
	nextPageOffsetInStack->offset=resp.offset;
	if (nextPageOffsetInStack->offset+resp.size > pagesize){
		nextPageOffsetInStack->page++;
		nextPageOffsetInStack->offset=0;
	}else{
		nextPageOffsetInStack->offset+=resp.size;
	}

}

void updatePageOffsetAvailable(u_int32_t size){
	if (nextPageOffsetInStack->offset+size > pagesize){
		nextPageOffsetInStack->page++;
		nextPageOffsetInStack->offset=0;
	}
}

u_int32_t getPageofPos(t_puntero pos){
	return pos/pagesize;
}

u_int32_t getOffsetofPos(t_puntero pos){
	u_int32_t page = getPageofPos(pos);
	return pos - (page * pagesize);
}


void handlerDesconexion(int signum){
	if(signum==SIGUSR1){
		//printf("handlerDesconexion con signum==SIGUSR1");
		flagDesconeccion = 1;
	}
}

//
char * read_file(char * path) {
	FILE * file;
	char *buffer = malloc(255);

	if(strcmp(path, "1") == 0) {
		file = fopen("/home/utnso/eje.ansisop", "r");
	}
	else file = fopen(path, "r");

	if(file) {
		char* string = string_new();

		while(fgets(buffer, 255, (FILE*)file)) {
			string_append(&string, buffer);
		}
		if(feof(file)) {
			fclose(file);
			printf("\nMensaje compilado: %s \n", string);
			free(buffer);
			return string;
		}
		return NULL;
	}
	return NULL;

}

/*
t_PCB* crear_PCB_Prueba(){
	pcb = malloc(sizeof(t_PCB));
	pcb->pid = 1;

	char* PROGRAMA = read_file("1");

/*
			"#!/usr/bin/ansisop\n"
			"begin\n"
			"variables a, b\n"
			"alocar a 50\n"
			"*a = 5\n"
			"prints n *a\n"
			"b = 3 + *a\n"
			"*b = 10\n"
			"prints n *a\n"
			"liberar a\n"
			"end\n"
			"\n";
*/

/*			"#!/usr/bin/ansisop\n"
			"begin\n"
			"variables a, b\n"
			"b=1\n"
			"a=2\n"
			"!UnaVar=b+10\n"
			"!Global=a+10\n"
			"prints n !UnaVar+!Global\n"
			"end\n"
			"\n";
*/

/*			"#!/usr/bin/ansisop\n"
			"begin\n"
			"variables a, b, c, d\n"
			"alocar a 50\n"
			"alocar b 30\n"
			"liberar a\n"
			"liberar b\n"
			"end\n"
			"\n";
*/
/*
			"#!/usr/bin/ansisop\n"
			"begin\n"
			"variables a, b, c\n"
			"abrir a LC /archivo1.txt\n"
			"abrir b LC /archivo2.txt\n"
			"abrir c LC /archivo1.txt\n"
			"end\n"
			"\n";
*/


/*
			"#!/usr/bin/ansisop\n"
			"begin\n"
			"variables a\n"
			"a = 5\n"
			"prints n a\n"
			"end\n"
			"\n";
*/
/*
"#!/usr/bin/ansisop\n"
"#Programa para probar manejo de  archivos\n"
"begin\n"
"variables a, b\n"
"alocar a 50\n"
"abrir LC /utn/so/archivo\n"
"leer 3 a 5\n"
"escribir 0 a 5\n"
"liberar a\n"
"end\n"
"\n";

 */
/*
			 "#!/usr/bin/ansisop\n"
			"begin\n"
			"#comentario 1 12345678901234567890\n"
			"#comentario 2 12345678901234567890\n"
			"#comentario 3 12345678901234567890\n"
			"variables a,g\n"
			"a = 1\n"
			"g <- doble a\n"
			"prints s g\n"
			"end\n"
			"\n"
			"function doble\n"
			"variables f\n"
			"#comentario 4 12345678901234567890\n"
			"#xxxxxxxxxx\n"
			"f = $0 + $0\n"
			"return f\n"
			"end\n"
			"\n";
*/
/*
	char *programa = strdup(PROGRAMA);

	pcb->cantidad_paginas = 1;
	pcb->PC = 0;

	t_metadata_program* metadata = metadata_desde_literal(programa);

	pcb->SP = 0;
	pcb->cantidad_instrucciones = metadata->instrucciones_size;
	pcb->indice_codigo = obtener_indice_codigo(metadata);
	pcb->indice_etiquetas = obtener_indice_etiquetas(metadata);
	pcb->indice_stack = list_create();

	metadata_destruir(metadata);
	// Mandar Codigo a memoria

	return pcb;
}
*/


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
		memcpy(value, metadata->etiquetas+offset,sizeof(t_puntero_instruccion)); //	copio el puntero de instruccion
		offset += sizeof(t_puntero_instruccion);
		dictionary_put(indice_etiquetas, key, (void*)value);
	}
	return indice_etiquetas;
}
