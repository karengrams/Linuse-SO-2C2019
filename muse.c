#include "muse.h"

t_config* leer_config() {
	return config_create("/home/utnso/Escritorio/muse.config");
}

int leer_del_config(char* valor, t_config* archivo_config) {
	return config_get_int_value(archivo_config, valor);
}

void liberacion_de_recursos(void*mem,t_config *config) {
	config_destroy(config);
	free(mem);
	void _destroy_element(void *elemento) {
		frame *marco = (frame*) elemento;
		free(marco);
	}
	list_destroy_and_destroy_elements(FRAMES_TABLE, &_destroy_element);
	free(BIT_ARRAY_FRAMES->bitarray);
	bitarray_destroy(BIT_ARRAY_FRAMES);
}

uint32_t muse_alloc(t_proceso* proceso,int tam){
	uint32_t offset;
	t_list *tabla_de_segmento = proceso->tablaDeSegmentos;
	segment *ptr_segmento = buscar_segmento_heap_para_tam(tabla_de_segmento,tam); // Busca segmento de tipo heap con espacio
	if(!ptr_segmento){
		ptr_segmento= buscar_segmento_heap_expandible_para_tam(tabla_de_segmento,tam);
		if(ptr_segmento)
			expandir_segmento(ptr_segmento,tam);

		else
			ptr_segmento=crear_segmento(HEAP,tam,tabla_de_segmento);

	}

	offset = obtener_offset_para_tam(ptr_segmento,tam);

	segmentmetadata *paux_seg_metadata_ocupado = buscar_metadata_de_segmento_segun(offset,ptr_segmento);
	segmentmetadata *paux_seg_metadata_libre= (segmentmetadata*)malloc(sizeof(segmentmetadata));

	heapmetadata *paux_metadata_libre = (heapmetadata*) malloc(sizeof(heapmetadata));
	heapmetadata *paux_metadata_ocupado = paux_seg_metadata_ocupado->metadata;

	paux_metadata_libre->bytes=paux_metadata_ocupado->bytes-tam-sizeof(heapmetadata);
	paux_metadata_libre->ocupado=false;

	paux_seg_metadata_libre->posicion_inicial=offset+sizeof(heapmetadata)+tam;
	paux_seg_metadata_libre->metadata=paux_metadata_libre;
	list_add(ptr_segmento->metadatas,paux_seg_metadata_libre);
	paux_metadata_ocupado->bytes=tam;
	paux_metadata_ocupado->ocupado=true;
	ptr_segmento->tamanio+=tam;
	return ptr_segmento->base_logica+offset+sizeof(heapmetadata);
}

void muse_free(t_proceso *proceso, uint32_t direccion){
	segment *ptr_segmento = buscar_segmento_dada_una_direccion(direccion,proceso->tablaDeSegmentos);
	segmentmetadata *ptr_seg_metadata = buscar_metadata_para_liberar(direccion-ptr_segmento->base_logica,ptr_segmento);
	if(ptr_seg_metadata){
		heapmetadata *ptr_metadata = ptr_seg_metadata->metadata;
		ptr_metadata->ocupado=false;
		buddy_system(ptr_seg_metadata,ptr_segmento->metadatas);
	}
}

int main(void) {
	inicilizar_tabla_de_frames();
	inicializar_bitmap();
	t_config* config = leer_config();
	void *memoria = (void*) malloc(leer_del_config("MEMORY_SIZE", config));
	dividir_memoria_en_frames(memoria,leer_del_config("PAGE_SIZE", config),leer_del_config("MEMORY_SIZE", config));
	TAM_PAG = leer_del_config("PAGE_SIZE", config);



	liberacion_de_recursos(memoria,config);

	//Arranca a atender clientes
	/*
	 fd_set master;   // conjunto maestro de descriptores de fichero
	 fd_set read_fds; // conjunto temporal para lectura de descriptores de fichero para select()
	 int fdmax;        // Ultimo socket recibido
	 FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	 FD_ZERO(&read_fds);
	 int socketEs = iniciar_socket_escucha("127.0.0.1", puerto); // obtener socket para listen

	 FD_SET(socketEs, &master);// añadir socketEscucha al conjunto maestro
	 fdmax = socketEs;

	 while(1){
	 read_fds = master;
	 select(fdmax+1, &read_fds, NULL, NULL, NULL); // @suppress("Symbol is not resolved")
	 for(int i = 0; i <= fdmax; i++) { // explorar conexiones existentes en busca de datos que leer
	 if (FD_ISSET(i, &read_fds)) { //Hay datos que leer...
	 if (i == socketEs) { //si se recibe en el socket escucha hay nuevas conexiones que aceptar
	 admitirNuevoCliente(&master, &fdmax, i); //agregar al master los nuevos clientes
	 } else {
	 atenderCliente(&master, i); //leer mensajes y paquetes e imprimirlos por consola
	 }
	 }
	 }
	 }*/
	return 0;
}

/*
 * Cosas de Facu
 */
//void* magia_muse_get(t_proceso* proceso, t_list* paqueteRecibido){
//	int cantidadDeBytes = *((int*)list_get(paqueteRecibido, 1));
//	uint32_t direccion = *((uint32_t*)list_get(paqueteRecibido, 2));
//
//	segment* segmento = buscar_segmento_dada_una_direccion(proceso->tablaDeSegmentos, direccion);
//	void* buffer = malloc(cantidadDeBytes);
//	void* marco;
//	page* pagina = malloc(sizeof(page));
//
//		if(segmento == NULL){
//			free(buffer);
//			return NULL; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
//		}
//
//		if((direccion+cantidadDeBytes)>limite_segmento(segmento)){
//			free(buffer);
//			return NULL; // SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
//		}
//
//		div_t aux = numero_pagina(segmento, direccion);
//		int numeroPagina = aux.quot; //pagina correspondiente a la direccion
//		int desplazamientoEnPagina = aux.rem; //desde que posicion de esa pagina vamos a empezar a copiar
//
//		//int sobrante = cantidadDeBytes - (TAM_PAG - desplazamientoEnPagina);
//		//int cantidadDePaginasAMappear = 1; //minimo vamos a mappear la pagina que tiene el dato
//		//
//		//		if(sobrante>0){
//		//			cantidadDePaginasAMappear += paginas_necesarias(sobrante);
//		//			} CREO que esto termina siendo innecesario, lo dejo comentado por las dudas
//
//		int auxiliar = 0;
//		int tamanioACopiar;
//		int desplazamientoEnBuffer = 0;
//
//		while(cantidadDeBytes>0){
//
//				pagina = list_get(segmento->tablaDePaginas, numeroPagina+auxiliar);
//				tamanioACopiar = minimo(cantidadDeBytes, (TAM_PAG - desplazamientoEnPagina));
//				marco = list_get(FRAMES_TABLE, (pagina->numero_frame));
//
//				memcpy(buffer+desplazamientoEnBuffer, marco+desplazamientoEnPagina , tamanioACopiar);
//
//				cantidadDeBytes -= tamanioACopiar;
//				auxiliar++; //siguiente pagina
//				desplazamientoEnBuffer += tamanioACopiar;
//				desplazamientoEnPagina = 0; //Solo era valido para la primera pagina
//		}
//
//		free(pagina);
//		return(buffer);
//}
//int magia_muse_init(t_proceso* cliente_a_atender, char* ipCliente, int id){
//		if(cliente_a_atender != NULL){
//			return ERROR; //YA EXISTE EN NUESTRA TABLA ERROR
//
//			} else {
//			t_proceso* procesoNuevo = crear_proceso(id, ipCliente);
//			list_add(PROCESS_TABLE, procesoNuevo); //Si no existe lo creamos y agregamos
//			return 0;
//			}
//}
//int magia_muse_cpy(t_proceso* proceso, t_list* paqueteRecibido){
//
//	int cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 1));
//	uint32_t direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 3));
//
//	void* buffer = (void*)malloc(cantidad_de_bytes);
//
//	memcpy(buffer, list_get(paqueteRecibido,2), cantidad_de_bytes);
//
//	segment* segmento = buscar_segmento_dada_una_direccion(proceso->tablaDeSegmentos, direccion_pedida);
//
//		if(segmento == NULL){
//				free(buffer);
//				return -1; //ERROR DIRECCION NO CORRESPONDE A UN SEGMENTO.
//			}
//
//		if((direccion_pedida+cantidad_de_bytes)>limite_segmento(segmento)){
//				free(buffer);
//				return -1; // SEGMENTATION FAULT, si bien la direccion corresponde al segmento, se desplaza mas alla de su limite
//			}
//
//	int desplazamientoEnSegmento = direccion_pedida - segmento->baseLogica;
//	void *segmentoMappeado = malloc(((segmento->tablaDePaginas->elements_count)*TAM_PAG)+sizeof(metadata));
//	mappear_segmento(segmento, segmentoMappeado);
//
//	bool puedeEscribirse = segmento_puede_escribirse(segmentoMappeado, desplazamientoEnSegmento, cantidad_de_bytes);
//
//	if(!puedeEscribirse){
//		free(buffer);
//		free(segmentoMappeado);
//		return -1; //Quiere escribir sobre una metadata.
//	}
//
//	escribir_segmento(segmento, direccion_pedida, cantidad_de_bytes, buffer);
//	free(buffer);
//	free(segmentoMappeado);
//	return 0;
//}
